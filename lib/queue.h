/*-----------------------------------------------------------------------------
FILE        : QUEUE.H
INFO        : Header of queue class implementation
DEVELOPEMENT: GCC 2.6.3 && Borland C++ 3.1, Model=Large, ANSI-Keywords
CREATED BY  : RG[28-Apr-1995]
MODIFIED BY : XX[XX-XXX-XXXX]
-----------------------------------------------------------------------------*/
#ifndef	_QUEUE_H
#define	_QUEUE_H

/*------------- GLOBAL DEFINITIONS ------------------------------------------*/
#define MAX_QUEUE	10


int  QueuePackage__Constr(void);
void QueuePackage__Destru(void);
int  Queue__Constr(void);
void Queue__Destru(int q);
int  Queue__Insert(char *i, int q);
int  Queue__HInsert(char *i, int q);
char *Queue__Extract(int q);
char *Queue__ExtractBC(char *i, int q);
char *Queue__Pick(int i, int q);
char *Queue__Read(int i, int q);
int  Queue__Count(int q);

#endif  /* _QUEUE_H */
