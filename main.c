/*-----------------------------------------------------------------------------
FILE        : MAIN.C
INFO        : Free Superscalar Simulator - Main program file for FREESS
PURPOSE     : Educational Tool
DEVELOPEMENT: GCC 2.7.0
CREATED BY  : RG[14-May-2009]
MODIFIED BY : XX[XX-XXX-XXXX]
-----------------------------------------------------------------------------*/
/* Automatically generated by Tool Builder Interface V1.0. */
/* File: main.c   (Don't edit this file.) */

/*------------- LIBRARY PROCEDURES ------------------------------------------*/
#include "sui.h"
#include "getopt1.h"
#include "superscalar.h"
#include "main.h"

/*------------- GENERAL DEFINITIONS -----------------------------------------*/
#define PROG_NAME       "freess"
#define PROG_PRESEN     \
   "Free Superscalar Simulator - Copyright Roberto Giorgi - giorgi@acm.org\n"\
   "v0.1 - Released 01-Nov-120 "
#define PROG_PRESEN2    \
   "FREESS v0.1 - Released 01-Nov-120 "
#define PROG_ULINE      \
   "-------------------------------------------------------------------"

/*------------- GLOBAL VARIABLES --------------------------------------------*/
/* Structure that I pass to SUI */
static const char *freess_app[] =
{
   PROG_NAME,
   PROG_PRESEN,
   PROG_ULINE
};

/* The version of program */
const char *version_string = PROG_PRESEN;

/* If nonzero, ... */
static int autox;
int Tracing;
int ETracing;
int verbose;   /* I need THIS in TRACE FACTORY tools */
int debug;     /* I need THIS in TRACE FACTORY tools */
static int template;
static int nolog;
static int logga;

/* If non-zero, display usage information and exit.  */
static int show_help;

/* If non-zero, print the version on standard output and exit.  */
static int show_version;

/* User selected name of .ini file */
static char *inifile = NULL;

/* Declare option-variables */
static char	clv1[MAXLINELEN] = "program";
static char	clv2[MAXLINELEN] = "no";
static char	clv3[MAXLINELEN] = "yes";
static char	clv4[MAXLINELEN] = "no";
static char	clv5[MAXLINELEN] = "3";
static char	clv6[MAXLINELEN] = "0";
static char	clv7[MAXLINELEN] = "8";
static char	clv8[MAXLINELEN] = "24";
static char	clv9[MAXLINELEN] = "FDPIXWC";
static char	clv10[MAXLINELEN] = "yes";
static char	clv11[MAXLINELEN] = "no";
static char	clv12[MAXLINELEN] = "no";
static char	clv13[MAXLINELEN] = "yes";
static char	clv14[MAXLINELEN] = "4";
static char	clv15[MAXLINELEN] = "4";
static char	clv16[MAXLINELEN] = "4";
static char	clv17[MAXLINELEN] = "4";
static char	clv18[MAXLINELEN] = "4";
static char	clv19[MAXLINELEN] = "4";
static char	clv20[MAXLINELEN] = "16";
static char	clv21[MAXLINELEN] = "16";
static char	clv22[MAXLINELEN] = "4";
static char	clv23[MAXLINELEN] = "0";
static char	clv24[MAXLINELEN] = "1";
static char	clv25[MAXLINELEN] = "4";
static char	clv26[MAXLINELEN] = "yes";
static char	clv27[MAXLINELEN] = "4";
static char	clv28[MAXLINELEN] = "1";
static char	clv29[MAXLINELEN] = "1";
static char	clv30[MAXLINELEN] = "2";
static char	clv31[MAXLINELEN] = "yes";
static char	clv32[MAXLINELEN] = "1";
static char	clv33[MAXLINELEN] = "1";
static char	clv34[MAXLINELEN] = "yes";
static char	clv35[MAXLINELEN] = "1";
static char	clv36[MAXLINELEN] = "0";
static char	clv37[MAXLINELEN] = "1";
static char	clv38[MAXLINELEN] = "1";
static char	clv39[MAXLINELEN] = "yes";
static char	clv40[MAXLINELEN] = "0";
static char	clv41[MAXLINELEN] = "def.log";


static struct option long_options[] =
{
  {"inifile", required_argument, NULL, 0},
  {"auto", no_argument, &autox, 1},
  {"tracing", no_argument, &Tracing, 1},
  {"etracing", no_argument, &ETracing, 1},
  {"verbose", no_argument, &verbose, 1},
  {"debug", required_argument, NULL, 0},
  {"template", no_argument, &template, 1},
  {"log", no_argument, &logga, 1},
  {"help", no_argument, &show_help, 1},
  {"version", no_argument, &show_version, 1},
  {"exe", required_argument, NULL, 0},
  {"s", required_argument, NULL, 0},
  {"int", required_argument, NULL, 0},
  {"batch", required_argument, NULL, 0},
  {"iter", required_argument, NULL, 0},
  {"startck", required_argument, NULL, 0},
  {"lregs", required_argument, NULL, 0},
  {"pregs", required_argument, NULL, 0},
  {"pstruct", required_argument, NULL, 0},
  {"unilsu", required_argument, NULL, 0},
  {"ioi", required_argument, NULL, 0},
  {"ioc", required_argument, NULL, 0},
  {"unidi", required_argument, NULL, 0},
  {"fw", required_argument, NULL, 0},
  {"dw", required_argument, NULL, 0},
  {"pw", required_argument, NULL, 0},
  {"iw", required_argument, NULL, 0},
  {"ww", required_argument, NULL, 0},
  {"cw", required_argument, NULL, 0},
  {"wins", required_argument, NULL, 0},
  {"robs", required_argument, NULL, 0},
  {"afu", required_argument, NULL, 0},
  {"alat", required_argument, NULL, 0},
  {"mfu", required_argument, NULL, 0},
  {"mlat", required_argument, NULL, 0},
  {"mpipe", required_argument, NULL, 0},
  {"ffu", required_argument, NULL, 0},
  {"xfu", required_argument, NULL, 0},
  {"lfu", required_argument, NULL, 0},
  {"llat", required_argument, NULL, 0},
  {"lpipe", required_argument, NULL, 0},
  {"sfu", required_argument, NULL, 0},
  {"slat", required_argument, NULL, 0},
  {"spipe", required_argument, NULL, 0},
  {"bfu", required_argument, NULL, 0},
  {"blat", required_argument, NULL, 0},
  {"lqs", required_argument, NULL, 0},
  {"sqs", required_argument, NULL, 0},
  {"spec", required_argument, NULL, 0},
  {"wblat", required_argument, NULL, 0},
  {"logfile", required_argument, NULL, 0},

  {NULL, 0, NULL, 0}
};

General				G;
Architecture			AA;
Program_Defaults		Pro;

static IniSection GeneralSec[] =
{
   "General",				HEADER,	0,	0,	NULL,
   "Program Name",			STRING,	0,	0,	&G.progname,
   "Silent",				YES_NO,	0,	0,	&G.silent,
   "Interactive",			YES_NO,	0,	0,	&G.interactive,
   "Mode",				YES_NO,	0,	0,	&G.batch,
   "Iterations",			NUMVAL,	1,	0,	&G.iterations,
   "Starting Cycle",			NUMVAL,	-100,	100,	&G.start_ck,
   "",					ENDING,	0,	0,	NULL
};

static IniSection ArchitectureSec[] =
{
   "Architecture",			HEADER,	0,	0,	NULL,
   "Logical Registers",			NUMVAL,	1,	MAXPHYSREGS,	&AA.lregs,
   "Physical Registers",		NUMVAL,	1,	MAXPHYSREGS,	&AA.pregs,
   "Pipeline Structure",		STRING,	0,	0,	&AA.pipestruct,
   "Unified LSU",			YES_NO,	0,	0,	&AA.uni_lsu,
   "In-Order Issue",			YES_NO,	0,	0,	&AA.io_issue,
   "In-Order Complete",			YES_NO,	0,	0,	&AA.io_complete,
   "Unified Dispatch/Issue",		YES_NO,	0,	0,	&AA.uni_di,
   "Fetch Width",			NUMVAL,	1,	MAXISSUEWIDTH,	&AA.f_width,
   "Decode Width",			NUMVAL,	1,	MAXISSUEWIDTH,	&AA.d_width,
   "Dispatch Width",			NUMVAL,	1,	MAXISSUEWIDTH,	&AA.p_width,
   "Issue Width",			NUMVAL,	1,	MAXISSUEWIDTH,	&AA.i_width,
   "Write-Back Width",			NUMVAL,	1,	MAXISSUEWIDTH,	&AA.w_width,
   "Commit Width",			NUMVAL,	1,	MAXROBSIZE,	&AA.c_width,
   "Window Size",			NUMVAL,	1,	MAXWINDOWSIZE,	&AA.win_size,
   "ROB Size",				NUMVAL,	1,	MAXROBSIZE,	&AA.rob_size,
   "Integer ALU Units",			NUMVAL,	1,	MAXINTFU,	&AA.int_fu,
   "Integer ALU Latency",		NUMVAL,	0,	MAXLATENCY,	&AA.a_lat,
   "Integer Mult. Units",		NUMVAL,	1,	MAXINTFU,	&AA.im_fu,
   "Integer Mult. Latency",		NUMVAL,	1,	MAXLATENCY,	&AA.im_lat,
   "Integer Mult. Pipe",		YES_NO,	0,	0,	&AA.im_pipe,
   "Floating Point Units",		NUMVAL,	1,	MAXFPFU,	&AA.fp_fu,
   "Floating Point Mult",		NUMVAL,	1,	MAXFPFU,	&AA.fm_fu,
   "Load Units",			NUMVAL,	1,	MAXLFU,	&AA.l_fu,
   "Load Latency",			NUMVAL,	1,	MAXLATENCY,	&AA.l_lat,
   "Load Pipe",				YES_NO,	0,	0,	&AA.l_pipe,
   "Store Units",			NUMVAL,	1,	MAXSFU,	&AA.s_fu,
   "Store Latency",			NUMVAL,	1,	MAXLATENCY,	&AA.s_lat,
   "Store Pipe",			YES_NO,	0,	0,	&AA.s_pipe,
   "Branch Units",			NUMVAL,	1,	MAXBFU,	&AA.b_fu,
   "Branch Latency",			NUMVAL,	0,	MAXLATENCY,	&AA.b_lat,
   "Load Queue Size",			NUMVAL,	1,	MAXLQSIZE,	&AA.lqsize,
   "Store Queue Size",			NUMVAL,	1,	MAXSQSIZE,	&AA.sqsize,
   "Speculation",			YES_NO,	0,	0,	&AA.speculation,
   "Write Back Latency",		NUMVAL,	1,	0,	&AA.wblat,
   "",					ENDING,	0,	0,	NULL
};

static IniSection Program_DefaultsSec[] =
{
   "Program Defaults",			HEADER,	0,	0,	NULL,
   "Log File Name",			STRING,	0,	0,	&Pro.logfile,
   "",					ENDING,	0,	0,	NULL
};



static char rfn[MAXFILENAMELEN];
static char iniv1[12*MAXLINELEN];
static char iniv[10*MAXLINELEN];
static char aux1[MAXFILENAMELEN];

/*------------- INTERNAL FUNCTIONS PROTOTYPES -------------------------------*/
static  int     MainConstr(int argc, char **argv);
static  void    MainDestru(void);
static  void    Usage(int status);

/*------------- MAIN PROGRAM ------------------------------------------------*/
int main(int argc, char **argv)
{
   MainConstr(argc, argv);
   Superscalar__Start(argc, argv);
   ExitProg("-Goodbye.");
   return (0);
}

/*------------- FUNCTION IMPLEMENTATION -------------------------------------*/
/*---------------------------------------------------------------------------*
 NAME      :
 PURPOSE   :
 PARAMETERS: None
 RETURN    : None
 *---------------------------------------------------------------------------*/
int MainConstr(int argc, char **argv)
{
   int  c, usage = 0, ok = 0, option_index;
   FILE *fp;
   char fn[MAXFILENAMELEN];

   /* Scan command line */
   while (1) {
      option_index = 0;

      c = getopt_long_only(argc, argv, "i:vd", long_options, &option_index);
      if (c == EOF) break;

      switch (c) {
      case 0: /* Long-named option. */
         if (strcmp("verbose", long_options[option_index].name) == 0)
            if (optarg) { ++verbose; ok = 1; }
         if (strcmp("debug", long_options[option_index].name) == 0)
            if (optarg) { debug = atoi(optarg); ok = 1; }
         if (strcmp("inifile", long_options[option_index].name) == 0)
            if (optarg) { inifile = optarg; ok = 1; }
         if (strcmp("template", long_options[option_index].name) == 0)
            { logga = 0; ok = 1; }
         if (strcmp("auto", long_options[option_index].name) == 0)
            { autox = 1; ok = 1; }
         if (strcmp("tracing", long_options[option_index].name) == 0)
            { Tracing = 1; ok = 1; }
         if (strcmp("etracing", long_options[option_index].name) == 0)
            { ETracing = 1; Tracing = 1; ok = 1; }
         if (strcmp("log", long_options[option_index].name) == 0)
            { logga = 1; ok = 1; }
         if (strcmp("exe", long_options[option_index].name) == 0)
            if (optarg) { strcpy(clv1, optarg); ok = 1; }
         if (strcmp("s", long_options[option_index].name) == 0)
            if (optarg) { strcpy(clv2, optarg); ok = 1; }
         if (strcmp("int", long_options[option_index].name) == 0)
            if (optarg) { strcpy(clv3, optarg); ok = 1; }
         if (strcmp("batch", long_options[option_index].name) == 0)
            if (optarg) { strcpy(clv4, optarg); ok = 1; }
         if (strcmp("iter", long_options[option_index].name) == 0)
            if (optarg) { strcpy(clv5, optarg); ok = 1; }
         if (strcmp("startck", long_options[option_index].name) == 0)
            if (optarg) { strcpy(clv6, optarg); ok = 1; }
         if (strcmp("lregs", long_options[option_index].name) == 0)
            if (optarg) { strcpy(clv7, optarg); ok = 1; }
         if (strcmp("pregs", long_options[option_index].name) == 0)
            if (optarg) { strcpy(clv8, optarg); ok = 1; }
         if (strcmp("pstruct", long_options[option_index].name) == 0)
            if (optarg) { strcpy(clv9, optarg); ok = 1; }
         if (strcmp("unilsu", long_options[option_index].name) == 0)
            if (optarg) { strcpy(clv10, optarg); ok = 1; }
         if (strcmp("ioi", long_options[option_index].name) == 0)
            if (optarg) { strcpy(clv11, optarg); ok = 1; }
         if (strcmp("ioc", long_options[option_index].name) == 0)
            if (optarg) { strcpy(clv12, optarg); ok = 1; }
         if (strcmp("unidi", long_options[option_index].name) == 0)
            if (optarg) { strcpy(clv13, optarg); ok = 1; }
         if (strcmp("fw", long_options[option_index].name) == 0)
            if (optarg) { strcpy(clv14, optarg); ok = 1; }
         if (strcmp("dw", long_options[option_index].name) == 0)
            if (optarg) { strcpy(clv15, optarg); ok = 1; }
         if (strcmp("pw", long_options[option_index].name) == 0)
            if (optarg) { strcpy(clv16, optarg); ok = 1; }
         if (strcmp("iw", long_options[option_index].name) == 0)
            if (optarg) { strcpy(clv17, optarg); ok = 1; }
         if (strcmp("ww", long_options[option_index].name) == 0)
            if (optarg) { strcpy(clv18, optarg); ok = 1; }
         if (strcmp("cw", long_options[option_index].name) == 0)
            if (optarg) { strcpy(clv19, optarg); ok = 1; }
         if (strcmp("wins", long_options[option_index].name) == 0)
            if (optarg) { strcpy(clv20, optarg); ok = 1; }
         if (strcmp("robs", long_options[option_index].name) == 0)
            if (optarg) { strcpy(clv21, optarg); ok = 1; }
         if (strcmp("afu", long_options[option_index].name) == 0)
            if (optarg) { strcpy(clv22, optarg); ok = 1; }
         if (strcmp("alat", long_options[option_index].name) == 0)
            if (optarg) { strcpy(clv23, optarg); ok = 1; }
         if (strcmp("mfu", long_options[option_index].name) == 0)
            if (optarg) { strcpy(clv24, optarg); ok = 1; }
         if (strcmp("mlat", long_options[option_index].name) == 0)
            if (optarg) { strcpy(clv25, optarg); ok = 1; }
         if (strcmp("mpipe", long_options[option_index].name) == 0)
            if (optarg) { strcpy(clv26, optarg); ok = 1; }
         if (strcmp("ffu", long_options[option_index].name) == 0)
            if (optarg) { strcpy(clv27, optarg); ok = 1; }
         if (strcmp("xfu", long_options[option_index].name) == 0)
            if (optarg) { strcpy(clv28, optarg); ok = 1; }
         if (strcmp("lfu", long_options[option_index].name) == 0)
            if (optarg) { strcpy(clv29, optarg); ok = 1; }
         if (strcmp("llat", long_options[option_index].name) == 0)
            if (optarg) { strcpy(clv30, optarg); ok = 1; }
         if (strcmp("lpipe", long_options[option_index].name) == 0)
            if (optarg) { strcpy(clv31, optarg); ok = 1; }
         if (strcmp("sfu", long_options[option_index].name) == 0)
            if (optarg) { strcpy(clv32, optarg); ok = 1; }
         if (strcmp("slat", long_options[option_index].name) == 0)
            if (optarg) { strcpy(clv33, optarg); ok = 1; }
         if (strcmp("spipe", long_options[option_index].name) == 0)
            if (optarg) { strcpy(clv34, optarg); ok = 1; }
         if (strcmp("bfu", long_options[option_index].name) == 0)
            if (optarg) { strcpy(clv35, optarg); ok = 1; }
         if (strcmp("blat", long_options[option_index].name) == 0)
            if (optarg) { strcpy(clv36, optarg); ok = 1; }
         if (strcmp("lqs", long_options[option_index].name) == 0)
            if (optarg) { strcpy(clv37, optarg); ok = 1; }
         if (strcmp("sqs", long_options[option_index].name) == 0)
            if (optarg) { strcpy(clv38, optarg); ok = 1; }
         if (strcmp("spec", long_options[option_index].name) == 0)
            if (optarg) { strcpy(clv39, optarg); ok = 1; }
         if (strcmp("wblat", long_options[option_index].name) == 0)
            if (optarg) { strcpy(clv40, optarg); ok = 1; }
         if (strcmp("logfile", long_options[option_index].name) == 0)
            if (optarg) { strcpy(clv41, optarg); ok = 1; }

         break;
      case 'i':
         if (optarg) { inifile = optarg; ok = 1; }
         break; 
      case 'v':
         ++verbose; ok = 1;
         break;
      case 'd':
         debug = 1; ok = 1;
         break;
      default:
         usage = 1;
         break;
      }
   }
   if (show_version)
   {
      printf("%s\n", version_string);
      ExitProg("%s", "-");
   }
   if (show_help) Usage(0);
   if (usage || ok == 0 || optind < argc) Usage(1);


   RegisterIniSec(GeneralSec);
   RegisterIniSec(ArchitectureSec);
   RegisterIniSec(Program_DefaultsSec);

   /* Shell script execution */
   strcat(iniv, "progname="); strcat(iniv, clv1); strcat(iniv, ";");
   strcat(iniv, "silent="); strcat(iniv, clv2); strcat(iniv, ";");
   strcat(iniv, "interactive="); strcat(iniv, clv3); strcat(iniv, ";");
   strcat(iniv, "batch="); strcat(iniv, clv4); strcat(iniv, ";");
   strcat(iniv, "iterations="); strcat(iniv, clv5); strcat(iniv, ";");
   strcat(iniv, "start_ck="); strcat(iniv, clv6); strcat(iniv, ";");
   strcat(iniv, "lregs="); strcat(iniv, clv7); strcat(iniv, ";");
   strcat(iniv, "pregs="); strcat(iniv, clv8); strcat(iniv, ";");
   strcat(iniv, "pipestruct="); strcat(iniv, clv9); strcat(iniv, ";");
   strcat(iniv, "uni_lsu="); strcat(iniv, clv10); strcat(iniv, ";");
   strcat(iniv, "io_issue="); strcat(iniv, clv11); strcat(iniv, ";");
   strcat(iniv, "io_complete="); strcat(iniv, clv12); strcat(iniv, ";");
   strcat(iniv, "uni_di="); strcat(iniv, clv13); strcat(iniv, ";");
   strcat(iniv, "f_width="); strcat(iniv, clv14); strcat(iniv, ";");
   strcat(iniv, "d_width="); strcat(iniv, clv15); strcat(iniv, ";");
   strcat(iniv, "p_width="); strcat(iniv, clv16); strcat(iniv, ";");
   strcat(iniv, "i_width="); strcat(iniv, clv17); strcat(iniv, ";");
   strcat(iniv, "w_width="); strcat(iniv, clv18); strcat(iniv, ";");
   strcat(iniv, "c_width="); strcat(iniv, clv19); strcat(iniv, ";");
   strcat(iniv, "win_size="); strcat(iniv, clv20); strcat(iniv, ";");
   strcat(iniv, "rob_size="); strcat(iniv, clv21); strcat(iniv, ";");
   strcat(iniv, "int_fu="); strcat(iniv, clv22); strcat(iniv, ";");
   strcat(iniv, "a_lat="); strcat(iniv, clv23); strcat(iniv, ";");
   strcat(iniv, "im_fu="); strcat(iniv, clv24); strcat(iniv, ";");
   strcat(iniv, "im_lat="); strcat(iniv, clv25); strcat(iniv, ";");
   strcat(iniv, "im_pipe="); strcat(iniv, clv26); strcat(iniv, ";");
   strcat(iniv, "fp_fu="); strcat(iniv, clv27); strcat(iniv, ";");
   strcat(iniv, "fm_fu="); strcat(iniv, clv28); strcat(iniv, ";");
   strcat(iniv, "l_fu="); strcat(iniv, clv29); strcat(iniv, ";");
   strcat(iniv, "l_lat="); strcat(iniv, clv30); strcat(iniv, ";");
   strcat(iniv, "l_pipe="); strcat(iniv, clv31); strcat(iniv, ";");
   strcat(iniv, "s_fu="); strcat(iniv, clv32); strcat(iniv, ";");
   strcat(iniv, "s_lat="); strcat(iniv, clv33); strcat(iniv, ";");
   strcat(iniv, "s_pipe="); strcat(iniv, clv34); strcat(iniv, ";");
   strcat(iniv, "b_fu="); strcat(iniv, clv35); strcat(iniv, ";");
   strcat(iniv, "b_lat="); strcat(iniv, clv36); strcat(iniv, ";");
   strcat(iniv, "lqsize="); strcat(iniv, clv37); strcat(iniv, ";");
   strcat(iniv, "sqsize="); strcat(iniv, clv38); strcat(iniv, ";");
   strcat(iniv, "speculation="); strcat(iniv, clv39); strcat(iniv, ";");
   strcat(iniv, "wblat="); strcat(iniv, clv40); strcat(iniv, ";");
   strcat(iniv, "logfile="); strcat(iniv, clv41); strcat(iniv, ";");


   strcpy(iniv1, iniv);
   strcat(iniv1, "rfn=`echo \"$progname\"`");
   Mysysopen(iniv1);
   Mysyspars(rfn, "rfn");
   Mysysclose();

   strcpy(iniv1, iniv);
   strcat(iniv1, "rfn="); strcat(iniv1, rfn); strcat(iniv1, ";");
   strcat(iniv1, "logfile=$rfn.log");
   Mysysopen(iniv1);

   Mysysclose();

   /* Command Line Variable Initialization */
   strcpy(G.progname, clv1);
   G.silent = (0 == strcmp(clv2, "yes") ? 1 : 0);
   G.interactive = (0 == strcmp(clv3, "yes") ? 1 : 0);
   G.batch = (0 == strcmp(clv4, "yes") ? 1 : 0);
   IsNumVal(clv5, &(G.iterations));;
   IsNumVal(clv6, &(G.start_ck));;
   IsNumVal(clv7, &(AA.lregs));;
   IsNumVal(clv8, &(AA.pregs));;
   strcpy(AA.pipestruct, clv9);
   AA.uni_lsu = (0 == strcmp(clv10, "yes") ? 1 : 0);
   AA.io_issue = (0 == strcmp(clv11, "yes") ? 1 : 0);
   AA.io_complete = (0 == strcmp(clv12, "yes") ? 1 : 0);
   AA.uni_di = (0 == strcmp(clv13, "yes") ? 1 : 0);
   IsNumVal(clv14, &(AA.f_width));;
   IsNumVal(clv15, &(AA.d_width));;
   IsNumVal(clv16, &(AA.p_width));;
   IsNumVal(clv17, &(AA.i_width));;
   IsNumVal(clv18, &(AA.w_width));;
   IsNumVal(clv19, &(AA.c_width));;
   IsNumVal(clv20, &(AA.win_size));;
   IsNumVal(clv21, &(AA.rob_size));;
   IsNumVal(clv22, &(AA.int_fu));;
   IsNumVal(clv23, &(AA.a_lat));;
   IsNumVal(clv24, &(AA.im_fu));;
   IsNumVal(clv25, &(AA.im_lat));;
   AA.im_pipe = (0 == strcmp(clv26, "yes") ? 1 : 0);
   IsNumVal(clv27, &(AA.fp_fu));;
   IsNumVal(clv28, &(AA.fm_fu));;
   IsNumVal(clv29, &(AA.l_fu));;
   IsNumVal(clv30, &(AA.l_lat));;
   AA.l_pipe = (0 == strcmp(clv31, "yes") ? 1 : 0);
   IsNumVal(clv32, &(AA.s_fu));;
   IsNumVal(clv33, &(AA.s_lat));;
   AA.s_pipe = (0 == strcmp(clv34, "yes") ? 1 : 0);
   IsNumVal(clv35, &(AA.b_fu));;
   IsNumVal(clv36, &(AA.b_lat));;
   IsNumVal(clv37, &(AA.lqsize));;
   IsNumVal(clv38, &(AA.sqsize));;
   AA.speculation = (0 == strcmp(clv39, "yes") ? 1 : 0);
   IsNumVal(clv40, &(AA.wblat));;
   strcpy(Pro.logfile, clv41);

   strcpy(flogname, Pro.logfile);

   /* Call all constructors */
   nolog = 1 - logga;
   SUIConstr1(MainDestru, freess_app, inifile, nolog);

   /*  */
   if (template) {
      GenIniTemplate();
      ExitProg("%s", "-");
   }

   /* Log Ini Parameters */
   LogIniParm();

   /* Show Program Headings */
   if (verbose) Display("%s", PROG_PRESEN2);
   Superscalar__Constr();

   return (0);
}

/*---------------------------------------------------------------------------*
 NAME      :
 PURPOSE   :
 PARAMETERS:
 RETURN    : 
 *---------------------------------------------------------------------------*/
void MainDestru(void)
{
   /* Call all destructors */
   Superscalar__Destru();
   SUIDestru();
}


/*---------------------------------------------------------------------------*
 NAME      :
 PURPOSE   :
 PARAMETERS:
 RETURN    : 
 *---------------------------------------------------------------------------*/
void Usage(int status)
{
   if (status != 0)
      fprintf (stderr, "Try `%s -help' for more information.\n\n", PROG_NAME);
   else
   {
      printf ("\
Usage: %s [OPTION(s)]\n\
Options:\n\
", PROG_NAME);
      printf ("\
\n\
  -i, -inifile=FILENAME		  set inifile to FILENAME\n\
  -v, -verbose			  print detailed operations on screen\n\
  -help				  display this help and exit\n\
  -version			  output version information and exit\n\
  -auto				  allow the autostart (with no options)\n\
  -tracing			  trace functions (for debugging)\n\
  -log				  enable log-file creation\n\
  -template			  produce inifile with current parameters\n\
  -exe <program_name>		  Name of program (program)\n\
  -s <silent>			  Silent Mode (no)\n\
  -int <interactive>		  Interactive Mode (yes)\n\
  -batch <mode>			  Batch File Mode (no)\n\
  -iter <iterations>		  Number of Iterations (3)\n\
  -startck <starting_cycle>	  Starting Cycle (0)\n\
  -lregs <logical_registers>	  Number of Logical Regs (8)\n\
  -pregs <physical_registers>	  Number of Physical Regs (24)\n\
  -pstruct <pipeline_structure>	  Pipeline Structure (FDPIXWC)\n\
  -unilsu <unified_lsu>		  Unified LSU (yes)\n\
  -ioi <in-order_issue>		  In-Order Issue (no)\n\
  -ioc <in-order_complete>	  In-Order Complete (no)\n\
  -unidi <unified_dispatch/issue>  Unified Dispatch/Issue (yes)\n\
  -fw <fetch_width>		  Fetch Width (4)\n\
  -dw <decode_width>		  Decode Width (4)\n\
  -pw <dispatch_width>		  Dispatch Width (4)\n\
  -iw <issue_width>		  Issue Width (4)\n\
  -ww <write-back_width>	  Write-Back Width (4)\n\
  -cw <commit_width>		  Commit Width (4)\n\
  -wins <window_size>		  Instruction Window Size (16)\n\
  -robs <rob_size>		  Reorder Buffer Size (16)\n\
  -afu <integer_alu_units>	  Number of Int. FU (4)\n\
  -alat <integer_alu_latency>	  Latency of INT ALU FU (0)\n\
  -mfu <integer_mult._units>	  Number of Mult. FU (1)\n\
  -mlat <integer_mult._latency>	  Latency of Mult. FU (4)\n\
  -mpipe <integer_mult._pipe>	  Pipelinization of Mult. (yes)\n\
  -ffu <floating_point_units>	  Number of FP FU (4)\n\
  -xfu <floating_point_mult>	  Number of FP Mult. FU (1)\n\
  -lfu <load_units>		  Number of Load FU (1)\n\
  -llat <load_latency>		  Latency of Load FU (2)\n\
  -lpipe <load_pipe>		  Pipelinization of Load (yes)\n\
  -sfu <store_units>		  Number of Store FU (1)\n\
  -slat <store_latency>		  Latency of Store FU (1)\n\
  -spipe <store_pipe>		  Pipelinization of Store (yes)\n\
  -bfu <branch_units>		  Number of Branch FU (1)\n\
  -blat <branch_latency>	  Latency of Branch FU (0)\n\
  -lqs <load_queue_size>	  Load Queue Size (1)\n\
  -sqs <store_queue_size>	  Store Queue Size (1)\n\
  -spec <speculation>		  Enable Speculation (yes)\n\
  -wblat <write_back_latency>	  Write Back Latency (0)\n\
  -logfile <log_file_name>	  Log File (def.log)\n\
\n\
");
   }
   ExitProg("%s", "-");
}


