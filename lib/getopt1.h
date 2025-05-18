/*-----------------------------------------------------------------------------
FILE        : GETOPT1.H
INFO        : Interface to getopt
DEVELOPEMENT: GCC 2.6.3 && Borland C++ 3.1, Model=Large, ANSI-Keywords
CREATED BY  : RG[28-Apr-1995]
MODIFIED BY : XX[XX-XXX-XXXX]
-----------------------------------------------------------------------------*/
#ifndef	_GETOPT1_H
#define	_GETOPT1_H

/*------------- GLOBAL DEFINITIONS ------------------------------------------*/
#include "getopt.h"

int getopt_long (int argc, char *const *argv, const char *options,
   const struct option *long_options, int *opt_index);

int getopt_long_only (int argc, char *const *argv, const char *options,
   const struct option *long_options, int *opt_index);


#endif  /* _GETOPT1_H */
