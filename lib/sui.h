/*-----------------------------------------------------------------------------
FILE        : SUI.H
INFO        : Shell User Interface
DEVELOPEMENT: GCC 2.6.3 && Borland C++ 3.1, Model=Large, ANSI-Keywords
CREATED BY  : RG[28-Apr-1995]
MODIFIED BY : XX[XX-XXX-XXXX]
-----------------------------------------------------------------------------*/
#ifndef	_SUI_H
#define	_SUI_H

#include <stdio.h>	/* FILE *,  */
#include <ctype.h>	/* toupper() */
#include <string.h>	/* strcpy(), strcat(),  */
#include <stdlib.h>	/* atoi()  */

/*------------- GLOBAL DEFINITIONS ------------------------------------------*/
#ifdef	WIN32
   #define POPEN _popen
   #define PCLOSE _pclose
#else
   #define POPEN popen
   #define PCLOSE pclose
#endif

#define TRUE            1
#define FALSE           0

typedef int		mybool;
typedef unsigned long   ulng;
typedef unsigned int    uint;
typedef short int       shrt;
typedef	unsigned char	byte;
typedef double		real;

#define MAXNODENAMELEN  80
#define MAXUSERNAMELEN  32
#define MAXEXTLEN       16

#define MAXULNGBITS	32

#ifdef __BORLANDC__
   #define SLASH "\\"
   #define spanfn(a, u, dir, f, t, k, o) \
   { \
      char xxx[8]; \
      sprintf(xxx, "%d", k); \
      sprintf(a, "%s\\%s\\%c%c%s%s%s", u, dir, (f)[0], (f)[1], t, (k < 0) ? "" : xxx, o); \
   }
   #define spanfn01(a, u, dir, f, t, k, o) \
   { \
      char xxx[8]; \
      sprintf(xxx, "%03d", k); \
      sprintf(a, "%s\\%s\\%c%c%s%s%s", u, dir, (f)[0], (f)[1], t, (k < 0) ? "" : xxx, o); \
   }
   #define spanfn0(a, u, dir, f) \
      sprintf(a, "%s\\%s\\%s", u, dir, f)
   #define spanfn00(a, u, dir, f) \
      sprintf(a, "%s\\%s%s", u, dir, f)
   #define spanfn000(a, u, dir, f) \
      sprintf(a, "%s\\%s\\%s", u, dir, f)
   #define spanfn0000(a, u, dir, cdir, n, f) \
      sprintf(a, "%s\\%s\\%s.%d\\%s", u, dir, cdir, n, f)
   #define spanfn00000(a, u, dir, cdir, n, sdir, f) \
      sprintf(a, "%s\\%s\\%s.%d\\%s\\%s", u, dir, cdir, n, sdir, f)
   #define spanfn1(a, u, dir, t, pn, pi, ext) \
      sprintf(a, "%s\\%s\\%c%03d%03d%s", u, dir, (t)[0], pn, pi, ext)
   #define spanfn1(a, u, dir, t, pn, pi, ext) \
      sprintf(a, "%s\\%s\\%c%03d%03d%s", u, dir, (t)[0], pn, pi, ext)
   #define spanfn2(a, u, dir, t, pi, ext) \
      sprintf(a, "%s\\%s\\%c%03d%s", u, dir, (t)[0], pi, ext)
   #define spanfn3(a, u, dir, t, n, q, ext) \
      sprintf(a, "%s\\%s\\%c%s%s", u, dir, (t)[0], q, ext)
   #define spanfn3(a, u, dir, t, q, n, ext) \
      sprintf(a, "%s\\%s\\%c%03d%s%s", u, dir, (t)[0], q, n, ext)
   #define NULDEV " > NUL"
#else
   #define SLASH "/"
   #define spanfn(a, u, dir, f, t, k, o) \
   { \
      char xxx[8]; \
      sprintf(xxx, "%d", k); \
      sprintf(a, "%s/%s/%s%s%s%s", u, dir, f, t, (k < 0) ? "" : xxx, o); \
   }
   #define spanfn01(a, u, dir, f, t, k, o) \
   { \
      char xxx[8]; \
      sprintf(xxx, "%03d", k); \
      sprintf(a, "%s/%s/%s%s%s%s", u, dir, f, t, (k < 0) ? "" : xxx, o); \
   }
   #define spanfn0(a, u, dir, f) \
      sprintf(a, "%s/%s/%s", u, dir, f)
   #define spanfn00(a, u, dir, f) \
      sprintf(a, "%s/%s%s", u, dir, f)
   #define spanfn000(a, u, dir, f) \
      sprintf(a, "%s/%s/%s", u, dir, f)
   #define spanfn001(a, u, dir, sdir, f, ext) \
      sprintf(a, "%s/%s/%s/%s%s", u, dir, sdir, f, ext)
   #define spanfn0000(a, u, dir, cdir, n, f) \
      sprintf(a, "%s/%s/%s.%d/%s", u, dir, cdir, n, f)
   #define spanfn00000(a, u, dir, cdir, n, sdir, f) \
      sprintf(a, "%s/%s/%s.%d/%s/%s", u, dir, cdir, n, sdir, f)
   #define spanfn000000(a, u, dir, cdir, n, sdir, f, ext) \
      sprintf(a, "%s/%s/%s.%d/%s/%s%s", u, dir, cdir, n, sdir, f, ext)
   #define spanfn1(a, u, dir, t, pn, pi, ext) \
      sprintf(a, "%s/%s/%s_%03d%03d%s", u, dir, t, pn, pi, ext)
   #define spanfn2(a, u, dir, t, pi, ext) \
      sprintf(a, "%s/%s/%s_%03d%s", u, dir, t, pi, ext)
   #define spanfn3(a, u, dir, t, q, ext) \
      sprintf(a, "%s/%s/%s_%s%s", u, dir, t, q, ext)
   #define spanfn4(a, u, dir, t, q, n, ext) \
      sprintf(a, "%s/%s/%s_%s%03d%s", u, dir, t, q, n, ext)
   #define NULDEV " 2> /dev/null"

   #define spanfn10(a, u, dir, t, pn, tn, pi, ext) \
   {\
      char x[32]; int i = 0;\
      while ((t)[i] != '\0' && i < 31) { x[i] = toupper((t)[i]); ++i; }\
      (x)[i] = '\0';\
      sprintf(a, "%s/%s/%s/%d/%s_%03d%03d%s", u, dir, x, pn, t, tn, pi, ext);\
   }
   #define spanfn20(a, u, dir, t, pn, pi, ext) \
   {\
      char x[32]; int i = 0;\
      while ((t)[i] != '\0' && i < 31) { x[i] = toupper((t)[i]); ++i; }\
      (x)[i] = '\0';\
      sprintf(a, "%s/%s/%s/%d/%s_%03d%s", u, dir, x, pn, t, pi, ext);\
   }
#endif

#define perc2s(x, tot) ((ulng)(((float)(x) / tot) * 100))
#define perc2d(x, tot) \
   ((ulng)(((float)(x) / tot) * 10000) - (perc2s(x, tot)) * 100)

#define myceil(a)       ((a) + ((((a) - ((long)(a))) > 0) ? 1 : 0)) 
#define myceil1(a, b)   (((long)(a))/((long)(b)) + (((((long)(a)) % ((long)(b))) != 0) ? 1 : 0)) 

#define MAXLINELEN      1024 
#define MAXFPNAMELEN	1024	/* MAX Full Path  NAME  LENgth */
#define	MAXFILENAMELEN	1024    /* MAX FILE       NAME  LENgth */
#define MAXIDNAMELEN	32	/* MAX IDentifier NAME  LENgth */
#define MAXIDVALLEN	1024    /* MAX IDentifier VALue LENgth */


#define	MAXINISECTIONS	64      /* No more than 64 IniSections !! */
#define HEADER		0
#define ENDING		255
#define	STRING		1
#define	NUMVAL		2
#define	CHRVAL		3
#define YES_NO		4
#define	LNUMVAL		5
#define	LINF		2147483647L
typedef struct
{
   char varname[MAXIDNAMELEN];
   int  vartype;
   long minval;
   long maxval;
   void *value;
} IniSection;
typedef	IniSection	*IniT;

void	SUIConstr(void (*md)(void), const char *app[], char *ifn);
void	SUIConstr1(void (*md)(void), const char *app[], char *ifn, int nolog);
void	SUIDestru(void);

/* Send output string to                                  LOG DIS OUT FIL */
int	Log(char *fmt, ...);                   /*          +              */
int	Display(char *fmt, ...);               /*          +   +          */
int	Out(char *fmt, ...);                   /*          +       +      */
int	DisplayOut(char *fmt, ...);            /*          +   +   +      */
int	FileOut(FILE *fp, char *fmt, ...);     /*          +           +  */

void	*Malloc(unsigned long size);
void	*Calloc(unsigned long nmemb, unsigned long size);
void	Free(void *block);
void	ExitProg(char *fmt, ...);
int	RegisterIniSec(IniSection *isp);
void	LogIniParm(void);
void	GenIniTemplate(void);

FILE	*FOpen(const char *fname, const char *mode, int	(**fc)(FILE *));

/* Parsing Primitives */
int GetToken(char *str, int i, char *buf);
int IsNumVal(char *buf, int *v);
int IsUNumVal(char *buf, uint *v);
int IsLNumVal(char *buf, ulng *v);

void Mysyspars(char *resu, char *key);
void Mysysopen(char *strin);
void Mysysclose(void);

extern char   flogname[MAXFPNAMELEN];

/*************************************************************/
/* Common definitions for MSK and SKG */

#define PID_IDLE        32766
#define PID_ENDTRACE    32767
/*************************************************************/

/** QUEUE module ****************************************************/
/** QUEUE.H **/
#ifndef	_QUEUE_H
#define	_QUEUE_H

#define MAX_QUEUE	500

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

/********************************************************************/

/** VERSION module **************************************************/
/** VERSION.H **/
#include "version.h"
/********************************************************************/

/** FGZIP module ****************************************************/
/** FGZIP.H **/
#ifndef	_FGZIP_H
#define	_FGZIP_H
#define MAXFPNAMELEN	1024	/* MAX Full Path  NAME  LENgth */

#ifdef __BORLANDC__
   #define SLASH "\\"
   #define NULDEV " > NUL"
#else
   #define SLASH "/"
   #define NULDEV " 2> /dev/null"
#endif

typedef struct FPnodeTAG
{
   FILE *fp;
   int (*fc)(FILE *);
   int (*gtc)(FILE *fp);
   int (*ptc)(int c, FILE *fp);
   int (*prf)(FILE *stream, const char *format, ...);
   struct FPnodeTAG *next;
} FPnode;


FILE	*Fopen(const char *fname, const char *mode);
int 	Fclose(FILE *fp);
int	Fgetc(FILE *fp);
int	Fputc(int c, FILE *fp);
int	FPrintf(FILE *stream, const char *format, ...);

#endif  /* _FGZIP_H */
/********************************************************************/

/********************************************************************/

/** PARRAY module****************************************************/
/** PARRAY.H **/
#ifndef	_PARRAY_H
#define	_PARRAY_H

#define MAX_PARRAY	10

int  PArrayPackage__Constr(void);
void PArrayPackage__Destru(void);
int  PArray__Constr(int hbits, int lbits, int tbits);
void PArray__Destru(int p);
int  PArray__Store(ulng a, char *i, int p);
int  PArray__Load(ulng a, char **i, int p);
int  PArray__Count(int p);
int  PArray__ForEach(int p, void (*f)(char *));

#endif  /* _PARRAY_H */
/********************************************************************/

/* Uniform random number generator macros */
#define NI 0xFFFFFFFFUL
#define A0 69069
#define C0 1

#ifdef OLDGEN
extern ulng xx;
#define UNIFORM1(u)     (((u = (u * A0 + C0))) / (real)NI)
#define NF 4294967295.0
#define Uniform1(u)     ((u = (u) * A0 + C0))
#define UNIFORMN(u, n)  ((short)((Uniform1(u) / (NF + 1)) * (n)))
#else
/* Note: '&NI' is necessary when compiling on 64-bit processors (Alpha)*/
#define UNIFORM1(u)     (((u = ((u * A0 + C0) & NI))) / (real)NI)
#define UNIFORM0(u)     (((u = ((u * A0 + C0) & NI))) / ((real)NI + 1))
#define UNIFORMN(u, n)  ((short)(UNIFORM0(u) * (n)))
#endif

#endif  /* _SUI_H */
