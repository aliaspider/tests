#pragma once

#include <assert.h>
#include <stdio.h>
#include <inttypes.h>
#include "interface.h"

#define fatal(...)   do{debug_log(__VA_ARGS__); fflush(stdout); assert(0);}while(0)

#define CHECK_ERR(x) do{if((x) != 0) {fflush(stdout);assert(0);}}while(0)

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

