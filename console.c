#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#define CON_BUFFER_SIZE    (1 << 13)
#define CON_BUFFER_MASK    (CON_BUFFER_SIZE - 1)

static char con_buffer[CON_BUFFER_SIZE];
static char* con_w = con_buffer;

void console_log(const char* fmt, ...)
{
   char* str;
   va_list va;
   va_start(va, fmt);
   int len = vasprintf(&str, fmt, va);
   va_end(va);

   int available = (CON_BUFFER_SIZE - 1 - (con_w - con_buffer));
   if(len > (CON_BUFFER_SIZE >> 1))
      len = (CON_BUFFER_SIZE >> 1) - 1;

   if(available < len)
   {
      const char* src = con_buffer + (CON_BUFFER_SIZE >> 1);
      while(*src && *src != '\n')
         src++;
      memcpy(con_buffer, src, (con_w - con_buffer) - (src - con_buffer) + 1);
      con_w -= (src - con_buffer) - 1;
   }
   memcpy(con_w, str, len);
   con_w += len;
   *con_w = '\0';

   printf(str);
   free(str);
}

const char* console_get(void)
{
   return con_buffer;
}

int console_get_len(void)
{
   return (con_w - con_buffer) < (CON_BUFFER_SIZE >> 1) ? (con_w - con_buffer) : (CON_BUFFER_SIZE >> 1);
}

