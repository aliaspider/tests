#pragma once

#include <assert.h>
#include <stdio.h>
#include "interface.h"

#define fatal(...)   do{debug_log(__VA_ARGS__); fflush(stdout); assert(0);}while(0)

#define CHECK_ERR(x) do{if((x) < 0) {fflush(stdout);assert(0);}}while(0)

#define DEBUG_CHAR(x) do{debug_log("%-40s : %6c\n", #x, x); fflush(stdout);}while(0)
#define DEBUG_INT(x) do{debug_log("%-40s : %6i\n", #x, x); fflush(stdout);}while(0)
#define DEBUG_BOOL(x) do{debug_log("%-40s : %6s\n", #x, x?"true":"false"); fflush(stdout);}while(0)
#define DEBUG_LONG(x) do{debug_log("%-40s : %6li\n", #x, x); fflush(stdout);}while(0)
#define DEBUG_FTPOS(x) do{debug_log("%-40s : %6li  (%4li)\n", #x, x >> 6, x); fflush(stdout);}while(0)
#define DEBUG_FTSHORT(x) do{debug_log("%-40s : %6hi  (%4hi)\n", #x, x >> 6, x); fflush(stdout);}while(0)

const char* console_get(void);
int console_get_len(void);
