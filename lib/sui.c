/*-----------------------------------------------------------------------------
FILE        : SUI.C
INFO        : Shell User Interface
PURPOSE     : Interface user program with shell
DEVELOPEMENT: GCC 2.6.3 && Borland C++ 3.1, Model=Large, ANSI-Keywords
CREATED BY  : RG[28-Apr-1995]
MODIFIED BY : XX[XX-XXX-XXXX]
-----------------------------------------------------------------------------*/
/*------------- LIBRARY PROCEDURES ------------------------------------------*/
#include <stdio.h>	/* printf(), fprintf() */
#include <stdlib.h>	/* malloc(), free(), size_t */
#include <string.h>     /* strcpy(), bzero() */
#include <ctype.h>      /* toupper() */
#include <stdarg.h>     /* va_list, va_start() */
#include <errno.h>	/* EACCES */
#include <time.h>       /* localtime(), time(), asctime() */
#include <signal.h>	/* signal(), SIG_DFL */
#include <unistd.h>	/* unlink() */
#include "sui.h"
#include "getopt1.h"

/*------------- GENERAL DEFINITIONS -----------------------------------------*/
#define FINI_EXTENSION	"ini"
#define FLOG_EXTENSION	"log"
#define FOUT_EXTENSION	"out"
#define	MAXBUFFERLEN	8192

/*------------- GLOBAL VARIABLES --------------------------------------------*/
static	const char **App;

static	char    errmsg[MAXLINELEN] = "";
static	int     errcod = 0;
static	char	buf[MAXBUFFERLEN] = "";
static	char	blog[MAXBUFFERLEN] = "";
static	char	bout[MAXBUFFERLEN] = "";
static	FILE	*fpini = NULL;
static	char	fininame[MAXFPNAMELEN] = "";

FILE	*fplog = NULL;

/*
static	FILE	*fpout = NULL;
static	char	foutname[MAXFPNAMELEN] = "";
*/
static	FILE	*fptmp = NULL;
static	char	tlogname[L_tmpnam] = "";
static	int	detail_flag = 0;
static	long	max_allocated = 0;
static	long	cur_allocated = 0;
static	long	calls_to_Malloc = 0;
static	long	calls_to_Free = 0;
static	time_t	t0 = 0, t1 = 0;
static	int	time_counter_initialized = 0;

static	void (*MAINDestru)(void) = NULL;

static	IniT	ITAB[MAXINISECTIONS];
static	int	ITABdim = 0;
/*
static	IniSection Pro[] =
{
   "Program Defaults",          HEADER,         0,      0,      NULL,
   "Log File Name",             STRING,         0,      0,      &flogname,
   "",                          ENDING,         0,      0,      NULL
};
*/

static FILE *Mysysfp;
static char Mysystn[MAXFILENAMELEN];

char	flogname[MAXFPNAMELEN] = "";

/*------------- INTERNAL FUNCTIONS PROTOTYPES -------------------------------*/
static	void	CmdArg(char *param, int sec);
static	void	CmdFile(FILE *handle);
static	char	*StrLower(char *s);
static	int	TransformLog(void);
static	char	*StrVal(char *s, int n, int sec, int k);

void sigsui(int);
void sigsui11(int);
void sigsui17(int);

/*------------- FUNCTION IMPLEMENTATION -------------------------------------*/
/*---------------------------------------------------------------------------*
 NAME      :
 PURPOSE   :
 PARAMETERS: None
 RETURN    : None
 *---------------------------------------------------------------------------*/
void SUIConstr(void (*md)(void), const char *app[], char *ifn)
{
   SUIConstr1(md, app, ifn, 0);
}

/*---------------------------------------------------------------------------*
 NAME      :
 PURPOSE   :
 PARAMETERS: None
 RETURN    : None
 *---------------------------------------------------------------------------*/
void SUIConstr1(void (*md)(void), const char *app[], char *ifn, int nolog)
{
   char aux[MAXFPNAMELEN];
   int res;
   int k;

   /* Setup signal handlers */
   signal(SIGSEGV, sigsui11);
   #ifndef _Linux_
      #define SIGUNUSED 31
   #else
      #define SIGUNUSED SIGSYS
   #endif
   for (k = 1; k < SIGUNUSED && k != SIGSEGV; ++k) signal(k, sigsui);

   MAINDestru = md;
   App = app;

   /*-----------------------------------*/
   /* RegisterIniSec(Pro); */

   /*-----------------------------------*/
   if (nolog == 0) {
/*
      tlogname[0] = '\0';
      tmpnam(tlogname);

      if (NULL == (fplog = fopen(tlogname, "wt")))
         ExitProg("Cannot create tmp log file '%s'.", tlogname);
*/
      strcpy(tlogname, "suilibXXXXXX");
      res = mkstemp(tlogname);
      if (-1 == res)
         ExitProg("Cannot create tmp log file through mkstemp().");

      if (NULL == (fplog = fdopen(res, "wt")))
         ExitProg("Cannot create tmp log file '%s'.", tlogname);
/**/

      sprintf(buf, "___Log file created using temporary name '%s'.", tlogname);
      Log(buf);

      Log("Initializing.");
   }
   fininame[0] = '\0';
   /* flogname[0] = '\0'; */

   /*-----------------------------------*/
   if (ifn == NULL) {
/*
      strcpy(fininame, App[0]);
      strcat(fininame, ".");
      strcat(fininame, FINI_EXTENSION);
      spanfn0(aux, ".", ".", fininame);
      if (NULL == (fpini = fopen(aux, "rt")))
         ExitProg("Cannot find init file '%s'.", aux);
      CmdFile(fpini);
      fclose(fpini); fpini = NULL;
*/
   } else {
      strcpy(fininame, ifn);
      strcpy(aux, ifn);
      if (NULL == (fpini = fopen(aux, "rt")))
         ExitProg("Cannot find init file '%s'.", aux);
      CmdFile(fpini);
   }

   /* Transform temporary log file in current log file */
   if (nolog == 0) {
      if (0 != TransformLog()) ExitProg(errmsg);
      setbuf(fplog, NULL);
   }

   /*-----------------------------------*/
/*
   if (*foutname == '\0') {
      strcpy(foutname, App[0]);
      strcat(foutname, ".");
      strcat(foutname, FOUT_EXTENSION);
      spanfn0(aux, ".", ".", foutname);
   } else {
      strcpy(aux, foutname);
   }
   if (NULL == (fpout = fopen(aux, "wt")))
      ExitProg("Cannot create out file '%s'.", aux);
   setbuf(fpout, NULL);
*/

   /*-----------------------------------*/
   /* Initialize Queue Package */
   QueuePackage__Constr();

   /*-----------------------------------*/
   if (nolog == 0) {
      time(&t0); strftime(aux, 80, "%H:%M:%S %Z, %a %b %d %Y", localtime(&t0));
      time_counter_initialized = 1;
      Out("___Starting execution (%s).", aux);
   }
}

/*---------------------------------------------------------------------------*
 NAME      :
 PURPOSE   :
 PARAMETERS: None
 RETURN    : None
 *---------------------------------------------------------------------------*/
void SUIDestru(void)
{
   QueuePackage__Destru();
}

/*---------------------------------------------------------------------------*
 NAME      :
 PURPOSE   :
 PARAMETERS: None
 RETURN    : None
 *---------------------------------------------------------------------------*/
void sigsui(int i)
{
   ExitProg("SIGNAL %d", i);
}

/*---------------------------------------------------------------------------*
 NAME      :
 PURPOSE   :
 PARAMETERS: None
 RETURN    : None
 *---------------------------------------------------------------------------*/
void sigsui11(int i)
{
   ExitProg("Segment violation.");
}

/*---------------------------------------------------------------------------*
 NAME      :
 PURPOSE   :
 PARAMETERS: None
 RETURN    : None
 *---------------------------------------------------------------------------*/
void sigsui17(int i)
{
   ExitProg("Termination requested by user.");
}

/*---------------------------------------------------------------------------*
 NAME      : ExitProg
 PURPOSE   : Exit program taking some actions...
 PARAMETERS: msg - message string:
		if null string then return immediatly
		if string starts with '-' don't understand as error
		otherwise print message no stderr on log it as error
 RETURN    : None
 *---------------------------------------------------------------------------*/
void ExitProg(char *fmt, ...)
{
   int		k, n;
   va_list	ap;

   va_start(ap, fmt);
   vsprintf(buf, fmt, ap);
   switch (buf[0])
   {
      case '\0':
	 return;
      case '-':
	 /* kindly exit printing an information message */
	 strncpy(errmsg, buf, MAXLINELEN - 2); 
         errmsg[MAXLINELEN - 2] = '\0'; 
	 n = 0;
	 break;
      default:
	 /* Set errmsg to at most MAXLINELEN-2 char of msg; pad with spaces */
	    strncpy(errmsg, buf, MAXLINELEN - 2); 
            errmsg[MAXLINELEN - 2] = '\0'; 
  	 /* for (k = strlen(buf); k < MAXLINELEN - 2; ++k) errmsg[k] = ' '; 
	    strcat(errmsg, "\n"); 
	    fprintf(stderr, "%s", errmsg); */
	 Display("ERROR: %s", errmsg);
         errcod = 1;
/*
         Display(errmsg);
*/
	 n = -1;
	 break;
   }

   if (time_counter_initialized == 1)
   {
      int d, h, m, s, x;
      long dt;

      time(&t1); strftime(buf, 80, "%H:%M:%S %Z, %a %b %d %Y", localtime(&t1));
      dt = t1 - t0;
      d = dt / 86400UL; x = dt % 86400UL;
      h = x / 3600;   x = x % 3600; 
      m = x / 60;     s = x % 60;
      Out("___Ending execution   (%s).", buf);
      Display("___Total execution_time   = %ld s    (%dd %dh %dm %ds).",
         dt, d, h, m, s);
   }

   /* Call all destructors */
   if (MAINDestru != NULL) MAINDestru();
   if (fpini != NULL) fclose(fpini);
/*
   if (fpout != NULL) fclose(fpout);
*/

   sprintf(buf, "Maximum allocated memory: %ldK.", max_allocated >> 10);
   Out(buf);
   sprintf(buf, "Malloc/Free = %ld/%ld.", calls_to_Malloc, calls_to_Free);
   Out(buf);

   /* Maybe that only temporary log file was created but not renamed */
   if (*tlogname != '\0') if (0 != TransformLog()) printf("%s", errmsg);

   Log("Exiting program.");
   if (n == 0) {
      if (buf[1] != '\0') {
         if (fplog) { Log("%s", errmsg + 1); }
         else { printf("%s\n", errmsg + 1); }
      }
   }

   if (fplog != NULL) { fclose(fplog); fplog = NULL; }
   va_end(ap);

   exit(n);
}

/*---------------------------------------------------------------------------*
 NAME      :
 PURPOSE   :
 PARAMETERS:
 RETURN    : None
 *---------------------------------------------------------------------------*/
int RegisterIniSec(IniSection *isp)
{
   if (ITABdim < MAXINISECTIONS - 1)
   {
      ITAB[ITABdim++] = isp;
   }
   return (0);
}

/*---------------------------------------------------------------------------*
 NAME      : CmdArg
 PURPOSE   : Processes a single command-line/command-file argument
	     isolate 'variable=value' (or 'variable:=value') into
	     its components and process it.
	     All components have already been converted to lower case.
 PARAMETERS:
 RETURN    : None
 *---------------------------------------------------------------------------*/
void CmdArg(char *param, int sec)
{
   char	variable[MAXIDNAMELEN];	/* variable name goes here */
   char	value[MAXIDVALLEN];	/* variable value goes here*/
   long	numval;			/* numeric value of arg    */
   char	charval;		/* character value of arg  */
   int	f, j, k = 0;		/* temporary loop counters */

   /* Get variable name */
   for (j = 1; j < strlen(param) && param[j] != '='; j++);
   f = j + 1;
   if (j >= MAXIDNAMELEN || j >= strlen(param))
      ExitProg("'=' not found in argument '%s' of file '%s'.", param, fininame);
      						/* '=' not found             */
   strncpy(variable, param, j--);	        /* get the variable name     */
   /* Strip any trailing ':' or space or tab */
   while (j > 0 && (variable[j] == ':') || variable[j] <= ' ') --j;
   variable[++j] = '\0';	                /* truncate it               */

   /* Get the value string */
   while (param[f] <= ' ') ++f;			/* strip leading space or tab*/
   strcpy(value, &param[f]);
   /* Strip any trailing space or tab */
   for (j = 0; j < strlen(value) && value[j] != ';'; ++j);
   --j;
   while (j > 0 && value[j] <= ' ') --j;
   value[++j] = '\0';		                /* truncate it               */
   sscanf(value, "%ld", &numval);		/* get any numeric value     */
   charval = value[0];				/* get any letter  value     */

   /* Do associated action */
   for (k = 0; ITAB[sec][k].vartype != ENDING; ++k)
   {
      if (0 == strcmp(ITAB[sec][k].varname, variable))
      {
         switch (ITAB[sec][k].vartype)
         {
            case STRING:
               strcpy((char *)(ITAB[sec][k].value), value);
               break;
            case NUMVAL:
               if (numval < ITAB[sec][k].minval 
                  || numval > ITAB[sec][k].maxval)
               {
                  char aux[MAXBUFFERLEN];

                  sprintf(aux, "Value of '%s' must be ", variable);
                  if (ITAB[sec][k].minval != -LINF)
                     sprintf(aux + strlen(aux), "more than %ld", 
                        ITAB[sec][k].minval - 1);
                  if (ITAB[sec][k].minval != -LINF 
                     && ITAB[sec][k].maxval != LINF)
                        sprintf(aux + strlen(aux), " and " );
                  if (ITAB[sec][k].maxval != LINF)
                     sprintf(aux + strlen(aux), "less than %ld",
                        ITAB[sec][k].minval + 1);
                  sprintf(aux + strlen(aux), " in file '%s'.", fininame);
                  ExitProg(aux );
               }
               *((long *)(ITAB[sec][k].value)) = numval;
               break;
            case CHRVAL:
               *((char *)(ITAB[sec][k].value)) = charval;
               break;
            case YES_NO:
               StrLower(value);
               if 
                  (0 == strcmp(value, "yes") || charval == 'y') 
		  {
                     *((int *)(ITAB[sec][k].value)) = 1;
                  }
               else if 
                  (0 == strcmp(value, "no")  || charval == 'n') 
		  {
                     *((int *)(ITAB[sec][k].value)) = 0;
                  }
               else
                  ExitProg("Bad init value '%s' in file '%s'.", param,
                     fininame);
               break; 
            default:
               ExitProg("Bad init argument '%s' in file '%s'.", param, 
                     fininame);
         }
         break;
      }
   }
   if (ITAB[sec][k].vartype == ENDING) 
   {
      ExitProg("Argument unknown '%s' in file '%s'.", param, fininame);
   }
}

/*---------------------------------------------------------------------------*
 NAME      :
 PURPOSE   : Processes a single command-file. If (inifile), it looks
	     for '[...]' codes as well
 PARAMETERS:
 RETURN    : None
 *---------------------------------------------------------------------------*/
void CmdFile(FILE *fp)
{
   char	line[MAXBUFFERLEN + 1];
   int	toolsection = 1;
   int	i, j, k;

   while (fgets(line, MAXBUFFERLEN, fp) != NULL)
   {          					/* read thru a line at a time*/
      /* Strip trailing \n */
      i = strlen(line);
      if (i > 0) { if (line[i - 1] == '\n') line[i - 1] = '\0'; }
      else line[i + 1] = 0;			/* add second null   	     */

      /* Is it a section header? */
      if (line[0] == '[')
      {
	 toolsection = 0;
	 i = strchr(line, ']') - line - 1;
         for (k = 0; k < ITABdim; ++k)
         {
            if (0 == strncmp(line + 1, ITAB[k][0].varname, i))
            {
               toolsection = k + 1; break;
            }
         }
	 continue;				/* skip this line in any case*/
      }

      if (! toolsection) continue;		/* not our section	     */

      i = -1;					/* get a running start	     */
      while (line[++i] != '\0')
      {		                      		/* scan through the line     */
	 if (line[i] <= ' ') continue;          /* white space or tabs	     */
	 if (line[i] == ';') break;             /* comments		     */
	 j = i;	        			/* argument starts here	     */
	 while (line[++i] != '\0');		/* find the argument end     */
	 CmdArg(&line[j], toolsection - 1);	/* process the argument      */
      }
   }
}

/*---------------------------------------------------------------------------*
 NAME      :
 PURPOSE   :
 PARAMETERS:
 RETURN    :
 *---------------------------------------------------------------------------*/
char *StrLower(char *s)
{
   char *p = s;

   while (*p != '\0') { *p = tolower(*p); ++p; }
   return (s);
}

/*---------------------------------------------------------------------------*
 NAME      :
 PURPOSE   :
 PARAMETERS: None
 RETURN    : None
 *---------------------------------------------------------------------------*/
int Display(char *fmt, ...)
{
   va_list	ap;
   int		res;

   va_start(ap, fmt);
   if (errcod == 0) {
      vsprintf(buf, fmt, ap);
      res = printf("%s\n", buf);
      if (fplog) res = fprintf(fplog, "%s\n", buf);
   }
   va_end(ap);

   return (res);
}

/*---------------------------------------------------------------------------*
 NAME      :
 PURPOSE   :
 PARAMETERS:
 RETURN    : None
 *---------------------------------------------------------------------------*/
int DisplayOut(char *fmt, ...)
{
   va_list	ap;
   int		res = -1;

   va_start(ap, fmt);
   vsprintf(bout, fmt, ap);
   printf("%s\n", bout);
   if (fplog) fprintf(fplog, "%s\n", bout);
/*
   if (fpout) res = fprintf(fpout, "%s\n", bout);
*/
   va_end(ap);

   return (res);
}

/*---------------------------------------------------------------------------*
 NAME      :
 PURPOSE   :
 PARAMETERS:
 RETURN    : None
 *---------------------------------------------------------------------------*/
int Out(char *fmt, ...)
{
   va_list	ap;
   int		res = -1;

   va_start(ap, fmt);
   vsprintf(bout, fmt, ap);
   if (fplog) fprintf(fplog, "%s\n", bout);
/*
   if (fpout) res = fprintf(fpout, "%s\n", bout);
*/
   va_end(ap);

   return (res);
}

/*---------------------------------------------------------------------------*
 NAME      :
 PURPOSE   :
 PARAMETERS:
 RETURN    : None
 *---------------------------------------------------------------------------*/
int Log(char *fmt, ...)
{
   va_list	ap;
   int		res = -1;

   va_start(ap, fmt);
   vsprintf(blog, fmt, ap);
   if (fplog) res = fprintf(fplog, "%s\n", blog);
   va_end(ap);

   return (res);
}

/*---------------------------------------------------------------------------*
 NAME      :
 PURPOSE   :
 PARAMETERS:
 RETURN    : None
 *---------------------------------------------------------------------------*/
int FileOut(FILE *fp, char *fmt, ...)
{
   va_list	ap;
   int		res = -1;

   va_start(ap, fmt);
   vsprintf(buf, fmt, ap);
   if (fp) fprintf(fp, "%s\n", buf);
/*
   if (fpout) res = fprintf(fpout, "%s\n", buf);
*/
   if (fplog) res = fprintf(fplog, "%s\n", buf);
   va_end(ap);

   return (res);
}

/*---------------------------------------------------------------------------*
 NAME      :
 PURPOSE   :
 PARAMETERS: None
 RETURN    : None
 *---------------------------------------------------------------------------*/
void *Malloc(unsigned long size)
{
   void *p;

   p = malloc((size_t)size);
   if (p != NULL) cur_allocated += size;
   if (cur_allocated >= max_allocated) max_allocated = cur_allocated;

   if (p == NULL) 
      ExitProg("Failed to allocate %ldK (%ldK).", 
         size >> 10, max_allocated >> 10);
   ++calls_to_Malloc;

   /* Initialize to zero the allocated memory */
   bzero(p, size);

   return(p);
}

/*---------------------------------------------------------------------------*
 NAME      :
 PURPOSE   :
 PARAMETERS: None
 RETURN    : None
 *---------------------------------------------------------------------------*/
void *Calloc(unsigned long nmemb, unsigned long size)
{
   void *p;

   p = calloc((size_t)nmemb, (size_t)size);
   if (p != NULL) cur_allocated += size * nmemb;
   if (cur_allocated >= max_allocated) max_allocated = cur_allocated;

   if (p == NULL) 
      ExitProg("Failed to allocate %ldK (%ldK).", 
         size >> 10, max_allocated >> 10);
   ++calls_to_Malloc;

   return(p);
}

/*---------------------------------------------------------------------------*
 NAME      :
 PURPOSE   :
 PARAMETERS: None
 RETURN    : None
 *---------------------------------------------------------------------------*/
void Free(void *block)
{
    if (block != NULL) {
       ++calls_to_Free;
       /* for now don't decrement cur_allocated */
       free(block);
    }
}

/*---------------------------------------------------------------------------*
 NAME      :
 PURPOSE   :
 PARAMETERS: None
 RETURN    : 0 if successful, -1 otherwise
 *---------------------------------------------------------------------------*/
int TransformLog(void)
{
   /* Make sure that flogname is set */
   if (*flogname == '\0')
   {
      strncpy(flogname, App[0], MAXFPNAMELEN - 5);
      flogname[MAXFPNAMELEN - 6] = '\0';
      strcat(flogname, ".");
      strcat(flogname, FLOG_EXTENSION);
   }

   /* Remove eventual old log file */
   if (EACCES == remove(flogname))
   {
      sprintf(errmsg, "Permission denied removing old log file '%s'.",
	 flogname);
      return (-1);
   }

   /* Close current log file whose name is temporary to flush buffers */
   if (fplog != NULL) fclose(fplog);

   /* Create new log file now that i have its name */
   if (NULL == (fplog = fopen(flogname, "wt")))
   {
      sprintf(errmsg, "Cannot create log file '%s'.", flogname);
      return (-1);
   }

   /* Reopen temporary log file */
   if (NULL == (fptmp = fopen(tlogname, "rt")))
   {
      sprintf(errmsg, "Cannot reopen tmp log file '%s'.", tlogname);
      return (-1);
   }

   /* Copy temporary content to new log file */
   while (NULL != fgets(buf, MAXBUFFERLEN - 1, fptmp)) fputs(buf, fplog);

   /* Close temporary file and remove it */
   if (fptmp != NULL) fclose(fptmp);
   if (EACCES == remove(tlogname))
   {
      sprintf(errmsg, "Permission denied removing tmp log file '%s'.",
	 tlogname);
      return (-1);
   }
   tlogname[0] = '\0';

   return (0);
}

/*---------------------------------------------------------------------------*
 NAME      :
 PURPOSE   :
 PARAMETERS:
 RETURN    : 
 *---------------------------------------------------------------------------*/
void GenIniTemplate(void)
{
   int sec, k;
   char aux[MAXLINELEN];
   char fn[MAXFILENAMELEN];
   FILE *fp;

   strncpy(fn, App[0], MAXFPNAMELEN - 5);
   fn[MAXFPNAMELEN - 6] = '\0';
   strcat(fn, ".");
   strcat(fn, "tem");
   if (NULL == (fp = fopen(fn, "wt"))) {
      ExitProg("Cannot create template file '%s'.", fn);
   }

   for (sec = 0; sec < ITABdim; ++sec)
   {
      fprintf(fp, "[%s]\n", ITAB[sec][0].varname);
      for (k = 1; ITAB[sec][k].vartype != ENDING; ++k) {
         fprintf(fp, "%-22s = %s\n", ITAB[sec][k].varname, 
            StrVal(aux, MAXLINELEN, sec, k));
      }
      fprintf(fp, "\n");
   }

   fclose(fp);

   printf("Template file is %s\n", fn);
}

/*---------------------------------------------------------------------------*
 NAME      :
 PURPOSE   :
 PARAMETERS:
 RETURN    : 
 *---------------------------------------------------------------------------*/
void LogIniParm(void)
{
   int sec, k;
   char aux[MAXLINELEN];

   Display("\nLIST OF INITALIZATION PARAMETERS");
   for (sec = 0; sec < ITABdim; ++sec)
   {
      Display("--------------------------------------------[%s]", 
         ITAB[sec][0].varname);
      for (k = 1; ITAB[sec][k].vartype != ENDING; ++k)
         Display("%-22s = %s", ITAB[sec][k].varname, 
            StrVal(aux, MAXLINELEN, sec, k));
   }
   Display("--------------------------------------------"); 
}

/*---------------------------------------------------------------------------*
 NAME      :
 PURPOSE   :
 PARAMETERS:
 RETURN    : 
 *---------------------------------------------------------------------------*/
char *StrVal(char *s, int n, int sec, int k)
{
   switch(ITAB[sec][k].vartype)
   {
      case STRING:
         strncpy(s, (char *)(ITAB[sec][k].value), n - 1);
	 s[MAXLINELEN - 1] = '\0';
	 break;
      case NUMVAL:
	 sprintf(s, "%d", *((int *)(ITAB[sec][k].value)));
	 break;
      case LNUMVAL:
	 sprintf(s, "%ld", *((long *)(ITAB[sec][k].value)));
	 break;
      case CHRVAL:
	 sprintf(s, "%c", *((char *)(ITAB[sec][k].value)));
	 break;
      case YES_NO:
	 sprintf(s, "%s", *((int *)(ITAB[sec][k].value)) ? "yes" : "no ");
	 break;
      default:
	 s[0] = '\0';
	 break;
   }
   return (s);
}

/*---------------------------------------------------------------------------*
 NAME      :
 PURPOSE   :
 PARAMETERS:
 RETURN    : 
 *---------------------------------------------------------------------------*/
FILE *FOpen(const char *fname, const char *mode, int (**fc)(FILE *))
{
   FILE *fp;
   char GZcmd[2 * MAXFPNAMELEN];
   FILE *(*fo)(const char *, const char *);

   /* Verify if input file can be open */
   if (NULL == (fp = fopen(fname, mode)))
   {
      perror("FOpen");
      ExitProg("Cannot open file '%s'.", fname);
   }
   else
   {
      fclose(fp);
   }

   GZcmd[0] = '\0';
   if (0 == strcmp(fname + strlen(fname) - 2, "gz"))
   {
      #ifdef __BORLANDC__
      ExitProg("POpen not supported, .gz must be expanded.");
      #else
      if (0 == strcmp(mode, "r"))
         strcpy(GZcmd, "exec gzip -dc ");
      else
         strcpy(GZcmd, "exec gzip -c >");
      strncat(GZcmd, fname, 2*MAXFPNAMELEN-1); ; GZcmd[2*MAXFPNAMELEN-1]='\0';
      strncat(GZcmd, NULDEV, 2*MAXFPNAMELEN-1); ; GZcmd[2*MAXFPNAMELEN-1]='\0';
   
      fo = POPEN;
      *fc = PCLOSE;
      #endif
   }
   else if (0 == strcmp(fname + strlen(fname) - 3, "bz2"))
   {
      #ifdef __BORLANDC__
      ExitProg("POpen not supported, .bz2 must be expanded.");
      #else
      if (0 == strcmp(mode, "r"))
         strcpy(GZcmd, "exec bzip2 -dc ");
      else
         strcpy(GZcmd, "exec bzip2 -c >");
      strncat(GZcmd, fname, 2*MAXFPNAMELEN-1); ; GZcmd[2*MAXFPNAMELEN-1]='\0';
      strncat(GZcmd, NULDEV, 2*MAXFPNAMELEN-1); ; GZcmd[2*MAXFPNAMELEN-1]='\0';
   
      fo = POPEN;
      *fc = PCLOSE;
      #endif
   }
   else
   {
      strncat(GZcmd, fname, 2*MAXFPNAMELEN-1); ; GZcmd[2*MAXFPNAMELEN-1]='\0';
      fo = fopen;
      *fc = fclose;
   }

   /* Open file */
   if (NULL == (fp = fo(GZcmd, mode)))
   {
      perror("FOpen");
      ExitProg("Cannot do '%s'.", GZcmd);
   }
   Log("FOpen: '%s'.", GZcmd);

   return (fp);
}

/*---------------------------------------------------------------------------*
 NAME      :
 PURPOSE   : Get the i-th token into a string list
 PARAMETERS:
 RETURN    : 1 if token found, 0 if not found
 *---------------------------------------------------------------------------*/
int GetToken(char *str, int i, char *buf)
{
   char aux[MAXFPNAMELEN];
   char *p;
   int j = 0, r = 0;

   strcpy(aux, str);
   p = aux;
   if (*p != '{') {
      if (i == 1) {
         strcpy(buf, p); r = 1;
      }
   } else {
      p = strtok(aux, "{,}");
      for (j = 1; j < i && p != NULL; ++j) {
         p = strtok(NULL, ",}");
      }
      if (j == i && p != NULL) { strcpy(buf, p); r = 1; }
   }

   return (r);
}

/*---------------------------------------------------------------------------*
 NAME      :
 PURPOSE   : Check if string is a numeric int value
 PARAMETERS:
 RETURN    : 1 if found and v contains the value, 0 otherwise
 *---------------------------------------------------------------------------*/
int IsNumVal(char *str, int *v)
{
   char *c = str;
   int r = 0, l1, l2 = 0;
   uint t = 0;
   int sign = 1;
   
   l1 = strlen(str);
   while ((*c < '0' || *c > '9') && (*c != 0)) {
      if (*c == '-') { sign = -1; ++l2; } 
      if (*c == '+') { sign = 1; ++l2; }
      ++c;
   }
   while (*c >= '0' && *c <= '9')
   {
      t = t * 10 + *c - '0';
      ++c;
      ++l2;
   }
   if (l1 == l2 && l2 != 0) { r = 1; *v = sign * t; }

   return (r);
}

/*---------------------------------------------------------------------------*
 NAME      :
 PURPOSE   : Check if string is a numeric unsigned int value
 PARAMETERS:
 RETURN    : 1 if found and v contains the value, 0 otherwise
 *---------------------------------------------------------------------------*/
int IsUNumVal(char *str, uint *v)
{
   char *c = str;
   int r = 0, l1, l2 = 0;
   uint t = 0;
   
   l1 = strlen(str);
   while ((*c < '0' || *c > '9') && (*c != 0)) ++c;
   while (*c >= '0' && *c <= '9')
   {
      t = t * 10 + *c - '0';
      ++c;
      ++l2;
   }
   if (l1 == l2 && l2 != 0) { r = 1; *v = t; }

   return (r);
}

/*---------------------------------------------------------------------------*
 NAME      :
 PURPOSE   : Check if string is a numeric int value
 PARAMETERS:
 RETURN    : 1 if found and v contains the value, 0 otherwise
 *---------------------------------------------------------------------------*/
int IsLNumVal(char *str, ulng *v)
{
   char *c = str;
   int r = 0, l1, l2 = 0;
   ulng t = 0;
   
   l1 = strlen(str);
   while ((*c < '0' || *c > '9') && (*c != 0)) ++c;
   while (*c >= '0' && *c <= '9')
   {
      t = t * 10 + *c - '0';
      ++c;
      ++l2;
   }
   if (l1 == l2 && l2 != 0) { r = 1; *v = t; }

   return (r);
}

/*---------------------------------------------------------------------------*
 NAME      :
 PURPOSE   :
 PARAMETERS:
 RETURN    :
 *---------------------------------------------------------------------------*/
void Mysysopen(char *strin)
{
#ifdef __EMSCRIPTEN__
  // In WebAssembly (Node.js or Browser), no system() available.
  // Directly open the input file as Mysysfp.
  Mysysfp = fopen("/program-ex1", "r"); // hard-coded embedded file
  if (Mysysfp == NULL) {
    ExitProg("Mysysopen: Failed to open /program-ex1 in Emscripten virtual FS.");
  }
#else
  char aaa[10*MAXLINELEN];
  int res1, res2;

  strcpy(Mysystn, "suigenXXXXXX");
  res1 = mkstemp(Mysystn);
  if (-1 == res1)
    ExitProg("Mysysopen: Cannot create tmp file through mkstemp().");

  sprintf(aaa, "%s; set| cat >>%s", strin, Mysystn);
  res2 = system(aaa);
  Mysysfp = fdopen(res1, "rt");
#endif
}

/*---------------------------------------------------------------------------*
 NAME      :
 PURPOSE   :
 PARAMETERS:
 RETURN    :
 *---------------------------------------------------------------------------*/
void Mysysclose(void)
{
   if (Mysysfp != NULL) fclose(Mysysfp);
#ifndef __EMSCRIPTEN__
   unlink(Mysystn);
#endif
}

/*---------------------------------------------------------------------------*
 NAME      :
 PURPOSE   : resu is assigned only if the key is found
 PARAMETERS:
 RETURN    :
 *---------------------------------------------------------------------------*/
void Mysyspars(char *resu, char *key)
{
   char ccc[10*MAXLINELEN];
   char vvv[10*MAXLINELEN];
   char aux[10*MAXLINELEN];
   char c[2] = "x";
   size_t cnt;

   /* printf("SUI: key=%s\n", key); */
   aux[0] = '\0';
   fseek(Mysysfp, 0L, SEEK_SET);
//   while (!feof(Mysysfp)) {
//      cnt = fread(c, 1, 1, Mysysfp);
//      if (cnt != 1)
//         ExitProg("Mysyspars: Error while reading from Mysysfp.");
    while ((cnt = fread(c, 1, 1, Mysysfp)) == 1) {
      if (*c != '=' && *c != '\n') {
         strcat(aux, c);
      }
      if (*c == '=') {
         strcpy(ccc, aux);
         aux[0] = '\0';
      }
      if (*c == '\n') {
         strcpy(vvv, aux);
         aux[0] = '\0';
         if (0 == strcmp(key, ccc)) {
            strcpy(resu, vvv);
            /* printf("SUI: val=%s\n", vvv); */
            break;
         }
      }
   }
}
