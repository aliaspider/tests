#pragma once

#include <assert.h>
#include <stdio.h>
#include <inttypes.h>

#ifndef debug_log
#define debug_log    printf
#endif

#ifndef countof
#define countof(a) (sizeof(a)/ sizeof(*a))
#endif

#define CNT_ARGS(...) CNT_ARGS_(__VA_ARGS__,24,23,22,21,20,19,18,17,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0)
#define CNT_ARGS_(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,n,...) n

#define xstr(s) str(s)
#define str(s) #s

#define fatal(...)   do{debug_log(__VA_ARGS__); fflush(stdout); assert(0);}while(0)

#define CHECK_ERR(x) do{int res = x; if(res != 0) {printf("error at %s:%i:%s: 0x%08X(%i)\n", \
   __FILE__, __LINE__,__FUNCTION__, res, res);fflush(stdout);assert(0);}}while(0)

#define DEBUG_LINE(x) do{debug_log("%s@%i : %s\n", __FILE__, __LINE__, __FUNCTION__); fflush(stdout);}while(0)
#define DEBUG_CHAR(x) do{debug_log("%-40s : %6c\n", #x, x); fflush(stdout);}while(0)
#define DEBUG_STR(x) do{debug_log("%-40s : %6s\n", #x, x); fflush(stdout);}while(0)
#define DEBUG_INT(x) do{debug_log("%-40s : %6i\n", #x, x); fflush(stdout);}while(0)
#define DEBUG_BOOL(x) do{debug_log("%-40s : %6s\n", #x, x?"true":"false"); fflush(stdout);}while(0)
#define DEBUG_LONG(x) do{debug_log("%-40s : %6li\n", #x, x); fflush(stdout);}while(0)
#define DEBUG_PTR(x) do{debug_log("%-40s : 0x%016"PRIXPTR"\n", #x, x); fflush(stdout);}while(0)
#define DEBUG_FTPOS(x) do{debug_log("%-40s : %6li  (%4li)\n", #x, x >> 6, x); fflush(stdout);}while(0)
#define DEBUG_FTSHORT(x) do{debug_log("%-40s : %6hi  (%4hi)\n", #x, x >> 6, x); fflush(stdout);}while(0)

#ifdef __WIN32__
#include "win/wincommon.h"
#else
typedef int8_t s8;
typedef uint8_t u8;
typedef int16_t s16;
typedef uint16_t u16;
typedef int32_t s32;
typedef uint32_t u32;
typedef int64_t s64;
typedef uint64_t u64;
#endif

const char* console_get(void);
int console_get_len(void);
extern int console_update_counter;

void display_message(int frames, int x, int y, unsigned screen_mask, const char* fmt, ...);

