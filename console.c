
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

static char con_buffer[1 << 13];
static int con_pos;


void console_log(const char* fmt, ...)
{
   int available = (sizeof(con_buffer) - con_pos);

   va_list va, va2;
   va_start(va, fmt);
   va_copy(va2, va);

   int len = vsnprintf(con_buffer + con_pos, available, fmt, va);
   va_end(va);

   if(len > (available - 1))
   {
      va_start(va2, fmt);
      if(len > sizeof(con_buffer) >> 1)
      {
         char* str = malloc(len + 1);
         vsprintf(str, fmt, va2);
         con_pos = (sizeof(con_buffer) >> 1);
         memcpy(con_buffer, str + len - (sizeof(con_buffer) >> 1), (sizeof(con_buffer) >> 1) + 1);
         printf(str);
         free(str);
         return;
      }
      else
      {
         int copy_len = (sizeof(con_buffer) >> 1) - len;
         memcpy(con_buffer, con_buffer + con_pos - copy_len, copy_len);
         con_pos = copy_len;
         vsprintf(con_buffer + con_pos, fmt, va2);
      }
      va_end(va2);
   }

   printf(con_buffer + con_pos);
   con_pos += len;
}


const char* console_get(void)
{
   return con_pos < (sizeof(con_buffer) >> 1) ? con_buffer : con_buffer + con_pos - (sizeof(con_buffer) >> 1);
}

int console_get_len(void)
{
   return con_pos < (sizeof(con_buffer) >> 1) ? con_pos : (sizeof(con_buffer) >> 1);
}
