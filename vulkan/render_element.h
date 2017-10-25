
typedef struct render_element_t
{
   void(*update)(void* data);
   void(*render)(void* data);
   void* data;
}render_element_t;

