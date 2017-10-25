#include <stdlib.h>
#if 0
typedef struct node_t
{
   void* value;
   struct node_t* next;
}node_t;

typedef struct
{
   node_t* first;
   node_t* last;
}linked_list_t;

static inline void linked_list_add(linked_list_t* list, void* value)
{
   node_t* node = malloc(sizeof(*node));
   node->value = value;
   node->next = NULL;
   if(!list->first)
      list->first = node;

   if(list->last)
      list->last->next = node;

   list->last = node;
}

static inline void linked_list_remove(linked_list_t* list, void* value)
{
   node_t** node = &list->first;
   while(*node)
   {
      if((*node)->value == value)
      {
         if(list->last == *node)
            list->last = NULL;

         node_t* next = (*node)->next;
         free(*node);
         *node = next;
      }
      node = &(*node)->next;
   }
}
#else
typedef struct node_t
{
   void* value;
   struct node_t* next;
   struct node_t* prev;
}node_t;

typedef struct
{
   node_t* first;
   node_t* last;
}linked_list_t;

static inline void linked_list_add(linked_list_t* list, void* value)
{
   node_t* node = malloc(sizeof(*node));
   node->value = value;
   node->next = NULL;
   node->prev = list->last;
   if(!list->first)
      list->first = node;

   if(list->last)
      list->last->next = node;

   list->last = node;
}

static inline void linked_list_remove(linked_list_t* list, void* value)
{
   node_t* node = list->first;
   while(node)
   {
      if(node->value == value)
      {
         if(node->prev)
            node->prev->next = node->next;
         else
            list->first = node->next;

         if(node->next)
            node->next->prev = node->prev;
         else
            list->last = node->prev;

         free(node);
         return;
      }
      node = node->next;
   }
}

#define list_add(first, val)\
   do{\
   typeof(val*) node = dst;\
   while(node)\
   {\
      if(node->next) \
         node = node->next;\
      else \
      {\
         node->next = (typeof(val*))malloc(sizeof(val));\
         node->next = val;\
         node->next->next = NULL;\
         break;\
      }\
   }\
}while(0)

#define list_remove(first, pval)\
   do{\
   typeof(pval) node = dst;\
   while(node)\
   {\
      if(node->next) \
         node = node->next;\
      else \
      {\
         node->next = (typeof(val*))malloc(sizeof(val));\
         node->next = val;\
         node->next->next = NULL;\
         break;\
      }\
   }\
}while(0)

#endif
