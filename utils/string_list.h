
#include <stdlib.h>

typedef struct
{
   int count;
   int capacity;
   const char *data[];
} string_list_t;


static inline string_list_t* string_list_create()
{
   string_list_t* dst;
   int capacity = 256;
   dst = (string_list_t*)malloc(sizeof(dst) + capacity * sizeof(*dst->data));
   dst->count = 0;
   dst->capacity = capacity;
   return dst;
}

static inline void string_list_push(string_list_t *dst, const char *string)
{
   if (dst->count == dst->capacity)
   {
      dst->capacity <<= 1;
      dst = realloc(dst, sizeof(*dst) + dst->capacity * sizeof(*dst->data));
   }

   dst->data[dst->count++] = string;
}
