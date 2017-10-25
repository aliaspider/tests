#include <stdlib.h>

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
