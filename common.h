
#include <stdio.h>

#define debug_log    printf
#define fatal(...)   do{printf(__VA_ARGS__); fflush(stdout); assert(0);}while(0)

#define CHECK_ERR(x) do{if((x) < 0) {fflush(stdout);assert(0);}}while(0)
