
#include <stdio.h>

#define debug_log printf

#define CHECK_ERR(x) do{if((x) < 0) {fflush(stdout);exit(0);}}while(0)
