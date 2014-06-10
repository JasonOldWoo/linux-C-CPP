#ifndef _HANDLER_H__

#include "common.h"


int server_process(const RMSG* rmsg, const char* ibuf, const ssize_t ilen, char** obufp, ssize_t* olenp);

int user_login(const char* ibuf, const ssize_t ilen, char** obufp, ssize_t* olenp);


#endif	// __HANDLER_H__
