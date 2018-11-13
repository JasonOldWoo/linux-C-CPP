#ifndef __DRPC_H__
#define __DRPC_H__

typedef struct _pkt_hdr_t
{
  int cmd;
  int len;
  int cnt;
} pkt_hdr_t;

//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////
// macro refine
#define PARM1(h, cmd, p) \
  (h, cmd, 2, &(p), sizeof(p))
#define PARM2(h, cmd, p, p1) \
  (h, cmd, 4, &(p), sizeof(p), &(p1), sizeof(p1))
#define PARM3(h, cmd, p, p1, p2) \
  (h, cmd, 6, &(p), sizeof(p), &(p1), sizeof(p1), &(p2), sizeof(p2))
#define PARM4(h, cmd, p, p1, p2, p3) \
  (h, cmd, 8, &(p), sizeof(p), &(p1), sizeof(p1), &(p2), sizeof(p2), &(p3), sizeof(p3))
#define PARM5(h, cmd, p, p1, p2, p3, p4) \
  (h, cmd, 10, &(p), sizeof(p), &(p1), sizeof(p1), &(p2), sizeof(p2), &(p3), sizeof(p3), &(p4), sizeof(p4))
#define PARM6(h, cmd, p, p1, p2, p3, p4, p5) \
  (h, cmd, 12, &(p), sizeof(p), &(p1), sizeof(p1), &(p2), sizeof(p2), &(p3), sizeof(p3), &(p4), sizeof(p4), &(p5), sizeof(p5))

//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////

int drpc_call_core_input_param_cli(int, int cmd, int n, ...);
int drpc_call_core_output_param_cli(int handle, int cmd, int n, ...);
int drpc_call_core_end_cli(int handle);
int drpc_call_core_end_srv(int handle);
int drpc_call_core_init_srv();
int drpc_call_core_accept_srv(int handle);
int drpc_call_core_fetch_head_srv(int handle, pkt_hdr_t* hdr);
int drpc_call_core_input_param_srv(int handle, int cmd, int n, ...);
int drpc_call_core_output_param_srv(int handle, int cmd, int n, ...);

#endif
