#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#define CON_BUFFER_SIZE    (1 << 12)
#define CON_BUFFER_MASK    (CON_BUFFER_SIZE - 1)

static char con_buffer[CON_BUFFER_SIZE];
static const char* con_r = con_buffer;
static char* con_w = con_buffer;

void console_log(const char* fmt, ...)
{
   char* str;
   va_list va;
   va_start(va, fmt);
   int len = vasprintf(&str, fmt, va);
   va_end(va);

   int available = (CON_BUFFER_SIZE - (con_w - con_buffer));
   if(len > CON_BUFFER_SIZE)
      len = CON_BUFFER_SIZE - 1;

   if(len < available)
   {
      memcpy(con_w, str, len);
      if(con_r > con_w && con_r < con_w + len)
         con_r = con_w + len + 1;
      con_w += len;
   }
   else
   {
      memcpy(con_w, str, available);
      memcpy(con_buffer, str + available, len - available);
      con_w = con_buffer + len - available;
      if(con_r < con_w)
         con_r = con_w + 1;
   }
   *con_w = '\0';

   printf(str);
   free(str);
}

const char* console_get(void)
{
   return con_r;
}
