

#include "interface.h"

typedef struct
{
   void (*init)();
   void (*destroy)();
   void (*update)();
   pad_t pad;
}input_t;

extern const input_t input_x11;
extern input_t input;


