#pragma once

#include <assert.h>
#include <stdio.h>
#include <inttypes.h>
#include "interface.h"


#define CNT_ARGS(...) CNT_ARGS_(__VA_ARGS__,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0)
#define CNT_ARGS_(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,n,...) n

#define DROP_TYPE(...) DROP_TYPE_(CNT_ARGS(__VA_ARGS__),__VA_ARGS__)
#define DROP_TYPE_(n,...) DROP_TYPE__(n,__VA_ARGS__)
#define DROP_TYPE__(n,...) DROP_TYPE_##n(__VA_ARGS__)
#define DROP_TYPE_1()
#define DROP_TYPE_2(ptype,pname) pname
#define DROP_TYPE_4(ptype,pname,...) pname, DROP_TYPE_2(__VA_ARGS__)
#define DROP_TYPE_6(ptype,pname,...) pname, DROP_TYPE_4(__VA_ARGS__)
#define DROP_TYPE_8(ptype,pname,...) pname, DROP_TYPE_6(__VA_ARGS__)
#define DROP_TYPE_10(ptype,pname,...) pname, DROP_TYPE_8(__VA_ARGS__)
#define DROP_TYPE_12(ptype,pname,...) pname, DROP_TYPE_10(__VA_ARGS__)
#define DROP_TYPE_14(ptype,pname,...) pname, DROP_TYPE_12(__VA_ARGS__)
#define DROP_TYPE_16(ptype,pname,...) pname, DROP_TYPE_14(__VA_ARGS__)

#define MERGE_TYPE(...) MERGE_TYPE_(CNT_ARGS(__VA_ARGS__),__VA_ARGS__)
#define MERGE_TYPE_(n,...) MERGE_TYPE__(n,__VA_ARGS__)
#define MERGE_TYPE__(n,...) MERGE_TYPE_##n(__VA_ARGS__)
#define MERGE_TYPE_1()
#define MERGE_TYPE_2(ptype,pname) ptype pname
#define MERGE_TYPE_4(ptype,pname,...) ptype pname, MERGE_TYPE_2(__VA_ARGS__)
#define MERGE_TYPE_6(ptype,pname,...) ptype pname, MERGE_TYPE_4(__VA_ARGS__)
#define MERGE_TYPE_8(ptype,pname,...) ptype pname, MERGE_TYPE_6(__VA_ARGS__)
#define MERGE_TYPE_10(ptype,pname,...) ptype pname, MERGE_TYPE_8(__VA_ARGS__)
#define MERGE_TYPE_12(ptype,pname,...) ptype pname, MERGE_TYPE_10(__VA_ARGS__)
#define MERGE_TYPE_14(ptype,pname,...) ptype pname, MERGE_TYPE_12(__VA_ARGS__)
#define MERGE_TYPE_16(ptype,pname,...) ptype pname, MERGE_TYPE_14(__VA_ARGS__)

#define CONCAT_(a,b) a##b
#define CONCAT(a,b) CONCAT_(a,b)


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

//char err_str[256];
//FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, HRESULT_CODE(hr), 0, err_str, sizeof(err_str), NULL);
//printf("%s\n", err_str);

#define CHECK_WINERR(x) do{HRESULT hr = x; if(FAILED(hr)) {\
   printf("error at %s:%i:%s: (%i, %i, %i) 0x%08X(%i)\n", \
   __FILE__, __LINE__, __FUNCTION__, HRESULT_SEVERITY(hr), HRESULT_FACILITY(hr), HRESULT_CODE(hr), hr, hr);\
   fflush(stdout);assert(0);}}while(0)

#define DEBUG_WINERR(x) do{printf("(0x%X, 0x%X, 0x%X) 0x%08X\n", HRESULT_SEVERITY(hr), HRESULT_FACILITY(hr), HRESULT_CODE(hr), hr);fflush(stdout);}while(0)

#define WRAP(type,fn,...) static inline type CONCAT(PREFIX__,fn) (MERGE_TYPE(__VA_ARGS__)) {return DROP_TYPE(THIS_)->lpVtbl->fn(DROP_TYPE(__VA_ARGS__));}

#endif

const char* console_get(void);
int console_get_len(void);
extern int console_update_counter;
