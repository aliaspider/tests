
#include <stdio.h>
#include <inttypes.h>

#define debug_log printf

#define CHECK_ERR(x) do{if((x) < 0) {fflush(stdout);exit(0);}}while(0)

#ifdef __WIN32__
#define U64 "%llu"
#else
#define U64 "%lu"
#endif
