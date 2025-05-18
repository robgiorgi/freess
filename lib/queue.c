/*-----------------------------------------------------------------------------
FILE        : QUEUE.C
INFO        : Queuing stuff
DEVELOPEMENT: GCC 2.6.3 && Borland C++ 3.1, Model=Large, ANSI-Keywords
CREATED BY  : RG[28-Apr-1995]
MODIFIED BY : XX[XX-XXX-XXXX]
-----------------------------------------------------------------------------*/
/*------------- LIBRARY PROCEDURES ------------------------------------------*/
#include <stdio.h>	/* NULL */
#include "sui.h"
/* #include "queue.h" included in sui.h */

/*------------- GENERAL DEFINITIONS -----------------------------------------*/
typedef struct qelemTAG
{
   struct qelemTAG *next;
   struct qelemTAG *prev;
   char *info;
} qelem;

typedef struct QueueTAG
{
   qelem *head;
   qelem *tail;
   int items;
   int allocated;
} Queue;

/*------------- GLOBAL VARIABLES --------------------------------------------*/
static Queue *Q;

/*------------- INTERNAL FUNCTIONS PROTOTYPES -------------------------------*/
/*------------- FUNCTIONS IMPLEMENTATION ------------------------------------*/
/*---------------------------------------------------------------------------*
 NAME      :
 PURPOSE   :
 PARAMETERS:
 RETURN    :
 *---------------------------------------------------------------------------*/
int QueuePackage__Constr(void)
{
   int k;

   Q = Malloc(MAX_QUEUE * sizeof(Queue));
   for (k = 0; k < MAX_QUEUE; ++k) Q[k].allocated = 0;

   return (0);
}

/*---------------------------------------------------------------------------*
 NAME      :
 PURPOSE   :
 PARAMETERS:
 RETURN    :
 *---------------------------------------------------------------------------*/
void QueuePackage__Destru(void)
{
   int k;
  
   if (Q) for (k = 0; k < MAX_QUEUE; ++k) if (Q[k].allocated) Queue__Destru(k);
   Free(Q);
}

/*---------------------------------------------------------------------------*
 NAME      :
 PURPOSE   :
 PARAMETERS:
 RETURN    :
 *---------------------------------------------------------------------------*/
int Queue__Constr(void)
{
   int q;

   for (q = 0; q < MAX_QUEUE; ++q) if (Q[q].allocated == 0) break;
   if (q == MAX_QUEUE) ExitProg("Allowed only %d queues.", MAX_QUEUE);

   /* Initialize q-th queue */
   Q[q].head = NULL;
   Q[q].tail = NULL;
   Q[q].items = 0;
   Q[q].allocated = 1;

   return (q);
}

/*---------------------------------------------------------------------------*
 NAME      :
 PURPOSE   :
 PARAMETERS:
 RETURN    :
 *---------------------------------------------------------------------------*/
void Queue__Destru(int q)
{
   /* Sanity check */  
   if (q < 0 || q >= MAX_QUEUE) ExitProg("Queue index out of range.");

   if (Q[q].items > 0)
   {
      qelem *curr = Q[q].head, *temp;

      while (curr != NULL)
      {
         temp = curr->next;
         Free(curr);
         curr = temp;
      }
      Q[q].items = 0;
      Q[q].head = NULL;
      Q[q].tail = NULL;
      Q[q].allocated = 0;
   }
}

/*---------------------------------------------------------------------------*
 NAME      :
 PURPOSE   : Insert in the tail
 PARAMETERS:
 RETURN    :
 *---------------------------------------------------------------------------*/
int Queue__Insert(char *i, int q)
{
   qelem *temp = Malloc(sizeof(qelem));
 
   /* Sanity check */  
   if (temp == NULL) ExitProg("Not enough memory allocating queue %d.", q);

   /* Initialize queue element */
   temp->info = i;
   temp->next = NULL;
   temp->prev = NULL;

   /* Update queue data */
   ++Q[q].items;
   if (Q[q].tail)
   {
      Q[q].tail->next = temp;
      temp->prev = Q[q].tail;
      Q[q].tail = temp;
   }
   else
      Q[q].head = Q[q].tail = temp;
   
   return (0); 
}

/*---------------------------------------------------------------------------*
 NAME      :
 PURPOSE   : Insert in the head
 PARAMETERS:
 RETURN    :
 *---------------------------------------------------------------------------*/
int Queue__HInsert(char *i, int q)
{
   qelem *temp = Malloc(sizeof(qelem));
 
   /* Sanity check */  
   if (temp == NULL) ExitProg("Not enough memory allocating queue %d.", q);

   /* Initialize queue element */
   temp->info = i;
   temp->next = NULL;
   temp->prev = NULL;

   /* Update queue data */
   ++Q[q].items;
   if (Q[q].head)
   {
      Q[q].head->prev = temp;
      temp->next = Q[q].head;
      Q[q].head = temp;
   }
   else
      Q[q].head = Q[q].tail = temp;
   
   return (0); 
}

/*---------------------------------------------------------------------------*
 NAME      :
 PURPOSE   : Extract from head
 PARAMETERS:
 RETURN    :
 *---------------------------------------------------------------------------*/
char *Queue__Extract(int q)
{
   char *tmp_info;

   if (Q[q].head)
   {
      qelem *temp = Q[q].head->next;
      
      if (temp) temp->prev = NULL;
      if (Q[q].head == Q[q].tail) Q[q].tail = NULL;
      tmp_info = Q[q].head->info;
      Free(Q[q].head);
      --Q[q].items;
      Q[q].head = temp;
   }
   else
   {
      ExitProg("Cannot extract from empty queue %d.", q);
   }
  
   return (tmp_info);
}

/*---------------------------------------------------------------------------*
 NAME      :
 PURPOSE   : Extract By Content
 PARAMETERS:
 RETURN    :
 *---------------------------------------------------------------------------*/
char *Queue__ExtractBC(char *p,  int q)
{
   int k, found = 0;
   qelem *temp;
   char *tmp_info;

   /* Find element */
   temp = Q[q].head;
   while (! found && temp != NULL)
      if (temp->info == p) found = 1; else temp = temp->next;
   
   /* Extract element */
   if (! found) ExitProg("Element not found in queue %d.", q);
   tmp_info = temp->info;
   if (temp->prev) temp->prev->next = temp->next; else Q[q].head = temp->next;
   if (temp->next) temp->next->prev = temp->prev; else Q[q].tail = temp->prev;
   Free(temp);
   --Q[q].items;

   return (tmp_info);
}

/*---------------------------------------------------------------------------*
 NAME      :
 PURPOSE   : Extract i-th element
 PARAMETERS:
 RETURN    :
 *---------------------------------------------------------------------------*/
char *Queue__Pick(int i, int q)
{
   int k;
   qelem *temp;
   char *tmp_info;

   /* Sanity check */  
   if (i >= Q[q].items)
      ExitProg("Index out of range while picking from list %d.", q);

   /* Find i-th element */
   temp = Q[q].head;
   for (k = 1; k <= i; ++k) temp = temp->next; /* skip 0-element */

   /* Extract element */
   tmp_info = temp->info;
   if (temp->prev) temp->prev->next = temp->next; else Q[q].head = temp->next;
   if (temp->next) temp->next->prev = temp->prev; else Q[q].tail = temp->prev;
   Free(temp);
   --Q[q].items;

   return (tmp_info);
}

/*---------------------------------------------------------------------------*
 NAME      :
 PURPOSE   : Read i-th element
 PARAMETERS:
 RETURN    :
 *---------------------------------------------------------------------------*/
char *Queue__Read(int i, int q)
{
   int k;
   qelem *temp;
   char *tmp_info;

   /* Sanity check */  
   if (i >= Q[q].items)
      ExitProg("Index out of range while picking from list %d.", q);

   /* Find i-th element */
   temp = Q[q].head;
   for (k = 1; k <= i; ++k) temp = temp->next; /* skip 0-element */

   /* Read element */
   tmp_info = temp->info;

   return (tmp_info);
}

/*---------------------------------------------------------------------------*
 NAME      :
 PURPOSE   :
 PARAMETERS:
 RETURN    :
 *---------------------------------------------------------------------------*/
int Queue__Count(int q)
{
   return (Q[q].items);
}
