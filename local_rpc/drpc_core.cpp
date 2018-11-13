#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/uio.h>
#include <errno.h>
#include <unistd.h>
#include <limits.h>
#include "drpc.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////
// API

#define UNIX_DOMAIN "/tmp/domain.unix.rpc"
#if 1
void PRINTF(const char*, ...) {
}
#else
#define PRINTF printf
#endif

static
void print_iovec(struct iovec* iov, int count)
{
  int sum_len = 0;
  for (int n = 0; n < count; n++) {
    PRINTF("[%d] base=%p len=%zu\n", n, iov[n].iov_base, iov[n].iov_len);
    sum_len += iov[n].iov_len;
  }
  PRINTF("sum_len=%d\n", sum_len);
}

int __set_non_block(int sock)
{
	return 0;
  int flags = fcntl(sock, F_GETFL, 0);
  return fcntl(sock, F_SETFL, flags | O_NONBLOCK);
}

static
void __recalc_iovec(struct iovec*& iov, int& count, int snd_bytes)
{
  struct iovec* iovit = iov;
  PRINTF("%s enter==> iov=%p count=%d snd_bytes=%d\n", __FUNCTION__, iov, count, snd_bytes);
  int nit = 0;
  for (; nit < count; nit++, iovit++) {
    if (iovit->iov_len > (size_t) snd_bytes) {
      iovit->iov_len -= snd_bytes;
      iovit->iov_base = (char*)iovit->iov_base + snd_bytes;
      iov = iovit;
      break;
    } else {
      snd_bytes -= iovit->iov_len;
    }
  }
  count -= nit;
  PRINTF("%s leave==> iov=%p count=%d snd_bytes=%d\n", __FUNCTION__, iov, count, snd_bytes);
}

static
int __parse_header(int cmd, struct iovec* iov, int count, int* headinfo)
{
  if (cmd != headinfo[0]) {
    PRINTF("%s:%d ****__parse_header error\n", __FILE__, __LINE__);
    return -1;
  }
  for (int n = 0; n < count; n++) {
    if ((size_t) headinfo[n+8] != iov[n].iov_len) {
      PRINTF("%s:%d ****__parse_header error\n", __FILE__, __LINE__);
      return -1;
    }
  }
  return 0;
}

static
int __parse_header2(struct iovec* iov, int count, int* headinfo)
{
  for (int n = 0; n < count; n++) {
    if ((size_t) headinfo[n] != iov[n].iov_len) {
      PRINTF("%s:%d ****__parse_header2 error\n", __FILE__, __LINE__);
      return -1;
    }
  }
  return 0;
}

// sendmmsg requires glibc ver >= 2.14; pwritev is not posix not gnu source
/*
 * return < 0 error; =0 success;
 **/
static
int __posix_do_send(int sock, struct iovec* iov, int count, int total_len, int timeout, int& err)
{
  PRINTF("%s:%d\n", __FILE__, __LINE__);
  int remain_bytes = total_len;
  unsigned long long timeout_ms = timeout * 1000;
  struct timeval tv;
  tv.tv_sec = 0;
  tv.tv_usec = 500;
  while (timeout_ms > 0 && remain_bytes > 0) {
    fd_set fdset;
    FD_ZERO(&fdset);
    FD_SET(sock, &fdset);
    int r = select(sock+1, 0, &fdset, 0, &tv);
    PRINTF("do_send==> r=%d\n", r);
    err = errno;
    if (r < 0) {
      return r;
    } else if (!r) {
      if (timeout_ms < 1) return -1;
      timeout_ms -= 1;
      continue;
    } else if (r) {
      if (!FD_ISSET(sock, &fdset)) {
        if (timeout_ms < 10) return -1;
        timeout_ms -= 10;       // for sys call
        continue;
      }
    }
    int snd_bytes = writev(sock, iov, count);
    PRINTF("do_send==> snd_bytes=%d, error=%s\n", snd_bytes, strerror(errno));
    err = errno;
    if (snd_bytes < 0) {
      if (errno == EAGAIN
          || errno == EWOULDBLOCK
          || errno == EINTR) {
        continue;
      }
      return snd_bytes;
    } else if (!snd_bytes) {
      continue;
    }
    remain_bytes -= snd_bytes;
    if (remain_bytes > 0) __recalc_iovec(iov, count, snd_bytes);
  }
  if (!timeout_ms && remain_bytes > 0) return -1;
  return 0;
}

static
int __posix_do_recv(int sock, struct iovec* iov, int count, int total_len, int timeout, int& err)
{
  int remain_bytes = total_len;
  unsigned long long timeout_ms = timeout * 1000;
  struct timeval tv;
  tv.tv_sec = 0;
  tv.tv_usec = 500;
  while (timeout_ms > 0 && remain_bytes > 0) {
    tv.tv_sec = 0;
    tv.tv_usec = 500;
    fd_set fdset;
    FD_ZERO(&fdset);
    FD_SET(sock, &fdset);
    int r = select(sock+1, &fdset, 0, 0, &tv);
    //PRINTF("do recv==>  sock=%d r=%d\n", sock, r);
    err = errno;
    if (r < 0) {
      return r;
    } else if (!r) {
      if (timeout_ms < 1) return -1;
      timeout_ms -= 1;
      continue;
    } else if (r) {
      if (timeout_ms < 1) return -1;
      timeout_ms -= 1;       // for sys call
      if (!FD_ISSET(sock, &fdset)) {
        continue;
      }
    }
    int rcv_bytes = readv(sock, iov, count);
    //PRINTF("do recv==>  sock=%d rcv_bytes=%d error=%s\n", sock, rcv_bytes, strerror(errno));
    err = errno;
    if (rcv_bytes < 0) {
      if (errno == EAGAIN
          || errno == EWOULDBLOCK
          || errno == EINTR) {
        continue;
      }
      return rcv_bytes;
    } else if (!rcv_bytes) {
      // remote closed
      return -1;
    }
    remain_bytes -= rcv_bytes;
    if (remain_bytes > 0) __recalc_iovec(iov, count, rcv_bytes);
  }
  if (!timeout_ms && remain_bytes > 0) return -1;
  return 0;
}

static
int __posix_call_dunix_begin_cli()
{
  PRINTF("%s:%d\n", __FILE__, __LINE__);
  struct sockaddr_un srv_addr;
  bzero(&srv_addr, sizeof(srv_addr));
  int sock = socket(PF_UNIX, SOCK_STREAM, 0);
  __set_non_block(sock);
  if (sock < 0) {
    PRINTF("create socket error\n");
    return -1;
  }
  srv_addr.sun_family = AF_UNIX;
  strcpy(srv_addr.sun_path, UNIX_DOMAIN);
  int ret = connect(sock, (struct sockaddr*)&srv_addr, sizeof(srv_addr));
  if (ret < 0) {
    PRINTF("connect error\n");
    close(sock);
    return -2;
  }
  return sock;
}

static
int __posix_call_dunix_end_cli(int handle)
{
  close(handle);
  return 0;
}

static
int __posix_call_dunix_begin_srv()
{
  struct sockaddr_un srv_addr;
  bzero(&srv_addr, sizeof(srv_addr));
  int listen_fd = socket(PF_UNIX, SOCK_STREAM, 0);
  if (listen_fd < 0) {
    PRINTF("create listen fd error\n");
    return -1;
  }
  // set svr add
  srv_addr.sun_family = AF_UNIX;
  strncpy(srv_addr.sun_path, UNIX_DOMAIN, sizeof(srv_addr.sun_path)-1);
  unlink(UNIX_DOMAIN);
  // bind sockfd & addr
  if (bind(listen_fd, (struct sockaddr*)&srv_addr, sizeof(srv_addr)) < 0) {
    PRINTF("bind listen fd error\n");
    close(listen_fd);
    unlink(UNIX_DOMAIN);
    return -1;
  }
  if (listen(listen_fd, 10) < 0) {
    PRINTF("listen listen fd error\n");
    close(listen_fd);
    unlink(UNIX_DOMAIN);
    return -1;
  }
  return listen_fd;
}

static
int __posix_call_dunix_accept_srv(int listen_fd)
{
  socklen_t cli_addr_len = 0;
  struct sockaddr_un cli_addr;
  cli_addr_len = sizeof(cli_addr);
  int com_fd = accept(listen_fd, (struct sockaddr*)&cli_addr, &cli_addr_len);
  if (com_fd < 0) {
    PRINTF("accept error\n");
    return -1;
  }
  __set_non_block(com_fd);
  return com_fd;
}

static
int __posix_call_dunix_end_srv(int handle)
{
  close(handle);
  unlink(UNIX_DOMAIN);
  return 0;
}

/*
 * return < 0 error; =0 sucess
 */
static
int __posix_call_dunix_inv(int handle, int cmd, struct iovec* iov, int count, int timeout, int& err)
{
  PRINTF("%s:%d handle=%d cmd=%d\n", __FILE__, __LINE__, handle, cmd);
  int count_in = count + 1;
  if (count_in > IOV_MAX) {
    err = 0;
    return -1;
  }
  int headinfo[count+8];
  memset(headinfo, 0x0, sizeof(headinfo));
  int sum_len = 0;
  for (int n = 0; n < count; n++) {
    sum_len += iov[n].iov_len;
    headinfo[n+8] = iov[n].iov_len;
  }
  sum_len += (count+8) * sizeof(int);
  headinfo[0] = cmd;            // command id
  headinfo[1] = sum_len;        // packet length(include header)
  headinfo[2] = count;          // param number
  PRINTF("%s==> count_in=%d\n", __FUNCTION__, count_in);
  struct iovec iov_in[count_in];
  memcpy(iov_in+1, iov, sizeof(struct iovec)*count);
  iov_in[0].iov_base = headinfo;
  iov_in[0].iov_len = sizeof(headinfo);
  print_iovec(iov_in, count_in);
  return __posix_do_send(handle, iov_in, count_in, sum_len, timeout, err);
}

/*
 * notice it is only suitable for constant length of output parameter
 * return < 0 error; =0 sucess
 */
static
int __posix_call_dunix_ouv(int handle, int cmd, struct iovec* iov, int count, int timeout, int& err)
{
  int count_ou = count + 1;
  if (count_ou > IOV_MAX) {
    err = 0;
    return -1;
  }
  int headinfo[count+8];
  int sum_len = 0;
  for (int n = 0; n < count; n++) {
    sum_len += iov[n].iov_len;
  }
  sum_len += (count+8) * sizeof(int);
  memset(headinfo, 0x0, sizeof(headinfo));
  struct iovec iov_ou[count_ou];
  memcpy(iov_ou+1, iov, sizeof(struct iovec)*count);
  iov_ou[0].iov_base = headinfo;
  iov_ou[0].iov_len = sizeof(headinfo);
  PRINTF("%s==> handle=%d sum_len=%d\n", __FUNCTION__, handle, sum_len);
  if(__posix_do_recv(handle, iov_ou, count_ou, sum_len, timeout, err) < 0
      || __parse_header(cmd, iov, count, headinfo) < 0) {
    return -2;
  }
  return 0;
}

static
int __posix_call_dunix_ou_header(int handle, pkt_hdr_t* hdr, int timeout, int& err)
{
  char pkthdr[32];
  struct iovec hiov;
  hiov.iov_base = pkthdr;
  hiov.iov_len = sizeof(pkthdr);
  if (__posix_do_recv(handle, &hiov, 1, sizeof(pkthdr), timeout, err) < 0) return -2;
  memcpy(hdr, pkthdr, sizeof(pkt_hdr_t));
  return 0;
}

static
int __posix_call_dunix_ouv2(int handle, int cmd, struct iovec* iov, int count, int timeout, int& err)
{
  int count_ou = count + 1;
  if (count_ou > IOV_MAX) {
    err = 0;
    return -1;
  }
  int headinfo[count];
  int sum_len = 0;
  for (int n = 0; n < count; n++) {
    sum_len += iov[n].iov_len;
  }
  sum_len += sizeof(headinfo);
  memset(headinfo, 0x0, sizeof(headinfo));
  struct iovec iov_ou[count_ou];
  memcpy(iov_ou+1, iov, sizeof(struct iovec)*count);
  iov_ou[0].iov_base = headinfo;
  iov_ou[0].iov_len = sizeof(headinfo);
  PRINTF("%s==> handle=%d sum_len=%d\n", __FUNCTION__, handle, sum_len);
  if (__posix_do_recv(handle, iov_ou, count_ou, sum_len, timeout, err) < 0
      || __parse_header2(iov, count, headinfo) < 0) {
    return -2;
  }
  return 0;
}

static
int __posix_call_dunix_ous(int handle, int& cmd, char* buf, int& len, int& err)
{
  return -1;
}

typedef int (*p_call_begin)(void);
typedef int (*p_call_accept)(int);
typedef int (*p_call_inv)(int, int, struct iovec*, int, int, int&);
typedef int (*p_call_ouv)(int, int, struct iovec*, int, int, int&);
typedef int (*p_call_ouh)(int, pkt_hdr_t*, int, int&);
typedef int (*p_call_end)(int);
typedef int (*p_call_end_s)(int, int);

typedef struct _call_op
{
  p_call_begin   cli_init;
  p_call_end     cli_end;
  p_call_begin   srv_init;
  p_call_end     srv_end;
  p_call_accept  srv_accept;
  p_call_inv     input_paramv;
  p_call_ouv     output_paramv;
  p_call_ouv     input_paramv2;
  p_call_ouh     output_header;
} call_op;

static call_op s_call_op = {
  __posix_call_dunix_begin_cli,
  __posix_call_dunix_end_cli,
  __posix_call_dunix_begin_srv,
  __posix_call_dunix_end_srv,
  __posix_call_dunix_accept_srv,
  __posix_call_dunix_inv,
  __posix_call_dunix_ouv,
  __posix_call_dunix_ouv2,
  __posix_call_dunix_ou_header,
};

//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////
// wrapper
#include <stdarg.h>

int drpc_call_core_input_param_cli(int, int cmd, int n, ...)
{
  struct iovec iov[n >> 1];
  va_list vl;
  va_start(vl, n);
  for (int i = 0; i < n; ++i) {
    if (i % 2) {
      size_t val = va_arg(vl, size_t);
      iov[i >> 1].iov_len = val;
      PRINTF("[%d] len=%lu\n", i >> 1, iov[i >> 1].iov_len);
    } else {
      void* val = va_arg(vl, void*);
      iov[i >> 1].iov_base = val;
      PRINTF("[%d] base=%p\n", i >> 1, iov[i >> 1].iov_base);
    }
  }
  int error = 0;
  int handle = s_call_op.cli_init();
  if (handle < 0) return handle;
  if (s_call_op.input_paramv(handle, cmd, iov, n >> 1, 1000, error) < 0) {
    s_call_op.cli_end(handle);
    return -1;
  }
  return handle;
}

int drpc_call_core_output_param_cli(int handle, int cmd, int n, ...)
{
  struct iovec iov[n >> 1];
  va_list vl;
  va_start(vl, n);
  for (int i = 0; i < n; ++i) {
    if (i % 2) {
      size_t val = va_arg(vl, size_t);
      iov[i >> 1].iov_len = val;
      PRINTF("[%d] len=%lu\n", i >> 1, iov[i >> 1].iov_len);
    } else {
      void* val = va_arg(vl, void*);
      iov[i >> 1].iov_base = val;
      PRINTF("[%d] base=%p\n", i >> 1, iov[i >> 1].iov_base);
    }
  }
  int error = 0;
  return s_call_op.output_paramv(handle, cmd, iov, n >> 1, 1000, error);
}

int drpc_call_core_end_cli(int handle)
{
  return s_call_op.cli_end(handle);
}

int drpc_call_core_end_srv(int handle)
{
  return s_call_op.srv_end(handle);
}

int drpc_call_core_init_srv()
{
  return s_call_op.srv_init();
}

int drpc_call_core_accept_srv(int handle)
{
  return s_call_op.srv_accept(handle);
}

int drpc_call_core_fetch_head_srv(int handle, pkt_hdr_t* hdr)
{
  int error = 0;
  return s_call_op.output_header(handle, hdr, 1000, error);
}

int drpc_call_core_input_param_srv(int handle, int cmd, int n, ...)
{
  struct iovec iov[n >> 1];
  va_list vl;
  va_start(vl, n);
  for (int i = 0; i < n; ++i) {
    if (i % 2) {
      size_t val = va_arg(vl, size_t);
      iov[i >> 1].iov_len = val;
      PRINTF("[%d] len=%lu\n", i >> 1, iov[i >> 1].iov_len);
    } else {
      void* val = va_arg(vl, void*);
      iov[i >> 1].iov_base = val;
      PRINTF("[%d] base=%p\n", i >> 1, iov[i >> 1].iov_base);
    }
  }
  int error = 0;
  return s_call_op.input_paramv2(handle, cmd, iov, n >> 1, 1000, error);
}

int drpc_call_core_output_param_srv(int handle, int cmd, int n, ...)
{
  struct iovec iov[n >> 1];
  va_list vl;
  va_start(vl, n);
  for (int i = 0; i < n; ++i) {
    if (i % 2) {
      size_t val = va_arg(vl, size_t);
      iov[i >> 1].iov_len = val;
      PRINTF("[%d] len=%lu\n", i >> 1, iov[i >> 1].iov_len);
    } else {
      void* val = va_arg(vl, void*);
      iov[i >> 1].iov_base = val;
      PRINTF("[%d] base=%p\n", i >> 1, iov[i >> 1].iov_base);
    }
  }
  int error = 0;
  return s_call_op.input_paramv(handle, cmd, iov, n >> 1, 1000, error);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////
// unittest
static
int rpc_call_plus(int a, int b)
{
  struct iovec iov[2];
  iov[0].iov_base = &a;
  iov[0].iov_len = sizeof(a);
  iov[1].iov_base = &b;
  iov[1].iov_len = sizeof(b);
  int err = 0;
  int handle = __posix_call_dunix_begin_cli();
  if (handle < 0) return -1;
  PRINTF("%s==> handle=%d &a=%p &b=%p\n", __FUNCTION__, handle, &a, &b);
  int ret = __posix_call_dunix_inv(handle, 1, iov, 2, 10, err);
  if (ret < 0) {
    __posix_call_dunix_end_cli(handle);
    return -1;
  }
  int call_result = 0;
  struct iovec iov2;
  iov2.iov_base = &call_result;
  iov2.iov_len = sizeof(call_result);
  PRINTF("%s==> handle=%d iov2=%p\n", __FUNCTION__, handle, &iov2);
  ret = __posix_call_dunix_ouv(handle, 1, &iov2, 1, 10, err);
  PRINTF("%s==> recv output finish\n", __FUNCTION__);
  if (ret < 0) {
    __posix_call_dunix_end_cli(handle);
    return -1;
  }
  ret = __posix_call_dunix_end_cli(handle);
  return call_result;
}

static
int rpc_call_srv()
{
  int cli_handle = 0;
  int handle = __posix_call_dunix_begin_srv();
  if (handle < 0) return -1;
  struct iovec iov[6];
  memset(iov, 0x0, sizeof(iov));
  while ((cli_handle = __posix_call_dunix_accept_srv(handle)) > 0) {
    PRINTF("client new conn sock=%d\n", cli_handle);
    int err = 0;
    pkt_hdr_t hdr;
    int ret = __posix_call_dunix_ou_header(cli_handle, &hdr, 10, err);
    if (ret < 0) {
      PRINTF("%s:%d ****read request error!\n", __FILE__, __LINE__);
      __posix_call_dunix_end_cli(cli_handle);
      continue;
    } else {
      // DEBUG
      PRINTF("%s==> cmd=%d cnt=%d len=%d\n", __FUNCTION__, hdr.cmd, hdr.cnt, hdr.len);
    }
    if (1 == hdr.cmd) {
      struct iovec param[2];
      int a, b;
      param[0].iov_base = &a;
      param[0].iov_len = sizeof(a);
      param[1].iov_base = &b;
      param[1].iov_len = sizeof(b);
      int ret = __posix_call_dunix_ouv2(cli_handle, hdr.cmd, param, 2, 10, err);
      PRINTF("%s:%d call ouv2=%d\n", __FILE__, __LINE__, ret);
      if (ret < 0) {
        PRINTF("%s:%d ****read request error!\n", __FILE__, __LINE__);
        __posix_call_dunix_end_cli(cli_handle);
        continue;
      }
      int c = a + b;
      PRINTF("call plus method: a = %d, b = %d\n", a, b);
      struct iovec iov_in;
      iov_in.iov_base = &c;
      iov_in.iov_len = sizeof(c);
      ret = __posix_call_dunix_inv(cli_handle, 1, &iov_in, 1, 10, err);
      // add to cli handle list
      // ....
    }
    PRINTF("client conn sock=%d finish\n", cli_handle);
  }
  __posix_call_dunix_end_srv(handle);
  return 0;
}

static
int dispatch(int handle, int cmd)
{
  switch (cmd) {
  case 1:       // plus
  {
    int in_a = 0; int in_b = 0;
    int ret = drpc_call_core_input_param_srv PARM2(handle, cmd, in_a, in_b);
    if (ret < 0) return -1;
    int ou_c = in_a + in_b;
    PRINTF("%s==> plus: ina=%d inb=%d inc=%d\n", __FUNCTION__, in_a, in_b, ou_c);
    ret = drpc_call_core_output_param_srv PARM1(handle, cmd, ou_c);
    if (ret < 0) return -1;
    break;
  }
  default:
    break;
  }
  return 0;
}

static
int drpc_srv_routine()
{
  int cli_handle = 0;
  int handle = drpc_call_core_init_srv();
  if (handle < 0) {
    PRINTF("%s==> srv init error\n", __FUNCTION__);
    return -1;
  }
  while ((cli_handle = drpc_call_core_accept_srv(handle)) > 0) {
    PRINTF("%s==> client new conn sock=%d\n", __FUNCTION__, cli_handle);
    pkt_hdr_t hdr;
    int ret = drpc_call_core_fetch_head_srv(cli_handle, &hdr);
    if (ret < 0) {
      PRINTF("%s==> **** read request error\n", __FUNCTION__);
      drpc_call_core_end_cli(cli_handle);
      continue;
    } else {
      PRINTF("%s==> cmd=%d cnt=%d len=%d\n", __FUNCTION__, hdr.cmd, hdr.cnt, hdr.len);
    }
    ret = dispatch(cli_handle, hdr.cmd );
    if (ret < 0) {
      drpc_call_core_end_cli(cli_handle);
      continue;
    } else {
      // add to handle list
      // ....
    }
  }
  drpc_call_core_end_srv(handle);
  return 0;
}

static
int drpc_cli_plus(int a, int b)
{
  int handle = drpc_call_core_input_param_cli PARM2(0, 1, a, b);
  if (handle < 0) {
    return -1;
  }
  int ou_c = 0;
  int ret = drpc_call_core_output_param_cli PARM1(handle, 1, ou_c);
  drpc_call_core_end_cli(handle);
  if (ret < 0) return -1;
  else return ou_c;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////
#if 0
#include <pthread.h>

void* cli_routine(void*)
{
  sleep(2);
  PRINTF("start call!\n");
  PRINTF("11 result=%d\n", drpc_cli_plus(11, 29));
  PRINTF("22 result=%d\n", drpc_cli_plus(11, 29));
  return 0;
}

int main()
{
  pthread_t tid = 0;
  pthread_create(&tid, 0, cli_routine, 0);
  drpc_srv_routine();
  pthread_join(tid, 0);
  return 0;
}
#endif
