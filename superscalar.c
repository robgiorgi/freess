/*-----------------------------------------------------------------------------
FILE        : SUPERSCALAR.C
INFO        : Manage Superscalar Pipeline
DEVELOPEMENT: GCC 2.7.0
CREATED BY  : RG[14-May-2009]
MODIFIED BY : RG[14-May-2025]
-----------------------------------------------------------------------------*/
/*------------- LIBRARY PROCEDURES ------------------------------------------*/
#include <stdio.h>
#include <string.h>	/* strcmp(), strncpy(), strcpy() */
#include "main.h"
#include "superscalar.h"

/*------------- GENERAL DEFINITIONS -----------------------------------------*/
#define DEFAULT_ITERATIONS 3
static int STOREWAITS = 0; // whether the STORE occupies the issue buffers until
                           // completed (STOREWAITS==1) or not (STOREWATIS==0)
#define SCREENWIDTH     141

#define FORMAT_R        0
#define FORMAT_I        1
#define FORMAT_J        2
#define FORMAT_I2       3
#define FORMAT_I3       4
#define FORMAT_B        5
#define FORMAT_S2       6
#define FORMAT_S3       7

#define N_FU            0
#define A_FU            1
#define M_FU            2
#define L_FU            3
#define S_FU            4
#define B_FU            5
#define F_FU            6
#define X_FU            7
#define MAXFT           7
#define MAXFT1          (MAXFT+1)
#define AFUOP           1
#define MFUOP           2
#define LFUOP           4
#define SFUOP           8

#define ALLSTAGES	0
#define MAX_STAGE	7              //number of stages
#define MAX_STAGE1	(MAX_STAGE+1)  //no. of stage +1
#define FETCH	        1
#define DECODE	        2
#define RENAME	        2
#define DISPATCH	3
#define ISSUE	        4
#define EXECUTE	        5
#define MEMORY	        5
#define COMPLETE	6
#define WRITEBACK	6
#define COMMIT	        7
#define RETIRE	        7


typedef int u32;
typedef u32 rgid;
typedef u32 indx;
typedef u32 valu;
typedef u32 opcd;
typedef u32 bool;
typedef u32 addr;

#define NIN              0  // Not-An-Instruction
                            // RISC-V INSTRUCTION EQUIVALENT
                            // opcode+funct3(+funct7)
#define LOAD             1  // 0x03+2
#define STORE            2  // 0x23+2
#define BEQ              3  // 0x63+0
#define BNE              4  // 0x63+1
#define ADD              5  // 0x3b+0+0x0
#define ADDI             6  // 0x1b+0
#define MUL              7  // 0x3b+0+0x1

//Additional instructions
#define LOAD2           41  // 0x03+2
#define STORE2          42  // 0x23+2
#define NOP             49  // 0x33+0+0x0
#define MAXOPCODE       50  // 

typedef char InstrName[8];

typedef struct InstrFormatTAG {
   int format;
   int opcode;
   InstrName name;
   int futype;
} InstrFormat;

typedef struct FUnitTAG {
   int latency;
   int busy1;
   int inilatency;
   struct InstructionTAG *ip;
} FUnit;

typedef struct FArrayTAG {
   FUnit *FU;
   int tot;
   int busy2;
   int inilatency;
   int stalls;
   int pipe;
} FArray;

typedef struct InstructionTAG {
   int icount;
   int opcode;
   int optype;
   int rd;
   int rs1;
   int rs2;
   int rs3;
   int imv; // 1 if it has an immediate value
   int t[MAX_STAGE1];
   u32 CIC; /* Current IC */
   u32 CPC; /* Current PC */
   u32 NPC; /* Next PC */
   int bdir; /* DIRection prediction (1=Taken,0=Not-taken) */
   int pd;
   int ps1;
   int ps2;
   int ps3; /* 3rd source operand (e.g. SW x7,x1(x2)) */
   int pold;
   int robn;
   int winn;
   FUnit *fup;
   FUnit *fup1;
   int qi;
   int cj;
   int ck;
   int cl;
   int store;
   u32 efad;
   int inqueue;
   char iws[40];
   char rbs[30];
} Instruction;

typedef struct WindowEntryTAG {
   int opcode;
   int optype;
   int pi;
   int pj;
   int pk;
   int pl;
   int qj;
   int qk;
   int ql;
   int cj;
   int ck;
   int cl;
   int imv;
   int rob_entry;
   int delay; //models an eventual delay in processing the issue
   int busy;
   Instruction *ip;
} WindowEntry;

typedef struct ROBEntryTAG {
   u32 PC;
   int ri;
   int piold;
   int cplt;
   int cc;
   int exc;
   int st;
   int rbbusy;
   Instruction *ip;
} ROBEntry;

typedef struct InstructionSlotTAG {
   Instruction *ip;
   int delay;
   int inidelay;
} InstructionSlot;

typedef struct RegisterMapTAG {
   int busy;
   int ri;
   int vi;
   int qi;
} RegisterMap;

typedef struct RegFileTAG {
   int pn;
   int busy;
   int vi;
   int qi;
} RegFile;

typedef struct RegStationTAG {
   indx QJ, QK;
   valu VJ, VK;
   opcd OP;
   bool BUSY;
} RegStation;

typedef struct StoreQTAG {
   addr AD;
   indx qi;
   valu vi;
} StoreQ;

typedef struct LoadQTAG {
   addr AD;
} LoadQ;

typedef struct CDBusTAG {
   bool BUSY;
   indx qi;
   valu vi;
} CDBus;

typedef struct DECODELATCH_TAG {
   opcd OP;
   indx RS;
   indx RT;
   indx RD;
} DECODELATCH;

typedef struct FuncUnitTAG {
   bool BUSY;
   opcd OP;
} FuncUnit;

#define FU_MAX 3
#define RS_MAX 8
#define RF_MAX 8
#define LQ_MAX 2
#define SQ_MAX 2

typedef int (*Stagedo_pp)(Instruction **);
typedef int (*Stagedo_p)(Instruction *);

#define IHIST	80

/* Function prototypes */
int Stage(int st);
int ReleasePhysicalReg(int pn);
int StageMoveInstr(int st, int w0, int wk);
void print_RBEntry(Instruction *ip, char *buf);

int FETCH_DO(Instruction *ip);
int RENAME_DO(Instruction *ip);
int DISPATCH_DO(Instruction *ip);
int ISSUE_DO(Instruction *ip);
int EXECUTE_DO(Instruction *ip);
int COMPLETE_DO(Instruction *ip);
int COMMIT_DO(Instruction *ip);

int FETCH_END(Instruction *ip);
int RENAME_END(Instruction *ip);
int DISPATCH_END(Instruction *ip);
int ISSUE_END(Instruction *ip);
int EXECUTE_END(Instruction *ip);
int COMPLETE_END(Instruction *ip);
int COMMIT_END(Instruction *ip);
 
/*------------- GLOBAL VARIABLES --------------------------------------------*/
int bne_count = 0;
int StreamEnd;
FILE *CO = NULL; /* Cycle Out file pointer */
FILE *LOGSTALL = NULL; /* Cycle Out file pointer */
Instruction * LastInst;
InstrFormat IAR[] = {
   { FORMAT_R,  NIN,    "NULL", A_FU },
   { FORMAT_R,  ADD,    "ADD",  A_FU },
   { FORMAT_R,  MUL,    "MUL",  M_FU },
   { FORMAT_I,  ADDI,   "ADDI", A_FU },
   { FORMAT_B,  BNE,    "BEQ",  B_FU },
   { FORMAT_B,  BNE,    "BNE",  B_FU },
   { FORMAT_I2, LOAD,   "LW",   L_FU },
   { FORMAT_S2, STORE,  "SW",   S_FU },

   { FORMAT_I3, LOAD2,  "LW",   L_FU },
   { FORMAT_S3, STORE2, "SW",   S_FU },
   { FORMAT_R,  NOP,    "NOP",  A_FU },
};

char *ORDINAL[10] = { "first", "second", "third", "fourth", "fifth", "sixth", "seventh", "eighth", "ninth", "tenth" };

char *STAGE_ACR[MAX_STAGE1]     = {"-","F","D","P","I","X","W","C"};
int STAGE_DELAY[MAX_STAGE1]     = {  0,  1,  1,  1,  0,  1,  1,  1};
int STAGE_INORDER[MAX_STAGE1]   = {  1,  1,  1,  1,  0,  0,  0,  1};
int ST_IGNORE_FLAG[MAX_STAGE1]  = {  0,  0,  0,  0,  0,  0,  0,  0};
int ST_IGNORE_STBUF[MAX_STAGE1] = {  0,  0,  0,  0,  0,  0,  0,  1};
char *STAGE_NAME[MAX_STAGE1]    = {"-","FETCH","DECODE","DISPATCH","ISSUE","EXECUTE","COMPLETE","COMMIT"};
Stagedo_p STAGE_DO[MAX_STAGE1]  = {NULL,FETCH_DO,RENAME_DO,DISPATCH_DO,ISSUE_DO,EXECUTE_DO,COMPLETE_DO,COMMIT_DO};
Stagedo_p STAGE_END[MAX_STAGE1] = {NULL,FETCH_END,RENAME_END,DISPATCH_END,ISSUE_END,EXECUTE_END,COMPLETE_END,COMMIT_END};

InstrName OPNAME[MAXOPCODE];
int OPFORMAT[MAXOPCODE];
int OPFUTYPE[MAXOPCODE];

int	TotalFU;
int	FinishedPR;
int	PI; /* Static Program Instructions queue */
int	PI_n = 0; /* Static Program Length */
Instruction NULL_Instruction;
int	DYNSTREAM;     /* Dynamic Program Instructions queue */
WindowEntry *IW; /* Instruction Window */
WindowEntry *IWB; /* Instruction Window */
Instruction **LQ;  /* Load Store Queue */
Instruction **SQ;  /* Store Queue */
int PRHead;
int PRAllocated;
int PRStalls;
int IWAllocated;
int IWStalls;
int IWFull;
int RBAllocated;
int RBStalls;
int RBHead;
int RBTail;
int RBEmpty;
int RBFull;
int LQStalls;
int LQHead;
int LQTail;
int LQEmpty;
int LQFull;
int LQElems;
int SQStalls;
int SQHead;
int SQTail;
int SQEmpty;
int SQFull;
int SQElems;
ROBEntry *RB; /* Reorder Buffer */
FArray	*FA; /* Array of Functional units */
char *FUNAME[] = {"-", "A", "M", "L", "S", "B", "F", "X"};
Instruction **CB; /* Complete Buffer */
int CBHead; 

InstructionSlot *STAGE_BUF[MAX_STAGE1];
int	STAGE_SIZ[MAX_STAGE1];
int	STAGE_INORDER[MAX_STAGE1];
int	STAGE_LAST[MAX_STAGE1];    // first available slot in a given stage
RegisterMap *RM;
RegFile *RF;
int     RegFromFile = 0;

int	CK;	/* Current clock cycle */
int	PC;	/* Program Counter */
int	IC;	/* Instruction Counter */
int	PIC;	/* Previous Instruction Counter */
int	IsEnd;	/* Flag to indicate End */

RegStation   RS[RS_MAX + 1]; /* 2 RS_M, 3 RS_LS, 3 RS_A, 0 is not used*/
/*
LoadQ        LQ[LQ_MAX];
StoreQ       SQ[SQ_MAX];
*/
CDBus        CDB[1];
DECODELATCH  DCD[1];
FuncUnit     FU[FU_MAX + 1];

static  int             (*SSSClose)(FILE *) = NULL;
static  FILE            *SSS_fp = NULL;

static int Stall[MAX_STAGE1];

static int EST_DYNSTR_SIZE = 0; // Estimated Size of Dynamic Instruction Stream
#define LSQCOLSTRSIZE   30
static char **LSQCOLUMN;

#define LOGBFLEN 10000
char LOGBUF[LOGBFLEN];

/*------------- INTERNAL FUNCTIONS PROTOTYPES -------------------------------*/
void read_program(char *progname);
void dump_instruction_program(void);
void dump_pipeline(int stage, int ic_ini, int ic_end);
void print_lregs_args(Instruction *ip, char *regs, char *labpfx);

/*------------- FUNCTIONS IMPLEMENTATION ------------------------------------*/
/*---------------------------------------------------------------------------*
 NAME      : Superscalar__Constr
 PURPOSE   : Constructor
 PARAMETERS:
 RETURN    :
 *---------------------------------------------------------------------------*/
int Superscalar__Constr(void)
{
   int ic, j, k, st, n;
   InstructionSlot *isp;

   /**/
   n = sizeof(IAR) / sizeof(InstrFormat);
   for (k = 0; k < n; ++k) {
      strcpy(OPNAME[IAR[k].opcode], IAR[k].name);
      OPFORMAT[IAR[k].opcode] = IAR[k].format;
      OPFUTYPE[IAR[k].opcode] = IAR[k].futype;
   }
   
   /**/
   PI = Queue__Constr();
   RF = (RegFile *) Malloc((AA.lregs + 1) * sizeof(RegFile));
   if (RF == NULL) ExitProg("Cannot allocate Register File");
   for (k = 0; k <= AA.lregs; ++k) { RF[k].busy = 0; RF[k].vi = 0; RF[k].pn = -1; RF[k].qi = 0; }
   read_program(G.progname);
   EST_DYNSTR_SIZE = PI_n * G.iterations;
   LSQCOLUMN = (char **) Malloc((EST_DYNSTR_SIZE + 3) * sizeof(char*));
   if (LSQCOLUMN == NULL) ExitProg("Cannot allocate L/S Queue Column Buffer");
   for (k = 0; k < LSQCOLSTRSIZE + 3; ++k) {
      LSQCOLUMN[k] = (char *) Malloc(LSQCOLSTRSIZE);
      if (LSQCOLUMN[k] == NULL) ExitProg("Cannot allocate LSQ Buffer #%d", k);
   }

   /* IO/OO options */
   STAGE_INORDER[ISSUE]    = AA.io_issue ? 1 : 0;
   STAGE_INORDER[COMPLETE] = AA.io_complete ? 1 : 0;

   /* Stage DELAY options */
   STAGE_DELAY[ISSUE]      = AA.uni_di ? 0 : 1;

   /* Stage parameters */
   if (AA.uni_lsu) AA.s_fu = 0;
   TotalFU = AA.int_fu + AA.fp_fu + AA.l_fu + AA.s_fu + AA.b_fu + AA.im_fu + AA.fm_fu;
   FinishedPR = 0;
   STAGE_SIZ[0] = AA.f_width;
   STAGE_SIZ[FETCH] = AA.f_width;
   STAGE_SIZ[DECODE] = AA.d_width;
   STAGE_SIZ[DISPATCH] = AA.p_width;
   STAGE_SIZ[ISSUE] = AA.i_width;
   STAGE_SIZ[EXECUTE] = TotalFU;
   STAGE_SIZ[COMPLETE] = AA.w_width;
   STAGE_SIZ[COMMIT] = AA.c_width;

//   PI = Queue__Constr();

   /* Functional Units */
   FA = (FArray *) Malloc((MAXFT1) * sizeof(FArray));
   if (FA == NULL) ExitProg("Cannot allocate Functional Unit Array");
   FA[N_FU].tot = 1;		FA[N_FU].inilatency = 2;		FA[N_FU].pipe = 1;
   FA[A_FU].tot = AA.int_fu;	FA[A_FU].inilatency = 1 + AA.a_lat;	FA[A_FU].pipe = 1;
   FA[M_FU].tot = AA.im_fu;	FA[M_FU].inilatency = 1 + AA.im_lat;	FA[M_FU].pipe = AA.im_pipe;
   FA[L_FU].tot = AA.l_fu;	FA[L_FU].inilatency = 0 + AA.l_lat; 	FA[L_FU].pipe = AA.l_pipe;
   FA[S_FU].tot = AA.s_fu;	FA[S_FU].inilatency = 0 + AA.s_lat;	FA[S_FU].pipe = AA.s_pipe;
   FA[B_FU].tot = AA.b_fu;	FA[B_FU].inilatency = 1 + AA.b_lat;	FA[B_FU].pipe = 1;
   FA[F_FU].tot = AA.fp_fu;	FA[F_FU].inilatency = 2;		FA[F_FU].pipe = 1;
   FA[X_FU].tot = AA.fm_fu;	FA[X_FU].inilatency = 2;		FA[X_FU].pipe = 1;

   for (k = 0; k < MAXFT1; ++k) {
      FA[k].FU = (FUnit *) Malloc((FA[k].tot + 1) * sizeof(FUnit));
      if (FA[k].FU == NULL) ExitProg("Cannot allocate Functional Units type %d.", k);
      FA[k].stalls = 0;
      for (j = 0; j <= FA[k].tot; ++j) {
         FA[k].FU[j].busy1 = 0;
         FA[k].FU[j].inilatency = FA[k].inilatency;
      }
   }

   LQ = (Instruction **) Malloc((AA.lqsize) * sizeof(Instruction *));
   if (LQ == NULL) ExitProg("Cannot allocate Load Queue");
   LQHead = 0; LQTail = 0; LQFull = 0; LQEmpty = 1; LQStalls = 0; LQElems = 0;
   SQ = (Instruction **) Malloc((AA.sqsize) * sizeof(Instruction *));
   if (SQ == NULL) ExitProg("Cannot allocate Store Queue");
   SQHead = 0; SQTail = 0; SQFull = 0; SQEmpty = 1; SQStalls = 0; SQElems = 0;

   /* Instruction Window */
   IW = (WindowEntry *) Malloc((AA.win_size) * sizeof(WindowEntry));
   if (IW == NULL) ExitProg("Cannot allocate Instruction Window");
   for (k = 0; k < AA.win_size; ++k) { IW[k].busy = 0; }
   IWAllocated = 0;
   IWStalls = 0;
   IWFull = 0;

   /* Instruction Window Buffer (where instructions stay for displaying */
   IWB = (WindowEntry *) Malloc(EST_DYNSTR_SIZE * sizeof(WindowEntry));
   if (IWB == NULL) ExitProg("Cannot allocate Instruction Window Buffer");
   for (k = 0; k < EST_DYNSTR_SIZE; ++k) { IWB[k].busy = 0; }

   /* Reorder Buffer */
   RB = (ROBEntry *) Malloc((AA.rob_size) * sizeof(ROBEntry));
   if (RB == NULL) ExitProg("Cannot allocate Reorder Buffer");
   for (k = 0; k < AA.rob_size; ++k) RB[k].rbbusy = 0;
   RBAllocated = 0;
   RBHead = 0; RBTail = 0; RBEmpty = 1; RBFull = 0; RBStalls = 0;

   /* Stages Info */
   for (st = 0; st <= MAX_STAGE; ++st) {
      STAGE_BUF[st] = (InstructionSlot *) Malloc(STAGE_SIZ[st] * (sizeof(InstructionSlot)));
      if (STAGE_BUF[st] == NULL) ExitProg("Cannot allocate %s Buffer", STAGE_NAME[st]);
      STAGE_LAST[st] = -1;
      for (k = 0; k < STAGE_SIZ[st]; ++k) {
          STAGE_BUF[st][k].ip = NULL;
          STAGE_BUF[st][k].delay = 0;
          STAGE_BUF[st][k].inidelay = 1;
      }
      Display("%8s STAGE = %d entries.", STAGE_NAME[st], STAGE_SIZ[st]);
   }

   DYNSTREAM = Queue__Constr();

   NULL_Instruction.opcode = NIN;
   NULL_Instruction.rs1    = -1;
   NULL_Instruction.rs2    = -1;
   NULL_Instruction.rs3    = -1;
   NULL_Instruction.rd     = -1;
   NULL_Instruction.imv    = 0;
   for (k = 0; k < MAX_STAGE1; ++k) Stall[k] = 0;

   /* Register Map*/
   RM = (RegisterMap *) Malloc((AA.pregs + 1) * sizeof(RegisterMap));
   if (RM == NULL) ExitProg("Cannot allocate Register Map");
   for (k = 0; k <= AA.pregs; ++k) { RM[k].busy = 0; RM[k].qi = 1; }
   PRHead = 1;
   PRAllocated = 0;
   PRStalls = 0;

   /* Register File*/
   RF[0].pn = 0;
   RF[0].qi = 0;

   if (RegFromFile == 0) { //assign some default values
      RF[7].vi = 3;
      RF[1].vi = 0x1000;
      RF[3].vi = 0x3000;
   }

   /* Complete Buffer */
   CB = (Instruction **) Malloc((AA.w_width) * sizeof(RegFile));
   if (CB == NULL) ExitProg("Cannot allocate Complete Buffer");
   for (k = 0; k < AA.w_width; ++k) { CB[k] = &NULL_Instruction; }
   CBHead = 0;

   // clean LOGBUF
   *LOGBUF='\0';

   return (0);
}

/*---------------------------------------------------------------------------*
 NAME      : Superscalar__Destru
 PURPOSE   : Destructor
 PARAMETERS:
 RETURN    :
 *---------------------------------------------------------------------------*/
void Superscalar__Destru(void)
{
   int ic, st;

   if (SSSClose != NULL) SSSClose(SSS_fp);

   Queue__Destru(DYNSTREAM);
   for (st = 1; st <= MAX_STAGE; ++st) {
      Free(STAGE_BUF[st]);
   }

   Queue__Destru(PI);
   Free(RM);
   Free(RF);
   Free(IW);
   Free(RB);
}

/*---------------------------------------------------------------------------*
 NAME      : print_hypothesis
 PURPOSE   : Print the hypotheis of the test
 PARAMETERS:
 RETURN    :
 *---------------------------------------------------------------------------*/
void print_hypothesis(void)
{
    int ways = AA.f_width, onedone = 0;;
    char out1[256], out2[64];
    int k, n = Queue__Count(PI), kbranch=-1;
    Instruction *ip;
    char buf[32];

    //==============================================================
    strcpy(out1, ""); for (k = 1; k <= SCREENWIDTH; ++k) { strcat(out1, "="); } Display("%s", out1);

    Display("Consider the following snippet of code running on %d-ways out-of-order superscalar processor.", ways);
    strcpy(out1, "Initially, ");
    for (k = 1; k < AA.lregs; ++k) {
       if (RF[k].vi != 0) {
          if (onedone) strcat(out1, ", ");
          sprintf(out2, "x%d=0x%04X", k, RF[k].vi);
          strcat(out1, out2);
          onedone = 1;
       }
    }
    if (onedone) strcat(out1, " and the other registers contain zero."); else strcat(out1, "all registers contain zero.");
    Display(out1);
    Display("");

    // search for a label
    for (k = 0; k < n; ++k) {
       ip = (Instruction *)Queue__Read(k, PI);
       if (ip->opcode > MAXOPCODE - 1) ExitProg("BAD OPCODE in dump_instruction_program");
       if (ip->optype == B_FU) { kbranch = k + ip->rs3 + 1; }
    }

    for (k = 0; k < n; ++k) {
       ip = (Instruction *)Queue__Read(k, PI);
       print_lregs_args(ip, buf, "lab");
       if (ip->opcode > MAXOPCODE - 1) ExitProg("BAD OPCODE in dump_instruction_program");
       Display("\t\t%s%-4s %-11s", (kbranch >=0 && k == kbranch) ? "lab1:\t" : "\t", OPNAME[ip->opcode], buf);
    }

    Display("");
    Display("Working hypothesis:");
    char unilsu[40] = "separated (1 for L, 1 for S)";
    if (AA.uni_lsu) { strcpy(unilsu, "common"); }
    if (AA.f_width == AA.d_width && AA.f_width == AA.c_width) {
        Display("* the fetch, decode and commit stages are %d instructions wide", ways);
    } else {
        Display("* the fetch stages %d wide, the decode stage is %d wide, the commit stage is %d wide", AA.f_width, AA.d_width, AA.c_width);
    }

    Display("* the instruction window has %d slots", AA.win_size);
    Display("* we have %d physical registers in the free pool (excluding P0 which is hardwired to 0)", AA.pregs);
    if (AA.rob_size >=99) {
        Display("* the reorder buffer has unlimited size");
    } else {
        Display("* the reorder buffer has %d entries", AA.rob_size);
    }
    Display("* the integer multiplier has %d stages", AA.im_lat);
    if (AA.lqsize == AA.sqsize) {
        Display("* the load/store queues have %d slots each and a %s effective-address calculation unit", AA.lqsize, unilsu);
    } else {
        Display("* the load queue has %d slots and the load queue has %d slots and a %s effective-address calculation unit", AA.lqsize, AA.sqsize, unilsu);
    }
    Display("* there are %d ALUs for arithmetic and logic operations and for branching", AA.int_fu);
    if (STAGE_DELAY[ISSUE] == 0) {
       Display("* an ALU/BRANCH performs its operation in the same cycle when the operation is issued");
    }
    Display("* reads require %d clock cycle (after the addressing phase)", AA.l_lat - 1);
    Display("* the register file has %d input- and %d output-ports", AA.c_width, AA.c_width);
    Display("* there are %d logical registers (excluding x0 which is hardwired to 0)", AA.lregs);
    if (STOREWAITS) {
        Display("* he store operation occupies the issue stage until the store has completed");
    } else {
        Display("* the store operation leaves the issue stage as it is inserted in the store queue");
    }
    Display("* when the X stage finds a free slot in the LQ/SQ, also a slot in the I stage is reserved");
    Display("* branches are predicted taken");
    Display("");
    Display("Calculate the total cycles, needed to execute %d iterations of the above loop on such machine;", G.iterations);
    Display("complete the following chart until the end of the %s iteration of the code fragment above, including the", ORDINAL[G.iterations-1]);
    Display("renamed stream the precise evolution of the free pool of the physical registers (the register map), the");
    Display("Instruction Window, the Reorder Buffer (ROB) and the Load Queue (LQ) and Store Queue (SQ).");
    Display("Note: Ci, Cj, Ck, Cl indicate the cycle when the corresponding physical registers are available in IW");

}

/*---------------------------------------------------------------------------*
 NAME      : Superscalar__Start
 PURPOSE   : Starting the execution
 PARAMETERS:
 RETURN    :
 *---------------------------------------------------------------------------*/
int Superscalar__Start(int argc, char **argv)
{
   char dummy[10], fn[64], fnlog[64], *c = dummy;
   int st;
   int (*fc)(FILE *) = fclose;
   int ret;

   StreamEnd = 0;
   LastInst = NULL;
   CK = G.start_ck;
   PC = 0;
   IC = 0;
   PIC = 0;
   IsEnd = 0;
//   read_program(G.progname);
//   dump_instruction_program();
   if (!G.silent) Display("----------------------------------------------------------");

   // Opening 'stall.log' file
   sprintf(fnlog, "stall.log");
   LOGSTALL = FOpen(fnlog, "w+", &fc);
   if (LOGSTALL == NULL) ExitProg("Cannot write file '%s'", fnlog);

   // Print work hypothesis 
   print_hypothesis();

   while (!IsEnd) {
      if (verbose) printf("\n====== STARTING CYCLE %d ===================================\n", CK);
      sprintf(fn, "cycle%03d.txt", CK);
      if (G.batch) { CO = FOpen(fn, "w+", &fc); }
      for (st = MAX_STAGE; st > 0; --st) {
         if (STAGE_DELAY[st - 1] == 0 && st > 1) { 
            Stage(st - 1);
            Stage(st);
            --st;
         } else {
            Stage(st);
         }
      }

      if (!G.silent) {
         dump_pipeline(ALLSTAGES, 0, IC-1);
         Display("------------------------------------------------- Press ENTER to continue (PC=%d,IC=%d,CK=%d,CTOT=%d,IPC=%3.2f)...", PC, IC, CK, CK + 1, (float)IC/(CK + 1 + 0.000000001));
         Display(LOGBUF); *LOGBUF='\0';
      }
      if (G.interactive && ! G.silent) {
#ifndef __EMSCRIPTEN__
         ret = scanf("%c", c);
	 if (ret == 0) printf("\n"); //i.e., do nothing
#endif
      }
      if (G.batch) { if (CO != NULL) fclose(CO); }

      ++CK;
   }
   if (LOGSTALL != NULL) fclose(LOGSTALL);
   Display("Program '%s' FINISHED", G.progname);
   Display("----------------------------------------------------------");
   Display("PC=%d,IC=%d,CK=%d,IPC=%3.2f", PC, IC, CK, (float)IC/(CK+0.000000001));

   return (0);
}

/*---------------------------------------------------------------------------*
 NAME      : safecat
 PURPOSE   : Safely concatenate src to dest
 PARAMETERS:
 RETURN    :
 *---------------------------------------------------------------------------*/
void safecat(char *dest, const char *src, size_t maxlen) {
   size_t dest_len = strnlen(dest, maxlen);

   // If dest is already too long, forcibly terminate at maxlen - 1
   if (dest_len >= maxlen - 1) {
      dest[maxlen - 1] = '\0';
      return;
   }

   // Determine how much space is left for src (excluding final '\0')
   size_t space_left = maxlen - dest_len - 1;

   // Copy as much as possible from src
   if (space_left > 0) {
      // Use memcpy for strict control (no scanning of src)
      size_t src_len = strnlen(src, space_left);
      memcpy(dest + dest_len, src, src_len);
      dest[dest_len + src_len] = '\0';
   }
}

/*---------------------------------------------------------------------------*
 NAME      : LogStall
 PURPOSE   : Log the reasons of the stall at a certain cycle
 PARAMETERS:
 RETURN    :
 *---------------------------------------------------------------------------*/
void LogStall(int *stallcnt, char *stalltype)
{
   char msg[SCREENWIDTH];
   char aux[SCREENWIDTH+20];

   ++(*stallcnt); *msg = '\0';
   safecat(msg, stalltype, SCREENWIDTH); //cut to SCREENWIDTH
   sprintf(aux, "@%03d stall due to %s\n", CK, msg);
   fprintf(LOGSTALL, "%s", aux);
   fflush(LOGSTALL);
   safecat(LOGBUF, aux, LOGBFLEN);
}

/*---------------------------------------------------------------------------*
 NAME      : WriteCDB
 PURPOSE   : Write on the common data bus
 PARAMETERS:
 RETURN    :
 *---------------------------------------------------------------------------*/
int WriteCDB(indx r, valu v) {
   int ok = 0;

   if (CDB[0].BUSY == 0) {
      CDB[0].qi = r;
      CDB[0].vi = v;
      CDB[0].BUSY = 1;
      ok = 1;
   }

   return (ok);
}

void ExecA(int fu, indx r) {

}

void ExecM(int fu, indx r) {

}

void ExecL(int fu, indx r) {

}

void ExecS(int fu, indx r) {

}

void PushS(indx rd) {
   indx s; 

//   if (RF[rd].qi != 0) { SQ[s].qi = RF[rd].qi; } else {  SQ[s].vi = RF[rd].vi;  SQ[s].qi =0; }
}

void PushSA(addr AD) {

}

void PushL(addr AD) {

}

int Get_Available_RS(opcd op) {
   int rr;
   int found = 0;

   for (rr = 1; rr <= RS_MAX; ++rr) {
      if (RS[rr].BUSY == 0 && ((RS[rr].OP & op) == op)) { found = rr; break; }
   }

   return (found);
}

int Get_Available_FU(opcd op) {
   int ff;
   int found = 0;

   for (ff = 1; ff <= FU_MAX; ++ff) {
       if (FU[ff].BUSY == 0 && ((FU[ff].OP & op) == op)) { found = ff; break; }
   }

   return (found);
}

void Get_Decoded_Instr(opcd *op, indx *rs, indx *rt, indx *rd) {
   *op = DCD[0].OP;
   *rs = DCD[0].RS;
   *rt = DCD[0].RT;
   *rd = DCD[0].RD;
}

int Get_ResStation(opcd op) {
   int rr;
   int found = 0;

   for (rr = 1; rr <= RS_MAX; ++rr) {
      if (RS[rr].BUSY == 1 && ((RS[rr].OP & op) == op)) { found = rr; break; }
   }

   return (found);
}

void Initialize(void) {

   FU[1].OP = MFUOP;
   RS[1].OP = MFUOP;
   RS[2].OP = MFUOP;

   FU[2].OP = LFUOP|SFUOP;
   RS[3].OP = LFUOP|SFUOP;
   RS[4].OP = LFUOP|SFUOP;
   RS[5].OP = LFUOP|SFUOP;

   FU[3].OP = AFUOP;
   RS[6].OP = AFUOP;
   RS[7].OP = AFUOP;
   RS[8].OP = AFUOP;
}

/*---------------------------------------------------------------------------*
 NAME      : Dispatch_STAGE
 PURPOSE   : Implement the Dispatch (P) stage
 PARAMETERS:
 RETURN    :
 *---------------------------------------------------------------------------*/
int Dispatch_STAGE(void) {
   indx r; opcd op; rgid rs; rgid rt; rgid rd;

   Get_Decoded_Instr(&op, &rs, &rt, &rd);
   r = Get_Available_RS(op);
   if (r == 0) return -1;


   switch(op) {
      case L_FU: case S_FU: case M_FU: case A_FU:
         if (RF[rs].qi != 0) { RS[r].QJ = RF[rs].qi; } else {  RS[r].VJ = RF[rs].vi;  RS[r].QJ =0; }
         if (RF[rt].qi != 0) { RS[r].QK = RF[rt].qi; } else {  RS[r].VK = RF[rt].vi;  RS[r].QK =0; }
         break;
      case N_FU: default:
         Display("NOP or undefined instructions\n");
   }
   if (op == S_FU) { PushS(rd); }
   RS[r].BUSY = 1;
   RF[rd].qi = r;
   return(0);
}

/*---------------------------------------------------------------------------*
 NAME      : Issue_STAGE
 PURPOSE   : Implement the Issue (I) stage
 PARAMETERS:
 RETURN    :
 *---------------------------------------------------------------------------*/
int Issue_STAGE(void) {
   int fu;
   indx r;
   opcd op;

   for (fu = 1; fu < FU_MAX; ++fu) {
      op = FU[fu].OP;
      if (FU[fu].BUSY == 0 && ((FU[fu].OP & op) == op)) {

         r = Get_ResStation(op);      
         if (r == 0) continue;

         switch (op) {
            case M_FU: if (RS[r].QJ == 0 && RS[r].QK == 0) { RS[r].BUSY = 0; ExecM(fu, r); } break;
            case L_FU: if (RS[r].QJ == 0 && RS[r].QK == 0) { RS[r].BUSY = 0; ExecL(fu, r); } break;
            case A_FU: if (RS[r].QJ == 0 && RS[r].QK == 0) { RS[r].BUSY = 0; ExecA(fu, r); } break;
            case S_FU: if (RS[r].QJ == 0 && RS[r].QK == 0) { RS[r].BUSY = 0; ExecS(fu, r); } break;
         }
      }

   }
   return(0);
}

/*---------------------------------------------------------------------------*
 NAME      : Complete_STAGE
 PURPOSE   : Implement the Complete (Write-back or W) stage
 PARAMETERS:
 RETURN    :
 *---------------------------------------------------------------------------*/
void Complete_STAGE(void) {
   int rr;

   if (CDB[0].BUSY == 1) {
      for (rr = 1; rr <= RS_MAX; ++rr) {
         if (RS[rr].QJ == CDB[0].qi) { RS[rr].VJ = CDB[0].vi; RS[rr].QJ = 0; RS[rr].BUSY = 0; }
         if (RS[rr].QK == CDB[0].qi) { RS[rr].VK = CDB[0].vi; RS[rr].QK = 0; RS[rr].BUSY = 0; }
      }
      for (rr = 0; rr < RF_MAX; ++rr) {
         if (RF[rr].qi == CDB[0].qi) { RF[rr].vi = CDB[0].vi; RF[rr].qi = 0; }
      }
      for (rr = 0; rr < SQ_MAX; ++rr) {
//         if (SQ[rr].qi == CDB[0].qi) { SQ[rr].vi = CDB[0].vi; SQ[rr].qi = 0; }
      }
   }
}

/*---------------------------------------------------------------------------*
 NAME      : InsertIntoStageBuffer
 PURPOSE   : Insert an instruction in the stage buffers
 PARAMETERS:
 RETURN    :
 *---------------------------------------------------------------------------*/
int InsertIntoStageBuffer(Instruction *ips, int stage)
{
   InstructionSlot *islot = STAGE_BUF[stage];
   Instruction *ip;
   int k, found = 0, qfound, flag = 0, start, q, kfull = -1;

   // Inserting into STAGE buffer
   if (debug) printf("  --> INSERTING %s/%03d INTO %s:", OPNAME[ips->opcode], ips->CIC, STAGE_NAME[stage]);
//   if (STAGE_INORDER[stage]) { start = (STAGE_LAST[stage] != -1) ? STAGE_LAST[stage] : 0; }

   // Find last full
   for (k = 0; k < STAGE_SIZ[stage]; ++k) {
      if (islot[k].ip != NULL) kfull = k;
   }
   if (kfull < STAGE_SIZ[stage]) {
      start = kfull + 1;
      for (k = 0; k < STAGE_SIZ[stage]; ++k) {
         q = (start + k) % STAGE_SIZ[stage];
         ip = islot[q].ip;
         if (ip == NULL) { found = 1; qfound = q; break; }
      }
   }
   if (found) {
      islot[qfound].ip    = ips;
      islot[qfound].delay = 1; //tentative value: how about ip.delay ?
      flag = 1;
      if (STAGE_INORDER[stage]) STAGE_LAST[stage] = (qfound + 1) % STAGE_SIZ[stage]; //update head
      if (debug) printf(" slot %d", qfound);
   } else {
      if (debug) printf(" ---- all slots busy");
   }
   if (debug) printf("\n");
   return (flag);
}

/*---------------------------------------------------------------------------*
 NAME      : RemoveFromStageBuffer
 PURPOSE   : Remove an instruction from the stage buffers
 PARAMETERS:
 RETURN    :
 *---------------------------------------------------------------------------*/
int RemoveFromStageBuffer(Instruction *ips, int stage)
{
   InstructionSlot *islot = STAGE_BUF[stage];
   Instruction *ip;
   int k, found = 0, kfound;

   // Remove from STAGE buffer
   if (debug) printf("  --> REMOVING:");
   for (k = 0; k < STAGE_SIZ[stage]; ++k) {
      ip = islot[k].ip;
      if (ip != NULL) {
         if (ip->CIC == ips->CIC) { found = 1; kfound = k; break; }
      }
   }
   if (found) {
      if (debug) printf(" %s/%03d FROM %s[%d]", OPNAME[ip->opcode], ips->CIC, STAGE_ACR[stage], kfound);
      islot[kfound].ip = NULL;
      islot[kfound].delay = 0;
   } else {
      if (debug) printf(" NONE");
   }
   if (debug) printf("\n");
   return (found);
}

/*---------------------------------------------------------------------------*
 NAME      : PopLQ
 PURPOSE   : Pop an instruction from the Load Queue
 PARAMETERS:
 RETURN    :
 *---------------------------------------------------------------------------*/
Instruction *PopLQ()
{
   Instruction *ip;

   if (LQEmpty) return (NULL);
   ip = LQ[LQHead];
   ip->inqueue = 2;
   LQHead = (LQHead + 1) % AA.lqsize;
   LQFull = 0;
   --LQElems;
   if (LQTail == LQHead) LQEmpty = 1;
   if (verbose) printf("    --> PopLQ: LQElems=%d LQTail=%d LQHead=%d LQEmpty=%d\n", LQElems, LQTail, LQHead, LQEmpty);
   return (ip);
}

/*---------------------------------------------------------------------------*
 NAME      : PopSQ
 PURPOSE   : Pop an instruction from the Store Queue
 PARAMETERS:
 RETURN    :
 *---------------------------------------------------------------------------*/
Instruction *PopSQ()
{
   Instruction *ip;

   if (SQEmpty) return (NULL);
   ip = SQ[SQHead];
   ip->inqueue = 2;
   SQHead = (SQHead + 1) % AA.sqsize;
   SQFull = 0;
   --SQElems;
   if (SQTail == SQHead) SQEmpty = 1;
   if (verbose) printf("    --> PopSQ: SQElems=%d SQTail=%d SQHead=%d SQEmpty=%d\n", SQElems, SQTail, SQHead, SQEmpty);
   return (ip);
}

/*---------------------------------------------------------------------------*
 NAME      : PushLQ
 PURPOSE   : Push an instruction into the Load Queue
 PARAMETERS:
 RETURN    :
 *---------------------------------------------------------------------------*/
int PushLQ(Instruction *ipl)
{
//   Queue__HInsert((void *)(FA[L_FU].FU[1].ip), LQ);
   if (verbose) if (LQFull) printf("    --> PushLQ: FULL -- AA.lqsize=%d LQElems=%d LQTail=%d LQHead=%d LQEmpty=%d\n", AA.lqsize, LQElems, LQTail, LQHead, LQEmpty);
   if (LQFull) return 1;
   ipl->inqueue = 1;
   LQ[LQTail] = ipl;
//printf("PushLQ: %08X\n", LQ[LQTail]);
   LQTail = (LQTail + 1) % AA.lqsize;
   LQEmpty = 0;
   ++LQElems;
   if  (LQTail == LQHead) LQFull = 1;
   if (verbose) printf("    --> PushLQ: AA.lqsize=%d LQElems=%d LQTail=%d LQHead=%d LQEmpty=%d LQFull=%d\n", AA.lqsize, LQElems, LQTail, LQHead, LQEmpty,LQFull);
   return 0;
}

/*---------------------------------------------------------------------------*
 NAME      : PushSQ
 PURPOSE   : Push an instruction into the Store Queue
 PARAMETERS:
 RETURN    :
 *---------------------------------------------------------------------------*/
int PushSQ(Instruction *ips)
{
//   Queue__HInsert((void *)(FA[L_FU].FU[1].ip), SQ);
   if (verbose) if (SQFull) printf("    --> PushSQ: FULL == AA.sqsize=%d SQElems=%d SQTail=%d SQHead=%d SQEmpty=%d\n", AA.sqsize, SQElems, SQTail, SQHead, SQEmpty);
   if (SQFull) return 1;
   SQ[SQTail] = ips;
   ips->inqueue = 1;
   SQTail = (SQTail + 1) % AA.sqsize;
   SQEmpty = 0;

// Rremove from STAGE buffer
//RemoveFromStageBuffer(ips, EXECUTE);

   ++SQElems;
   if (SQTail == SQHead) SQFull = 1;
   if (verbose) printf("    --> PushSQ: AA.sqsize=%d SQElems=%d SQTail=%d SQHead=%d SQEmpty=%d SQFull=%d\n", AA.sqsize, SQElems, SQTail, SQHead, SQEmpty, SQFull);
   return 0;
}

/*---------------------------------------------------------------------------*
 NAME      : CheckLQ
 PURPOSE   : Check if Load queue is full
 PARAMETERS:
 RETURN    :
 *---------------------------------------------------------------------------*/
int CheckLQ()
{
//   if (LQFull) ++LQStalls;
   if (LQFull) LogStall(&LQStalls, "LQ full");
   return (!LQFull);
}

/*---------------------------------------------------------------------------*
 NAME      : CheckSQ
 PURPOSE   : Check if Store queue is full
 PARAMETERS:
 RETURN    :
 *---------------------------------------------------------------------------*/
int CheckSQ()
{
//   if (SQFull) ++SQStalls;
   if (SQFull) LogStall(&SQStalls, "SQ full");
   return (!SQFull);
}

/*---------------------------------------------------------------------------*
 NAME      :
 PURPOSE   :
 PARAMETERS:
 RETURN    :
 *---------------------------------------------------------------------------*/
//int TryPushLQ(Instruction *ip)
int TryPushLQ(void)
{
   int ec = 0;
   CheckLQ();

//printf("LQFull=%d",LQFull);
   ec = LQFull;

//   ec = PushLQ(ip);
//   if (ec) {
//      ExitProg("Load Queue FULL! (stall not implemented: increase the queue size)");
//      //TODO: stall in case of full LQ
//   }
   return (ec);
}

/*---------------------------------------------------------------------------*
 NAME      :
 PURPOSE   :
 PARAMETERS:
 RETURN    :
 *---------------------------------------------------------------------------*/
//int TryPushSQ(Instruction *ip)
int TryPushSQ(void)
{
   int ec = 0;
   CheckSQ();

//printf("SQFull=%d",SQFull);
   ec = SQFull;

//   ec = PushSQ(ip);
//   if (ec) {
//      ExitProg("Store Queue FULL! (stall not implemented: increase the queue size)");
//      //TODO: stall in case of full SQ
//   }
   return (ec);
}

/*---------------------------------------------------------------------------*
 NAME      :
 PURPOSE   :
 PARAMETERS:
 RETURN    :
 *---------------------------------------------------------------------------*/
int CheckFU(Instruction *ip)
{
   int k, t, t1 = ip->optype, found = 0;
   char aux[SCREENWIDTH+20];
   if (t1 == S_FU && AA.uni_lsu) t = L_FU; else t = t1;

   for (k = 1; k <= FA[t].tot; ++k) {
//printf("t=%d   t1=%d\n",t, t1);
      if (t1 == L_FU) if (TryPushLQ()) continue; //continue if LQ full
      if (t1 == S_FU) if (TryPushSQ()) continue; //continue if SQ full
      if (FA[t].FU[k].busy1 == 0) {
         found = k;
         break;
      }
   }
//printf("found=%d\n",found);
//   if (!found) ++(FA[t].stalls);
   sprintf(aux, "no %s-unit available", FUNAME[t1]);
   if (!found) LogStall(&(FA[t1].stalls), aux);

   return (found);
}

/*---------------------------------------------------------------------------*
 NAME      : GetFU
 PURPOSE   : Get a Functional Unit for the given instruction
 PARAMETERS:
 RETURN    :
 *---------------------------------------------------------------------------*/
FUnit *GetFU(Instruction *ip, int fn)
{
   int k, t = ip->optype;
   if (t == S_FU && AA.uni_lsu) t = L_FU;
   if (verbose) printf("  GetFU: FA[%s].FU[%d]\n", FUNAME[t], fn);

   FA[t].FU[fn].busy1 = 1;
   ++(FA[t].busy2);
   FA[t].FU[fn].ip = ip;
//printf("%08X\n", ip);

   return (&(FA[t].FU[fn]));
}

/*---------------------------------------------------------------------------*
 NAME      : ReleaseFU
 PURPOSE   : Release the Functional Unit used by a given instruction
 PARAMETERS:
 RETURN    :
 *---------------------------------------------------------------------------*/
int ReleaseFU(Instruction *ip)
{
   int j, t, nldone = 0, full;
   FUnit *fu, *ful;

   if (verbose) printf("  --> ReleaseFU: ");
   if (ip == NULL) { printf("\n"); return (1); }
   t = ip->optype;
   if (t == S_FU && AA.uni_lsu) t = L_FU;

   if (verbose) printf("FA[%s] (FA_TOTAL=%d)", FUNAME[t], FA[t].tot);
   for (j = 1; j <= FA[t].tot; ++j) {
      fu = &(FA[t].FU[j]); full = 1;
      switch (t) {
         case L_FU:
            ful = &(FA[t].FU[j]);
            if (ful->busy1 == 1) {
               if (verbose) { nldone = 0; printf(", %d:%4s/%03d", j, OPNAME[fu->ip->opcode], fu->ip->CIC); }
//if (!STOREWAITS) {
               if ((ful->ip)->store == 1) { nldone = 1; printf("\n"); full = PushSQ(ful->ip); }
               if ((ful->ip)->store == 0) { nldone = 1; printf("\n"); full = PushLQ(ful->ip); }
//               nldone = 1; printf("\n"); full = PushLQ(ful->ip);
               if (!full) {
                  ful->busy1 = 0;
                  --(FA[t].busy2);
               }
//  (FA[L_FU].FU[1].ip)->t[ISSUE] = CK;
//  (FA[L_FU].FU[1].ip)->fup = NULL;
            }
            break;
         case S_FU:
            ful = &(FA[t].FU[j]);
            if (ful->busy1 == 1) {
               if (verbose) { nldone = 0; printf(", %d:%4s/%03d", j, OPNAME[fu->ip->opcode], fu->ip->CIC); }
//if (!STOREWAITS) {
               if ((ful->ip)->store == 1) { nldone = 1; printf("\n"); full = PushSQ(ful->ip); }
               if ((ful->ip)->store == 0) { nldone = 1; printf("\n"); full = PushLQ(ful->ip); }
//               nldone = 1; printf("\n"); full = PushSQ(ful->ip);
               if (!full) {
                  ful->busy1 = 0;
                  --(FA[t].busy2);
               }
//  (FA[L_FU].FU[1].ip)->t[ISSUE] = CK;
//  (FA[L_FU].FU[1].ip)->fup = NULL;

            }
            break;
        default:
           if (fu->busy1 == 1) {
              if (verbose) { nldone = 0; printf(", %d:%4s/%03d", j, OPNAME[fu->ip->opcode], fu->ip->CIC); }
              fu->busy1 = 0;
              --(FA[t].busy2);
           }
           break;
      }
   }
   if (verbose) if (!nldone) printf("\n");
   return (0);
}

/*---------------------------------------------------------------------------*
 NAME      : CheckROBEntry
 PURPOSE   : Check if ROB is full
 PARAMETERS:
 RETURN    :
 *---------------------------------------------------------------------------*/
int CheckROBEntry(void)
{
//   if (RBFull) ++RBStalls;
   if (RBFull) LogStall(&RBStalls, "ROB full");
   return (!RBFull);
}

/*---------------------------------------------------------------------------*
 NAME      : GetROBEntry
 PURPOSE   : Get an available ROB entry index (if available)
 PARAMETERS:
 RETURN    :
 *---------------------------------------------------------------------------*/
int GetROBEntry(int *rk)
{
   int j, k, found = 0, start = 0, start1 = 0;
   
   if (!RBFull) {
      *rk = RBTail;
      RB[RBTail].rbbusy = 1;
      found =1;
      ++RBAllocated;
      RBTail = (RBTail + 1) % AA.rob_size;
      RBEmpty = 0;
      if (RBTail == RBHead) RBFull = 1;
   }

   return (found);
}

/*---------------------------------------------------------------------------*
 NAME      : ReleaseROBEntry
 PURPOSE   : Release a ROB entry associated with the given instruction
 PARAMETERS:
 RETURN    :
 *---------------------------------------------------------------------------*/
int ReleaseROBEntry(Instruction **ipp)
{
   int flag = 0, piold = -1, ri = -1, a = RBHead;
   Instruction *ip = *ipp;

   if (!RBEmpty) {
      if (RB[RBHead].cplt == 1) {
         RB[RBHead].rbbusy = 0;
         RB[RBHead].cc = CK;
         piold = RB[RBHead].piold;
         ri    = RB[RBHead].ri;
         *ipp  = RB[RBHead].ip;
         if (debug) printf("  - Release ROB entry: %s\n", OPNAME[(*ipp)->opcode]);
         (*ipp)->t[COMMIT] = CK;
         print_RBEntry(*ipp, (*ipp)->rbs);
         (*ipp)->robn = -1;

         if (piold != -1) {
            ReleasePhysicalReg(piold);
            RM[piold].busy = 0;
//??            RF[ri].vi = RM[piold].vi;
            RF[ri].vi = RM[piold].vi;
         }
         flag = 1;
         --RBAllocated;
         RBHead = (RBHead + 1) % AA.rob_size;
         RBFull = 0;
         if (RBTail == RBHead) RBEmpty = 1;
      }
   }
   if (verbose) {
      printf("    ReleaseROBEntry: flag=%d ri=%d piold=%d RBHead=%d-->%d\n", flag, ri, piold, a, RBHead);
   }
   
   return (flag);
}

/*---------------------------------------------------------------------------*
 NAME      : CheckIWEntry
 PURPOSE   : Check if the Instruction Window is full
 PARAMETERS:
 RETURN    :
 *---------------------------------------------------------------------------*/
int CheckIWEntry(void)
{
//   if (IWFull) ++IWStalls;
   if (IWFull) LogStall(&IWStalls, "IW Full");
   return (!IWFull);
}

/*---------------------------------------------------------------------------*
 NAME      : GetIWEntry
 PURPOSE   : Get the index of a free Instruction Window entry
 PARAMETERS:
 RETURN    :
 *---------------------------------------------------------------------------*/
int GetIWEntry(int *wk)
{
   int k, found = 0;
   
   for (k = 0; k < AA.win_size; ++k) {
      if (IW[k].busy == 0) {
         *wk = k;
         IW[k].busy = 1;
         IW[k].cj = -1;
         IW[k].ck = -1;
         IW[k].cl = -1;
         found =1;
         ++IWAllocated;
         if (IWAllocated == AA.win_size) IWFull = 1;
         break;
      }
   }
//   if (! found) { IWFull = 1; ++IWStalls; } else { IWFull = 0; }

   return (found);
}

/*---------------------------------------------------------------------------*
 NAME      : ReleaseIWEntry
 PURPOSE   : Release an Instruction Window entry associated with the instr.
 PARAMETERS:
 RETURN    :
 *---------------------------------------------------------------------------*/
void ReleaseIWEntry(int wk)
{
   if (debug) printf("  - Release IW entry: %s\n", OPNAME[IW[wk].ip->opcode]);
   if (IW[wk].qj == 0) { IW[wk].cj= CK; IW[wk].ip->cj = CK; }
   if (IW[wk].qk == 0) { IW[wk].ck= CK; IW[wk].ip->ck = CK; }
   if (IW[wk].ql == 0 && IW[wk].imv == 0 && IW[wk].ip->ps3 != -1) { IW[wk].cl= CK; IW[wk].ip->cl = CK; } 

   // Save the IW entry in the IW Buffer (IWB) so that I can print it later
   memcpy((void *)(&IWB[wk]), (void *)(&IW[wk]), sizeof(WindowEntry));
   IW[wk].busy = 0;
   --IWAllocated;
   IWFull = 0;
}

/*---------------------------------------------------------------------------*
 NAME      : StageMoveInstr
 PURPOSE   : Move an instruction inside the stage buffer
 PARAMETERS:
 RETURN    :
 *---------------------------------------------------------------------------*/
int StageMoveInstr(int st, int w0, int wk)
{
   int p, q, k;

   memcpy(&(STAGE_BUF[st][w0]), &(STAGE_BUF[st][wk]), sizeof(InstructionSlot));
   STAGE_BUF[st][wk].delay = 0;
   return(0);
}

/*---------------------------------------------------------------------------*
 NAME      : CompactBuffer
 PURPOSE   : Sweep and compact the entries in the given stage buffer
 PARAMETERS:
 RETURN    :
 *---------------------------------------------------------------------------*/
int CompactBuffer(int st)
{
   int p, q, r, s, t, i, j, k, q1, m = 0, s1, sf, sf1, e;

   if (STAGE_LAST[st] == -1) return 0;

//printf("SF0=%2d ", STAGE_LAST[st]);
   sf1 = (STAGE_LAST[st] - 1 + STAGE_SIZ[st]) % STAGE_SIZ[st];
   sf = STAGE_LAST[st];
   /* Realign STAGE_LAST to the first available slot */
   q = -1;
   for (k = 0; k < STAGE_SIZ[st]; ++k) {
      p = (sf - 1 - k + STAGE_SIZ[st]) % STAGE_SIZ[st];
      if (STAGE_BUF[st][p].delay > 0) { q = p; break; }
   }
   if (q >= 0) STAGE_LAST[st] = (q + 1 + STAGE_SIZ[st]) % STAGE_SIZ[st];
/*
printf("SF1=%2d ", STAGE_LAST[st]);
for (k = 0; k < AA.win_size; ++k) {
    printf("%2d=%-4s", k , STAGE_BUF[st][k].delay > 0 ? OPNAME[STAGE_BUF[st][k].ip->opcode] : "----");
}
printf("\n");
*/


   /* Compact instructions to maintain order */
   sf = STAGE_LAST[st];
   q = -1;
   /* search next instruction */
   for (j = 0; j < STAGE_SIZ[st]; ++j) {
      r = (sf + j) % STAGE_SIZ[st];
      if (STAGE_BUF[st][r].delay > 0) { q = r; break; }
   }
   if (q >= 0) {
      e = (sf - q + STAGE_SIZ[st]) % STAGE_SIZ[st];
      /* search first hole < q */
      for (i = 0; i < e - 1; ++i) {
         s1 = q + 1 + i;
         s = (q + 1 + i) % STAGE_SIZ[st];
         if (STAGE_BUF[st][s].delay == 0) {  
            for (k = 0; k < e - 2 - i; ++k) {
               t = (s + k + 1) % STAGE_SIZ[st];
               if (STAGE_BUF[st][t].delay > 0) {
                  StageMoveInstr(st, s, t);
                  ++m;
                  break;
               }
            }
         }
      }
   }
   q = -1;
   for (k = 0; k < STAGE_SIZ[st]; ++k) {
      p = (sf - 1 - k + STAGE_SIZ[st]) % STAGE_SIZ[st];
      if (STAGE_BUF[st][p].delay > 0) { q = p; break; }
   }
   if (q >= 0) STAGE_LAST[st] = (q + 1 + STAGE_SIZ[st]) % STAGE_SIZ[st];
/*
printf("SF1=%2d ", STAGE_LAST[st]);
for (k = 0; k < AA.win_size; ++k) {
    printf("%2d=%-4s", k , STAGE_BUF[st][k].delay > 0 ? OPNAME[STAGE_BUF[st][k].ip->opcode] : "----");
}
printf("\n");
*/
   return 0;
}

/*---------------------------------------------------------------------------*
 NAME      : ReleasePhysicalReg
 PURPOSE   : Release a Physical Register and update the Register Map
 PARAMETERS:
 RETURN    :
 *---------------------------------------------------------------------------*/
int ReleasePhysicalReg(int pn)
{
//   if (debug) printf("pn=%d\n", pn);
   RM[pn].busy = 0; 
   RM[pn].qi = 1; 
   --PRAllocated;
   return 0;
}

/*---------------------------------------------------------------------------*
 NAME      : GetPhysicalReg
 PURPOSE   : Get a Physical Register and update the Register Map
 PARAMETERS:
 RETURN    :
 *---------------------------------------------------------------------------*/
int GetPhysicalReg(int *pn)
{
   int j, k, found; 

   found = 0;
   for (k = 0; k < AA.pregs; ++k) {
      j = 1 + (PRHead - 1 + k) % AA.pregs;
      if (RM[j].busy == 0) {
         *pn = j;
         RM[j].busy = 1;
         found = 1;
         PRHead = 1 + j % AA.pregs;
         ++PRAllocated;
         break;
      } 
   }
//   if (! found) ++PRStalls;
   if (! found) LogStall(&PRStalls, "Physical registers not available");

   return (found);
}

/*---------------------------------------------------------------------------*
 NAME      : InstructionMemoryRead
 PURPOSE   : Read an instruction from the Instruction Memory
             and allocate associated metadata structures
 PARAMETERS:
 RETURN    :
 *---------------------------------------------------------------------------*/
Instruction *InstructionMemoryRead()
{
   Instruction *ip1, *ipn;
   int n; u32 mypc = PC;

   if (debug) printf(" mypc=%d  PI_n=%d\n",mypc, PI_n);
   if (mypc < PI_n) {
      ip1 = (Instruction *)Queue__Read(mypc, PI);
   } else {
      ip1 = &NULL_Instruction;
      StreamEnd = 1;
      n = Queue__Count(DYNSTREAM);
      LastInst = (Instruction *) Queue__Read(n - 1, DYNSTREAM);
   }
   ipn = (Instruction *) Malloc(sizeof(Instruction));          
   if (ipn == NULL) ExitProg("Cannot allocate memory for 1 instruction.");
   memcpy((void *)ipn, (void *)ip1, sizeof(Instruction));

   return (ipn);
}

/*---------------------------------------------------------------------------*
 NAME      : Branch_Prediction
 PURPOSE   : Implement branch prediction
 PARAMETERS:
 RETURN    :
 *---------------------------------------------------------------------------*/
void Branch_Prediction(u32 mypc, Instruction *ip)
{
   /* Always taken for now */
   ip->NPC = mypc + ip->rs3;
   ip->bdir = 1;

//PATCH
   ++bne_count;
   if (bne_count == G.iterations) {
      ip->NPC = mypc + 1;
      ip->bdir = 1;
   }
}

/*---------------------------------------------------------------------------*
 NAME      : FETCH_DO
 PURPOSE   : Execute the Fetch stage
 PARAMETERS:
 RETURN    :
 *---------------------------------------------------------------------------*/
int FETCH_DO(Instruction *ip)
{
   int flag = 1;
   Instruction *ip1, *nip;

   Queue__Insert((char *)ip, DYNSTREAM);
   ip->CPC = PC;
   ip->CIC = IC;
   if (ip->opcode == BNE || ip->opcode == BEQ) {
      nip = ip;
      Branch_Prediction(PC + 1, ip);
      PC = nip->NPC;
   } else {
      ++PC;
   }
   ++IC;
   if (ip->bdir) flag = 0;
   if (verbose) printf("* FETCH: %4s/%03d flag=%d PC=%02d\n", OPNAME[ip->opcode], ip->CIC, flag, ip->CPC);

   return (flag);
}

/*---------------------------------------------------------------------------*
 NAME      : RENAME_DO
 PURPOSE   : Execute the Rename stage
 PARAMETERS:
 RETURN    :
 *---------------------------------------------------------------------------*/
int RENAME_DO(Instruction *ip)
{
   int k, pold = -1, p0 = -1, p1 = -1, p2 = -1, p3 = -1, flag = 1;
   int p1new = 1, p2new = 1, p3new = 1;
   int p1qi = 0, p2qi = 0, p3qi = 0;
   int p1alloc = 0, p2alloc = 0, p3alloc = 0;
   int flag0 = 0, flag1 = 0, flag2 = 0, flag3 = 0;
   u32 vi1, vi2, vi3;

   /* Read current mappings, allocate physical registers */
   if (ip->rs1 >= 0 && ip->rs1 <= AA.lregs) {
      if (RF[ip->rs1].pn == -1) {
         if (! GetPhysicalReg(&p1)) {
            ++FinishedPR;
            flag = 0;
         } else {
            p1alloc = 1;
            p1new = RF[ip->rs1].qi;
            p1qi = RF[ip->rs1].qi;
            flag1 = 1;
            vi1 = RF[ip->rs1].vi;
         }
      } else {
         p1 = RF[ip->rs1].pn;
         p1new = RF[ip->rs1].qi;
         p1qi  = RF[ip->rs1].qi;
         flag1 = 1;
         vi1 = RF[ip->rs1].vi;
      }
   }
   if (ip->imv == 2) {
      p2 = ip->rs2;
      p2new = 0;
      flag2 = 1;
   } else {
      if (ip->rs2 >= 0 && ip->rs2 <= AA.lregs) {
         if (RF[ip->rs2].pn == -1) {
            if (! GetPhysicalReg(&p2)) {
               ++FinishedPR;
               flag = 0;
               if (p1alloc > 0) ReleasePhysicalReg(p1);
            } else {
               p2alloc = 1;
               p2new = RF[ip->rs2].qi;
               p2qi = RF[ip->rs2].qi;
               flag2 = 1;
               vi2 = RF[ip->rs2].vi;
            }
         } else {
            p2 = RF[ip->rs2].pn;
            p2new = RF[ip->rs2].qi;
            p2qi  = RF[ip->rs2].qi;
            flag2 = 1;
            vi2 = RF[ip->rs2].vi;
         }
      }
   }
   if (ip->imv == 1) {
      p3 = ip->rs3;
      p3new = 0;
      flag3 = 1;
   } else {
      if (ip->rs3 >= 0 && ip->rs3 <= AA.lregs) {
         if (RF[ip->rs3].pn == -1) {
            if (! GetPhysicalReg(&p3)) {
               ++FinishedPR;
               flag = 0;
               if (p1alloc > 0) ReleasePhysicalReg(p1);
               if (p2alloc > 0) ReleasePhysicalReg(p2);
            } else {
               p3alloc = 1;
               p3new = RF[ip->rs3].qi;
               p3qi = RF[ip->rs3].qi;
               flag3 = 1;
               vi3 = RF[ip->rs3].vi;
            }
         } else {
            p3 = RF[ip->rs3].pn;
            p3new = RF[ip->rs3].qi;
            p3qi  = RF[ip->rs3].qi;
            flag3 = 1;
            vi3 = RF[ip->rs3].vi;
         }
      }
   }
   if (ip->rd >= 0 && ip->rd <= AA.lregs) {
      if (! GetPhysicalReg(&p0)) {
         if (p1alloc > 0) ReleasePhysicalReg(p1);
         if (p2alloc > 0) ReleasePhysicalReg(p2);
         if (p3alloc > 0) ReleasePhysicalReg(p3);
         ++FinishedPR;
         flag = 0;
      } else {
         flag0 = 0;
      }
   }
   if (verbose) {
      printf("* RENAME: %4s/%03d flag=%d p0=%-3d p1=%-3d p2=%-3d p3=%-3d p1new=%-2d p2new=%-2d p3new=%-2d p1qi=%d p2qi=%d p3qi=%d vi1=%08X vi2=%08X vi3=%08X imm=%d rs1=%-2d rs2=%-2d rs3=%-2d\n", OPNAME[ip->opcode], ip->CPC, flag, p0, p1, p2, p3, p1new, p2new, p3new, p1qi, p2qi, p3qi, vi1, vi2, vi3, ip->imv, ip->rs1, ip->rs2, ip->rs3);
   }


   if (flag) {
      if (p1 != -1) { RF[ip->rs1].pn = p1; ip->ps1 = p1; RM[p1].ri = ip->rs1;
//ip->qj = p1new; 
                      RF[ip->rs1].qi = p1new; RM[p1].qi = p1qi; RM[p1].vi = p1qi ? RM[p1].vi : vi1; }
      if (p2 != -1) { RF[ip->rs2].pn = p2; ip->ps2 = p2; RM[p2].ri = ip->rs2; 
//ip->qk = p2new; 
                      RF[ip->rs2].qi = p2new; RM[p2].qi = p2qi; }
      if (ip->imv == 2) {
         ip->ps2 = p2;
//ip->qk = p2new;
      } else {
         if (p2 != -1) { RF[ip->rs2].pn = p2; ip->ps2 = p2; RM[p2].ri = ip->rs2;
//ip->qk = p2new;
                         RF[ip->rs2].qi = p2new; RM[p2].qi = p2qi; RM[p2].vi = p2qi ? RM[p2].vi : vi2; }
      }
      if (ip->imv == 1) {
         ip->ps3 = p3;
//ip->ql = p3new;
      } else {
         if (p3 != -1) { RF[ip->rs3].pn = p3; ip->ps3 = p3; RM[p3].ri = ip->rs3;
//ip->ql = p3new; 
                         RF[ip->rs3].qi = p3new; RM[p3].qi = p3qi; RM[p3].vi = p3qi ? RM[p3].vi : vi3; }
      }
      if (p0 != -1) { pold = RF[ip->rd].pn; RF[ip->rd].pn = p0; ip->pd = p0; ip->pold = pold; RM[p0].ri = ip->rd; RF[ip->rd].qi = 1; RM[p0].qi = 1; }
      ip->qi = 1;
   }

   return (flag);
}

/*---------------------------------------------------------------------------*
 NAME      : DISPATCH_DO
 PURPOSE   : Execute the Dispatch stage
 PARAMETERS:
 RETURN    :
 *---------------------------------------------------------------------------*/
int DISPATCH_DO(Instruction *ip)
{
   int p, q, k, wk=-1, rk=-1, flag = 1, found;

   if (! CheckROBEntry() || ! CheckIWEntry()) flag = 0;

   if (flag == 1) {

      GetROBEntry(&rk);
      GetIWEntry(&wk);

      ip->winn = wk;
      IW[wk].opcode    = ip->opcode;
      IW[wk].pi        = ip->pd;
      IW[wk].pj        = ip->ps1;
      IW[wk].pk        = ip->ps2;
      IW[wk].pl        = ip->ps3;
      IW[wk].qj        = ip->ps1 != -1 ? RM[ip->ps1].qi : 0;
      IW[wk].qk        = ip->imv == 2 ? 0 : (ip->ps2 != -1 ? RM[ip->ps2].qi : 0);
      IW[wk].ql        = ip->imv == 1 ? 0 : (ip->ps3 != -1 ? RM[ip->ps3].qi : 0);
//      IW[wk].cj        = ((ip->ps1 == -1 || ip->opcode == STORE || ip->opcode == BNE) ? -1 : IW[wk].qj == 0 ? CK : -2);
      IW[wk].cj        = ((ip->ps1 == -1 || ip->opcode == STORE) ? -1 : IW[wk].qj == 0 ? CK : -2);
//      IW[wk].cj        = ((ip->ps1 == -1 || ip->opcode == BNE) ? -1 : IW[wk].qj == 0 ? CK : -2);
//      IW[wk].cj        = ((ip->ps1 == -1 || ip->opcode == STORE || ip->opcode == BNE|| ip->opcode == ADDI) ? -1 : IW[wk].qj == 0 ? CK : -2);
      IW[wk].ck        = (ip->ps2 == -1 ? -1 : IW[wk].qk == 0 ? CK : -2);
//      IW[wk].ck        = ((ip->ps2 == -1 || ip->opcode == ADDI) ? -1 : IW[wk].qk == 0 ? CK : -2);
//      IW[wk].ck        = ((ip->ps2 == -1 || ip->opcode == STORE || ip->opcode == BNE|| ip->opcode == ADDI) ? -1 : IW[wk].qk == 0 ? CK : -2);
//      IW[wk].cl        = ((ip->ps3 == -1 || ip->opcode == STORE2 || ip->opcode == BNE) ? -1 : IW[wk].ql == 0 ? CK : -2);
      IW[wk].cl        = ((ip->ps3 == -1 || ip->opcode == STORE || ip->opcode == LOAD || ip->opcode == STORE2 || ip->opcode == BNE || ip->opcode == BEQ || ip->opcode == ADDI) ? -1 : IW[wk].ql == 0 ? CK : -2);
//      IW[wk].cl        = ((ip->ps3 == -1 || ip->opcode == LOAD || ip->opcode == BNE || ip->opcode == ADDI) ? -1 : IW[wk].ql == 0 ? CK : -2);
      IW[wk].imv       = ip->imv;
      IW[wk].rob_entry = rk;
      IW[wk].ip        = ip;
      IW[wk].delay     = 0;

      ip->robn = rk;
      RB[rk].PC    = ip->CPC;
      RB[rk].ri    = ip->rd;
      RB[rk].piold = ip->pold;
      RB[rk].st    = ip->store;
      RB[rk].exc   = 0;
      RB[rk].cplt  = 0;
      RB[rk].ip    = ip;
   }

   if (verbose) {
      printf("* DISPATCH: %4s/%03d flag=%d wk=%d rk=%d qj=%d qk=%d ql=%d\n", OPNAME[ip->opcode], ip->CIC, flag, wk, rk, IW[wk].qj, IW[wk].qk, IW[wk].ql);
   }

   return (flag);
}

/*---------------------------------------------------------------------------*
 NAME      : ISSUE_DO
 PURPOSE   : Execute the Issue steage
 PARAMETERS:
 RETURN    :
 *---------------------------------------------------------------------------*/
int ISSUE_DO(Instruction *ip)
{
   int flag = 1, fn = 0, t = ip->optype, k = ip->winn, is_store;

//printf("\nspec=%d\n",AA.speculation);
   if (debug) printf("  in ISSUE: %4s/%03d qj=%d qk=%d bdir=%d\n", OPNAME[ip->opcode], ip->CIC, IW[k].qj, IW[k].qk, ip->bdir);
//   if (debug) printf("  in ISSUE: ip 0x%lx IW.ip 0x%lx  ip.qj %d ip.qk %d ip.ql %d\n", ip, IW[k].ip, ip->qj, ip->qk, ip->ql);
   switch (ip->opcode) {
      case STORE:
//         if (IW[k].qj != 0 && ip->bdir == 0) flag = 0;
         if (IW[k].qk != 0 && ip->bdir == 0) flag = 0;
         break;
      case STORE2:
         if ((IW[k].qj != 0 || IW[k].qk != 0) && ip->bdir == 0) flag = 0;
         break;
      case LOAD:
         if (IW[k].qj != 0 && ip->bdir == 0) flag = 0;
         break;
      case LOAD2:
         if ((IW[k].qj != 0 || IW[k].qk != 0) && ip->bdir == 0) flag = 0;
         break;
      case BNE: case BEQ:
         if (AA.speculation) break;
         if ((IW[k].qj != 0 || IW[k].qk != 0)) flag = 0;
         break;
      default:
         if ((IW[k].qj != 0 || IW[k].qk != 0) && ip->bdir == 0) flag = 0;
         break;
   }

   if (IW[k].delay > 0 ) { IW[k].delay--; flag = -2; }

   if (flag > 0) {
      fn = CheckFU(ip);
      if (fn) {
         ip->fup1 = GetFU(ip, fn);
         ReleaseIWEntry(k);
      } else {
         flag = -1;
      }
   }
   if (verbose) {
      printf("* ISSUE: %4s/%03d flag=%d optype=%d(%s) fup1=%4s --> %s\n", \
         OPNAME[ip->opcode], ip->CIC, flag, t, FUNAME[t], \
         ip->fup1 != NULL ? FUNAME[(ip->fup1)->ip->optype] : "NULL" \
         , flag == 1 ? "OK" : flag == 0 ? "NOT_READY" : flag == -1 ? "NO_FU" : flag == -2 ? "DELAYED" : "???" \
      );
   }
   if (flag < 0) flag = 0; //re-adjust flag

   return (flag);
}

/*---------------------------------------------------------------------------*
 NAME      : EXECUTE_DO
 PURPOSE   : Execute the Execute stage
 PARAMETERS:
 RETURN    :
 *---------------------------------------------------------------------------*/
int EXECUTE_DO(Instruction *ip)
{
   u32 efad;
   int flag = 0, t = ip->optype;

//   if (debug) printf("  in EXECUTE: %4s/%03d qj=%d qk=%d ql=%d  fup1=%s\n", OPNAME[ip->opcode], ip->CIC, ip->qj, ip->qk, ip->ql, FUNAME[(ip->fup1)->ip->optype]);
   if (debug) printf("  in EXECUTE: %4s/%03d  fup1=%s\n", OPNAME[ip->opcode], ip->CIC, FUNAME[(ip->fup1)->ip->optype]);

  /* do actual operation*/
   switch (ip->opcode) {
      case BNE: case BEQ:
         ip->fup = ip->fup1; flag = 1;
         break;
      case MUL:
         RM[ip->pd].vi = RM[ip->ps1].vi * RM[ip->ps2].vi;
         ip->fup = ip->fup1; flag = 1;
         break;
      case ADD:
         RM[ip->pd].vi = RM[ip->ps1].vi + RM[ip->ps2].vi;
         ip->fup = ip->fup1; flag = 1;
         break;
      case ADDI:
         RM[ip->pd].vi = RM[ip->ps1].vi + ip->ps3;
         ip->fup = ip->fup1; flag = 1;
         break;
      case LOAD:
         efad = RM[ip->ps1].vi + ip->ps3;
         ip->efad = efad;
//Display("L efad=%08X",efad);
         ip->fup = ip->fup1; flag = 1;
        break;
      case STORE:
//         ea = RM[ip->ps1].vi + ip->ps2;
         efad = RM[ip->ps2].vi + ip->ps3;
         ip->efad = efad;
//Display("S efad=%08X",efad);
if (!STOREWAITS || (STOREWAITS && ip->qi == 0)) {
         ip->fup = ip->fup1; flag = 1;
}
         break;
      case LOAD2:
         efad = RM[ip->ps1].vi + RM[ip->ps2].vi;
         ip->efad = efad;
//Display("L efad=%08X  -- P%d P%d x%d x%d %08X  %08X",efad,ip->ps1,ip->ps2,RM[ip->ps1].qi,RM[ip->ps2].qi,RM[ip->ps1].vi,RM[ip->ps2].vi);
         ip->fup = ip->fup1; flag = 1;
         break;
      case STORE2:
//         efad = RM[ip->ps2].vi + RM[ip->ps2].vi;
//         efad = RM[ip->ps2].vi + RM[ip->ps3].vi;
         efad = RM[ip->ps1].vi + RM[ip->ps2].vi;
         ip->efad = efad;
//Display("S efad=%08X",efad);
if (!STOREWAITS || (STOREWAITS && ip->qi == 0)) {
         ip->fup = ip->fup1; flag = 1;
}
         break;
      case NOP:
      default:
         ip->fup = ip->fup1; flag = 1;
         break;
   }

   if (verbose) {
      printf("* EXECUTE: %4s/%03d flag=%d winn=%d robn=%d pd=%d\n", OPNAME[ip->opcode], ip->CIC, flag, ip->winn, ip->robn, ip->pd);
   }
   return (flag);
}

/*---------------------------------------------------------------------------*
 NAME      : COMPLETE_DO
 PURPOSE   : Execute the Complete stage
 PARAMETERS:
 RETURN    :
 *---------------------------------------------------------------------------*/
int COMPLETE_DO(Instruction *ip)
{
   int k, p, flag = 0, nldone = 0, reg, ok;

   if (debug) printf("  in COMPLETE: %-3s/%03d: ", OPNAME[ip->opcode], ip->CPC);

//   ReleaseFU(ip);

   /* Write back registers and common data bus */
   if ((! ip->store) && ip->opcode != BNE && ip->opcode != BEQ) {
      // Update Register Map (Physical Registers)
      RM[ip->pd].qi = 0;

      RF[RM[ip->pd].ri].vi = RM[ip->pd].vi;
      if (RF[RM[ip->pd].ri].pn == ip->pd) RF[RM[ip->pd].ri].qi = 0;
      if (debug) printf("P%d.qi=%d  x%d.qi=%d\n", ip->pd, RM[ip->pd].qi, RM[ip->pd].ri, RF[RM[ip->pd].ri].qi);


      /* Update Instruction Window */
      for (k = 0; k < AA.win_size; ++k) {
         if (IW[k].busy) {
            if (debug) printf("  IW[%d] %-3s/%03d pj=%d pk=%d pl=%d", \
               k, OPNAME[(IW[k].ip)->opcode], (IW[k].ip)->CPC, IW[k].pj, IW[k].pk, IW[k].pl);
//            if (IW[k].pj == ip->pd) { IW[k].qj = 0; IW[k].cj = CK; IW[k].ip->qj = 0; IW[k].ip->cj = CK; if (debug) printf(" upd QJ"); }
//            if (IW[k].pk == ip->pd) { IW[k].qk = 0; IW[k].ck = CK; IW[k].ip->qk = 0; IW[k].ip->ck = CK; if (debug) printf(" upd QK"); }
//            if (IW[k].pl == ip->pd) { IW[k].ql = 0; IW[k].cl = CK; IW[k].ip->ql = 0; IW[k].ip->cl = CK; if (debug) printf(" upd QL"); }
//printf(" qj=%d, qk=%d ql=%d", IW[k].qj,IW[k].qk,IW[k].ql);

//            if (IW[k].pj == ip->pd) { IW[k].qj = 0; IW[k].cj = CK; IW[k].ip->cj = CK; if (debug) printf(" upd QJ"); }
//            if (IW[k].pk == ip->pd) { IW[k].qk = 0; IW[k].ck = CK; IW[k].ip->ck = CK; if (debug) printf(" upd QK"); }
            if (IW[k].pj == ip->pd) { IW[k].qj = 0; IW[k].delay = AA.wblat; if (debug) printf(" upd QJ"); }
            if (IW[k].pk == ip->pd) { IW[k].qk = 0; IW[k].delay = AA.wblat; if (debug) printf(" upd QK"); }
//            if (IW[k].pj == ip->pd) { IW[k].qj = 0; IW[k].cj = CK; IW[k].ip->cj = CK; if (debug) printf(" upd QJ"); }
//            if (IW[k].pk == ip->pd) { IW[k].qk = 0; IW[k].ck = CK; IW[k].ip->ck = CK; if (debug) printf(" upd QK"); }

//            if (IW[k].pl == ip->pd) { IW[k].ql = 0; IW[k].cl = CK; IW[k].ip->cl = CK; if (debug) printf(" upd QL"); }
//            if (IW[k].pj == ip->pd && IW[k].imv ==0) { IW[k].qj = 0; IW[k].cj = CK; IW[k].ip->cj = CK; if (debug) printf(" upd QJ"); }
//            if (IW[k].pk == ip->pd && IW[k].imv ==0) { IW[k].qk = 0; IW[k].ck = CK; IW[k].ip->ck = CK; if (debug) printf(" upd QK"); }

//            if (IW[k].pl == ip->pd && IW[k].imv ==0) { IW[k].ql = 0; IW[k].cl = CK; IW[k].ip->cl = CK; if (debug) printf(" upd QL"); }
            if (IW[k].pl == ip->pd && IW[k].imv ==0) { IW[k].ql = 0; IW[k].delay = AA.wblat; if (debug) printf(" upd QL"); }
            if (debug) printf(",");
         }
      }
      if (debug) { nldone = 1; printf("\n"); }

      CB[CBHead++] = ip;
   }
   if (ip->store) {
   }

   /* Update Load/Store Queue */
   for (k = 1; k <= SQElems; ++k) {
      p = (SQTail - k + AA.sqsize) % AA.sqsize;

      if (SQ[p]->opcode == STORE)  reg = SQ[p]->ps1;
      if (SQ[p]->opcode == STORE2) reg = SQ[p]->ps3;
      if (reg == ip->pd) { 
         SQ[p]->qi = 0;
         if (SQ[p]->opcode == STORE)  SQ[p]->cj = CK;
         if (SQ[p]->opcode == STORE2) SQ[p]->cl = CK;
if (!STOREWAITS) {
         if(debug) printf("\n");
         ok = InsertIntoStageBuffer(SQ[p], EXECUTE);
         if (ok != 1) ExitProg("Cannot propagate executed-store info");
}
      }
   }
   /* Update Load/Store Queue */
   for (k = 1; k <= LQElems; ++k) {
      p = (LQTail - k + AA.lqsize) % AA.lqsize;
//         if (SQ[p]->ps3 == ip->pd) { SQ[p]->ql = 0; }
      if (LQ[p]->opcode == LOAD)  if (LQ[p]->pd == ip->pd) { LQ[p]->qi = 0; LQ[p]->cj = CK; }
      if (LQ[p]->opcode == LOAD2) if (LQ[p]->pd == ip->pd) { LQ[p]->qi = 0; LQ[p]->cj = CK; }
//if (debug) printf("\ncj=%d\n",  LQ[p]->cj);
   }

/*
   if (ip->store) {
//      if (!SQEmpty) if (SQ[SQHead]->ql == 0) { PopSQ(); }
      if (!SQEmpty) if (SQ[SQHead]->qj == 0) {
         if (debug) if (!nldone) { nldone = 1; printf("\n"); }
         PopSQ();
      }
   }
*/

   RB[ip->robn].cplt = 1;
   flag = 1;

   if (debug) if (!nldone) printf("\n");
   if (verbose) {
      printf("* COMPLETE: %4s/%03d flag=%d winn=%d robn=%d pd=%d\n", OPNAME[ip->opcode], ip->CIC, flag, ip->winn, ip->robn, ip->pd);
   }

   return (flag);
}

/*---------------------------------------------------------------------------*
 NAME      : COMMIT_DO
 PURPOSE   : Execute the Commit stage
 PARAMETERS:
 RETURN    :
 *---------------------------------------------------------------------------*/
int COMMIT_DO(Instruction *ip)
{
   Instruction *ip1;
   int k, j = 0, start = 0, flag = 0; 

//   if (ip == NULL) return (flag);
/*
   if ((! ip->store) && ip->opcode != BNE) {
      // Update Physical Registers
      RF[RM[ip->pd].ri].vi = RM[ip->pd].vi;
      if (RF[RM[ip->pd].ri].pn == ip->pd) RF[RM[ip->pd].ri].qi = 0;
      if (debug) printf("  in COMMIT: P%d.qi=%d  x%d.qi=%d\n", ip->pd, RM[ip->pd].qi, RM[ip->pd].ri, RF[RM[ip->pd].ri].qi);
   }
*/

   flag = ReleaseROBEntry(&ip1);
   if (flag == 1) {
      if (ip1 == LastInst) IsEnd = 1;
   }
   if (verbose) {
      printf("* COMMIT: %4s/%03d flag=%d RBHead=%d RBTail=%d RBEmpty=%d RBFull=%d\n", flag ? OPNAME[ip1->opcode] : "----", flag ? ip1->CIC : 0, flag, RBHead, RBTail, RBEmpty, RBFull);
   }

//   return (flag);
   return (flag);
}

/*---------------------------------------------------------------------------*
 NAME      : FETCH_END
 PURPOSE   : Final operations for the Fetch stage
 PARAMETERS:
 RETURN    :
 *---------------------------------------------------------------------------*/
int FETCH_END(Instruction *ipdummy)
{
   int k;
   Instruction *ip;
   InstructionSlot *isp = STAGE_BUF[FETCH];

   if (verbose) {
//      for (k = 0; k < 4; ++k) {
      for (k = 0; k < AA.f_width; ++k) {
         ip = isp[k].ip;
         printf("  FETCH_END:%4s k=%d delay=%d PC=%02d IC=%02d SF=%d\n", OPNAME[ip->opcode], k, isp[k].delay, ip->CPC, ip->CIC, STAGE_LAST[FETCH]);
      }
   }
   return(0);
}

/*---------------------------------------------------------------------------*
 NAME      : RENAME_END
 PURPOSE   : Final operations for the Rename stage
 PARAMETERS:
 RETURN    :
 *---------------------------------------------------------------------------*/
int RENAME_END(Instruction *ipdummy)
{
   return(0);
}

/*---------------------------------------------------------------------------*
 NAME      : DISPATCH_END
 PURPOSE   : Final operations for the Dispatch stage
 PARAMETERS: 
 RETURN    :
 *---------------------------------------------------------------------------*/
int DISPATCH_END(Instruction *ipdummy)
{
   int j, k;

   InstructionSlot *isp1;
   Instruction *ip1, *ip2;

   /* Analyze if any of the instruction to be possibly dispacthed (currently 
      in DISPATCH stage) have as source register my (completed) destination
      register*/
   for (j = 0; j < AA.w_width; ++j) {
      if (CB[j] != &NULL_Instruction) {
         ip2 = CB[j];
         isp1 = STAGE_BUF[DISPATCH];
      }
   }
   return(0);
}

/*---------------------------------------------------------------------------*
 NAME      : ISSUE_END
 PURPOSE   : Final operatins for the Issue stage
 PARAMETERS:
 RETURN    :
 *---------------------------------------------------------------------------*/
int ISSUE_END(Instruction *ipdummy)
{
   int k, fa, nldone; 
   Instruction *ip;
   InstructionSlot *sbp = STAGE_BUF[ISSUE];

   /* Model pipelining by freeing FUs at the end of stage */
/*
   for (fa = 1; fa <= MAXFT; ++fa) {
      for (k = 1; k <= FA[fa].tot; ++k) {
         if (FA[fa].pipe && fa != L_FU && FA[fa].FU[k].busy1) { FA[fa].FU[k].busy1 = 0; --FA[fa].busy2; }
      }
   }
*/
   for (k = 0; k < AA.i_width; ++k) {
      if (debug) { nldone = 0; printf("  ISSUE_END: I[%d]: ", k); }
      ip = sbp[k].ip;
      if (ip != NULL) {
         if (debug) { nldone = 1; printf("  %4s/%03d d=%d\n", OPNAME[ip->opcode], ip->CIC, sbp[k].delay); }
         if (sbp[k].delay == 1) ReleaseFU(ip);
         if (sbp[k].delay == 2 && ip->optype == L_FU) ReleaseFU(ip);
         if (!STOREWAITS) if (sbp[k].delay == 2 && ip->optype == S_FU) ReleaseFU(ip);
      }
      if (debug) if (!nldone) printf("\n");
   }

// ReleaseFU(ip);
   return(0);
}

/*---------------------------------------------------------------------------*
 NAME      : EXECUTE_END
 PURPOSE   : Final operations for the Execute stage
 PARAMETERS:
 RETURN    :
 *---------------------------------------------------------------------------*/
int EXECUTE_END(Instruction *ipdummy)
{
   int k, fa, nldone; 
   Instruction *ip;
   InstructionSlot *sbp = STAGE_BUF[EXECUTE];

   /* Model pipelining by freeing FUs at the end of stage */
/*
   for (fa = 1; fa <= MAXFT; ++fa) {
      for (k = 1; k <= FA[fa].tot; ++k) {
         if (FA[fa].pipe && fa != L_FU && FA[fa].FU[k].busy1) { FA[fa].FU[k].busy1 = 0; --FA[fa].busy2; }
      }
   }
*/
   for (k = 0; k < TotalFU; ++k) {
      ip = sbp[k].ip;
      if (ip != NULL) {
         if (debug) { nldone = 0; printf("  EXECUTE_END: X[%d]: ", k); }
         if (debug) { nldone = 1; printf("  %4s/%03d d=%d\n", OPNAME[ip->opcode], ip->CIC, sbp[k].delay); }
         if (!STOREWAITS) if (ip->store) RemoveFromStageBuffer(ip, EXECUTE);
         if (debug) if (!nldone) printf("\n");
      }
   }
   return(0);
}

/*---------------------------------------------------------------------------*
 NAME      : COMPLETE_END
 PURPOSE   : Final operations for the Complete stage
 PARAMETERS:
 RETURN    :
 *---------------------------------------------------------------------------*/
int COMPLETE_END(Instruction *ipdummy)
{
   int ec = 0;
   int flag = 1;
   int nldone;
   int dmemdone = 0;

   if (debug) { nldone = 0; printf("  COMPLETE_END:"); }
   if (!LQEmpty) if (LQ[LQHead]->qi == 0) { printf("\n"); nldone = 1; PopLQ(); dmemdone = 1; }
//   if (!SQEmpty) if (SQ[SQHead]->opcode == STORE) if (SQ[SQHead]->qi == 0) { printf("\n"); nldone = 1; PopSQ(); dmemdone = 1; }
//   if (!SQEmpty) if (SQ[SQHead]->opcode == STORE2) if (SQ[SQHead]->qi == 0) { printf("\n"); nldone = 1; PopSQ(); dmemdone = 1; }
   if (!SQEmpty) if (SQ[SQHead]->qi == 0) { printf("\n"); nldone = 1; PopSQ(); dmemdone = 1; }
   if (debug) if (!nldone) printf("\n");

   return (flag);
}

/*---------------------------------------------------------------------------*
 NAME      : COMMIT_END
 PURPOSE   : Final operations for the Commit stage
 PARAMETERS:
 RETURN    :
 *---------------------------------------------------------------------------*/
int COMMIT_END(Instruction *ipdummy)
{
   int k;

   /* Clean up Complete Buffer just before Complete Stage */
   for (k = 0; k < AA.w_width; ++k) { CB[k] = &NULL_Instruction; }
   CBHead = 0;

   //TODO; for the committed instructions, write the results of the Pregs into the Lregs
   return(0);
}

/*---------------------------------------------------------------------------*
 NAME      : Stage
 PURPOSE   : Move instructions from current to next stage IFF resources are
             avaiable in the next stage
 PARAMETERS: int st - current stage id
 GLOBALS   : int STAGE_LAST - 
 RETURN    : int ec - error code: 0==OK
 *---------------------------------------------------------------------------*/
int Stage(int st)
{
    int ec = 0; // error code

   int l0, l1, j, k, p, q, found, r, start = 0, start0 = 0, doorder = 0, stageend = 0, dowait;
   Instruction *ip0, *ip1, *ip2, *ipselected;
   int n = STAGE_SIZ[st - 1], m = STAGE_SIZ[st], needln, cicselected;
   InstructionSlot *prev = STAGE_BUF[st - 1], *curr = STAGE_BUF[st];
   char aux[SCREENWIDTH+20];

    // 
    if (STAGE_INORDER[st])     { start  = (STAGE_LAST[st]     != -1) ? STAGE_LAST[st]     : 0; }
    if (STAGE_INORDER[st - 1]) { start0 = (STAGE_LAST[st - 1] != -1) ? STAGE_LAST[st - 1] : 0; }   
//   start0 = (STAGE_LAST[st - 1] != -1) ? STAGE_LAST[st - 1] : 0;
   if (verbose) {
      printf("------ Start %s(%s): %s[%d..](max=%d)-->%s[%d..](max=%d)\n", STAGE_NAME[st], STAGE_ACR[st], STAGE_ACR[st-1], start0, n, STAGE_ACR[st], start, m);
   }

   needln = 0;
   for (j = 0; j < n; ++j) {
      q = (j + start0) % n;
//      l0 = (prev[q].ip != NULL) ? (((prev[q].ip)->fup != NULL) ? (prev[q].ip)->fup->inilatency : 2) : 2; 
      l0=1;
ipselected = NULL; cicselected = -1;
if (debug) { needln = 1; printf("%s%s[%d].d=%d", j!=0 ? "  ":"  ", STAGE_ACR[st-1], q, prev[q].delay); }
ip1=prev[q].ip;
if (ip1 != 0) { ipselected = ip1; cicselected = ip1->CIC; }
if (debug) { needln = 1; if (ip1 != 0) printf(" %s/%03d", OPNAME[ipselected->opcode], cicselected); else printf(" ---"); }
      if (prev[q].delay == l0 || st == FETCH || st == MAX_STAGE) {
         found = 0; ip0 = NULL;
         for (k = 0; k < m; ++k) {
            p = (k + start) % m;
//ipselected = NULL; cicselected = -1;
if (debug) { needln = 1; printf(" [start=%d p=%d] %s%s[%d].d=%d", start, p, k!=0 ? ", ":" --> ", STAGE_ACR[st], p, curr[p].delay); }
ip2=curr[p].ip;
//if (ip2 != 0) { ipselected = ip2; cicselected = ip2->CIC; }
if (debug) { needln = 1; if (ip2 != 0) printf(" %s/%03d", OPNAME[ip2->opcode], ip2->CIC); else printf(" ---"); }
//            l1 = (curr[p].ip != NULL) ? (((curr[p].ip)->fup != NULL) ? (curr[p].ip)->fup->inilatency - 1: 1) : 1; 
            l1 = 0;
            if (curr[p].delay == l1 || st >= MAX_STAGE - 1) {
               ip0 = (st > FETCH) ? prev[q].ip : InstructionMemoryRead();
               if (StreamEnd && st == FETCH) { stageend = 1; break; }
               if (STAGE_INORDER[st]) STAGE_LAST[st] = (p + 1) % m; //update head
//               STAGE_LAST[st] = (p + 1) % m; //update head
if (debug) { needln = 0; printf(" --> sl=%d found_free_slot(p)=%d\n", STAGE_LAST[st], p); fflush(stdin); }
               //----------------------------------------------------------------------------
//               r=0; if (ip0 != NULL) r = (STAGE_DO[st])(ip0);
               r=0; if (ip0 != NULL || ST_IGNORE_STBUF[st] == 1) { needln = 0; r = (STAGE_DO[st])(ip0); }
//               r = (STAGE_DO[st])(ip0);
               //----------------------------------------------------------------------------
               if (st == FETCH) {
                  --(prev[q].delay);
                  curr[p].ip = ip0;
                  curr[p].delay = 1; 
                  ip0->t[st] = CK;
                  ip0->fup = NULL;
                  found = 1;
               }
//               if (r == 0 && st < MAX_STAGE) {
               if (r == 0 && !ST_IGNORE_FLAG[st]) {
                  if (STAGE_INORDER[st]) { doorder = 1; }
                  break;
               }
               dowait = 0;
               if (st < MAX_STAGE) {
                  curr[p].ip = ip0;
//                  curr[p].delay = curr[p].inidelay;
//                  curr[p].delay = (ip0->fup1 != NULL && ip0->fup == NULL && ip0->store) ? 100 : 1; 
//                  curr[p].delay = (ip0->fup != NULL) ? ip0->fup->inilatency : 1 + (ip0->fup1 != NULL && (ip0->optype == L_FU));
//                  curr[p].delay = (ip0->fup != NULL) ? ip0->fup->inilatency : 1 + (ip0->fup1 != NULL && (ip0->optype == L_FU || ip0->optype == S_FU));
if (ip0->fup != NULL) {
//Display("ciao1\n");
    curr[p].delay = ip0->fup->inilatency;
} else {
//Display("ciao2\n");
    if (ip0->fup1 != NULL && (ip0->optype == L_FU || ip0->optype == S_FU)) {
//Display("ciao3 lat=%d type=%d\n", ip0->fup1->inilatency, ip0->optype);
//        curr[p].delay = ip0->fup1->inilatency;
//        curr[p].delay = 2;
        curr[p].delay = 1;
    } else {
//Display("ciao4\n");
        curr[p].delay = 1;
    }
}

// the above seems to be the cause of the apparent problem of only one less slott for ISSUE !!
//                  curr[p].delay = (ip0->fup != NULL) ? ip0->fup->inilatency : 1;
//                  if (ip0->ql && ip0->store && st >= EXECUTE) { dowait = 1; }
//                  else ip0->t[st] = CK;
                  ip0->t[st] = CK;
                  ip0->fup = NULL;
               }
               if (!dowait) --(prev[q].delay);
               found = 1;
               break;
            } else {
//if (debug) { needln = 0; printf("  NO FREE SLOT or LAST STAGE\n"); }
            }
         }
//         sprintf(aux, "NO SLOTS when moving instuction %s/%03d from stage %s to stage %s.", OPNAME[ip0->opcode], ip0->CIC, STAGE_ACR[st], STAGE_ACR[st+1]);
//*aux='\0';
         if (!stageend && !found && ipselected != NULL) {
            char aux2[10];
            if (ipselected != NULL) strcpy(aux2,OPNAME[ipselected->opcode]); else strcpy(aux2, "----");
            sprintf(aux, "NO SLOTS when trying to move instuction %s/%03d from stage %s to stage %s.", aux2, cicselected, STAGE_ACR[st-1], STAGE_ACR[st]);
            LogStall(&(Stall[st]), aux); 
            if (debug) { needln = 1;  printf(" --> NO SLOT"); }
         }
if (debug) if (needln) { needln = 0; printf("\n"); }
         if (doorder || stageend) { if (debug) printf("\n"); break; }
      }
      if (prev[q].delay > 1) {
         --(prev[q].delay);
      }
      if (found && st == DISPATCH) ec = CompactBuffer(st);
//      if (found && (st == DISPATCH || st == EXECUTE)) ec = CompactBuffer(st);
      if (ec) ExitProg("CompactBuffer has failed in function Stage.");
   }
if (debug) if (needln) { needln = 0; printf("\n"); }
   if (debug) printf("\n");
   //----------------------------------------------------------------------------
   if (!stageend) (STAGE_END[st])(NULL);
   //----------------------------------------------------------------------------

    return (ec);
}

/*---------------------------------------------------------------------------*
 NAME      : print_lregs_args
 PURPOSE   : Pretty-printing of logical registers
 PARAMETERS:
 RETURN    :
 *---------------------------------------------------------------------------*/
void print_lregs_args(Instruction *ip, char *regs, char *labpfx)
{
    int k = 1;
    *regs = '\0'; //empty string (sprintf (regs,"") not liked by GCC 7)

    switch (OPFORMAT[ip->opcode]) {
      case FORMAT_B:
         if (ip->rs1 == -1 || ip->rs2 == -1) return;
         if (0 != strcmp(labpfx, "")) {
            sprintf(regs, "x%d,x%d,%s%d", ip->rs1, ip->rs2, labpfx, k);
         } else {
            sprintf(regs, "x%d,x%d,%d", ip->rs1, ip->rs2, ip->rs3);
         }
         break;
      case FORMAT_I:
         if (ip->rs1 == -1 || ip->rd == -1) return;
         sprintf(regs, "x%d,x%d,%d", ip->rd, ip->rs1, ip->rs3);
         break;
      case FORMAT_I2:
         if (ip->rs1 == -1 || ip->rd == -1) return;
         sprintf(regs, "x%d,%d(x%d)", ip->rd, ip->rs3, ip->rs1);
         break;
      case FORMAT_S2:
//         if (ip->rs1 == -1 || ip->rs3 == -1) return;
         if (ip->rs1 == -1 || ip->rs2 == -1) return;
//         sprintf(regs, "x%d,%d(x%d)", ip->rs3, ip->rs2, ip->rs1);
         sprintf(regs, "x%d,%d(x%d)", ip->rs1, ip->rs3, ip->rs2);
         break;
      case FORMAT_I3:
         if (ip->rs1 == -1 || ip->rd == -1 || ip->rs2 == -1) return;
         sprintf(regs, "x%d,x%d(x%d)", ip->rd, ip->rs2, ip->rs1);
         break;
      case FORMAT_S3:
         if (ip->rs1 == -1 || ip->rs2 == -1 || ip->rs3 == -1) return;
         sprintf(regs, "x%d,x%d(x%d)", ip->rs3, ip->rs2, ip->rs1);
         break;
      case FORMAT_R:
         if (ip->rs1 == -1 || ip->rs2 == -1 || ip->rd == -1) return;
         sprintf(regs, "x%d,x%d,x%d", ip->rd, ip->rs1, ip->rs2);
         break;
      default:
         *regs = '\0'; //empty string (sprintf (regs,"") not liked by GCC 7)
         break;
    }
}

/*---------------------------------------------------------------------------*
 NAME      : print2_pregs_args
 PURPOSE   : Pretty-printing of physical registers
 PARAMETERS:
 RETURN    :
 *---------------------------------------------------------------------------*/
void print2_pregs_args(Instruction *ip, char *regs)
{
   *regs = '\0'; //empty string (sprintf (regs,"") not liked by GCC 7)

   switch (OPFORMAT[ip->opcode]) {
      case FORMAT_B:
         if (ip->ps1 == -1 || ip->ps2 == -1) return;
         sprintf(regs, ",P%d,P%d,%d", ip->ps1, ip->ps2, ip->ps3);
         break;
      case FORMAT_I:
         if (ip->ps1 == -1 || ip->pd == -1) return;
         sprintf(regs, "P%d,P%d,%d", ip->pd, ip->ps1, ip->ps3);
         break;
      case FORMAT_I2:
         if (ip->ps1 == -1 || ip->pd == -1) return;
         sprintf(regs, "P%d,%d(P%d)", ip->pd, ip->ps3, ip->ps1);
         break;
      case FORMAT_S2:
//         if (ip->ps1 == -1 || ip->ps3 == -1) return;
         if (ip->ps1 == -1 || ip->ps2 == -1) return;
//         sprintf(regs, "P%d,%d(P%d)", ip->ps3, ip->ps2, ip->ps1);
         sprintf(regs, ",%d(P%d)<-P%d", ip->ps3, ip->ps2, ip->ps1);
         break;
      case FORMAT_I3:
         if (ip->ps1 == -1 || ip->pd == -1 || ip->ps2 == -1) return;
         sprintf(regs, "P%d,P%d(P%d)", ip->pd, ip->ps2, ip->ps1);
         break;
      case FORMAT_S3:
         if (ip->ps1 == -1 || ip->ps2 == -1 || ip->ps3 == -1) return;
         sprintf(regs, ",P%d(P%d)<-P%d", ip->ps2, ip->ps1, ip->ps3);
         break;
      case FORMAT_R:
         if (ip->ps1 == -1 || ip->ps2 == -1 || ip->pd == -1) return;
         sprintf(regs, "P%d,P%d,P%d", ip->pd, ip->ps1, ip->ps2);
         break;
      default:
         *regs = '\0'; //empty string (sprintf (regs,"") not liked by GCC 7)
         break;
   }
}

/*---------------------------------------------------------------------------*
 NAME      : dump_instruction_program
 PURPOSE   : Print the instructions of the program (used for debugging)
 PARAMETERS:
 RETURN    :
 *---------------------------------------------------------------------------*/
void dump_instruction_program(void)
{
   int k, n = Queue__Count(PI);
   Instruction *ip;
   char buf[32];

   for (k = 0; k < n; ++k) {
      ip = (Instruction *)Queue__Read(k, PI);
      print_lregs_args(ip, buf, "");
      if (ip->opcode > MAXOPCODE - 1) ExitProg("BAD OPCODE in dump_instruction_program");
      Display("%03d) %-4s %-11s", ip->icount + 1, OPNAME[ip->opcode], buf);
   }
}

/*---------------------------------------------------------------------------*
 NAME      : read_program
 PURPOSE   : Read the program from the program-file
 PARAMETERS:
 RETURN    :
 *---------------------------------------------------------------------------*/
void read_program(char *pn)
{
   char buf[128], buf1[32], buf2[32], buf3[32];
   u32 fvers = 0, icount = 0;
   u32 iopcode, ird, irs1, irs2;
   Instruction *ip;
   int i, ret, regn;
   u32 val;
   
  
   Display("* Input program: '%s'  ", pn);
   SSS_fp = FOpen(pn, "r", &SSSClose);
   if (SSS_fp != NULL) {
      ret = fscanf(SSS_fp, "%126[^\n]\n", buf); buf[127] = '\0';
      if (ret == 0 || ret == EOF)
	 ExitProg("ERROR: Empty program file.");
      if (buf[0]=='#' && buf[1]=='!') { /* HEADER */
         sscanf(buf+2, "%d", &fvers); 
      } else {
         ExitProg("ERROR: Missing header.");
      }
      if (debug) { fprintf(stderr, "  File version = %d.\n", fvers); }

      if (fvers != 1) {
         ExitProg("ERROR: Unknown file version '%d'.\n", fvers);
      }
      /* ASSERT: fvers == 1*/

      while (!feof(SSS_fp)) {
         ret = fscanf(SSS_fp, "%126[^\n]\n", buf); buf[127] = '\0';
         if (ret == 0 || ret == EOF)
	    ExitProg("ERROR: Trying to read after file end.");
         if (buf[0]=='#' && buf[1]=='x') {
            RegFromFile = 1; //some regiser is defined in the file
            sscanf(buf+2, "%d %x [^\n]\n", &regn, &val); 
            if (regn >=1 && regn <= AA.lregs) {
               Display("  x%d <-- %08X", regn, val);
               RF[regn].vi = (int)val;
            }
	 } else {
             sscanf(buf, "%d %d %d %d\n", &iopcode, &ird, &irs1, &irs2);
             if (iopcode > MAXOPCODE - 1) ExitProg("BAD OPCODE in program '%s'.", pn);
             ip = (Instruction *) Malloc(sizeof(Instruction));          
             if (ip == NULL) ExitProg("Memory for 1 instruction.");
             ip->icount = icount;
             ip->opcode = iopcode;
             ip->NPC = 0;
             ip->bdir = 0; /* Default: predict Not-taken */
             ip->ps1 = -1;
             ip->ps2 = -1;
             ip->ps3 = -1;
             ip->pd = -1;
             ip->pold = -1;
             ip->robn = -1;
             ip->winn = -1;
             strcpy(ip->iws, "");
             strcpy(ip->rbs, "");
             ip->inqueue = 0;
             ip->qi = 1;
    //         ip->qj = 0;
    //         ip->qk = 0;
    //         ip->ql = 0;
             ip->cj = -2;
             ip->ck = -2;
             ip->cl = -2;
             ip->fup = NULL;
             ip->store = 0;
             switch (OPFORMAT[iopcode]) {
             case FORMAT_R:
                ip->rd = ird; ip->rs1 = irs1; ip->rs2 = irs2; ip->rs3 = -1;   ip->imv = 0;
                break;
             case FORMAT_I:
                ip->rd = ird; ip->rs1 = irs1; ip->rs2 = -1;   ip->rs3 = irs2; ip->imv = 1;
                break;
             case FORMAT_J:
                ip->rd = -1;  ip->rs1 = -1;   ip->rs2 = -1;   ip->rs3 = ird;  ip->imv = 1;
                break;
             case FORMAT_B:
                ip->rd = -1;  ip->rs1 = ird;  ip->rs2 = irs1; ip->rs3 = irs2; ip->imv = 1;
                break;
             case FORMAT_I2:
                ip->rd = ird; ip->rs1 = irs1; ip->rs2 = -1;   ip->rs3 = irs2; ip->imv = 1;
                break;
             case FORMAT_S2:
    //            ip->rd = -1;  ip->rs1 = irs1; ip->rs2 = irs2; ip->rs3 = ird;  ip->imv = 2;
                ip->rd = -1;  ip->rs1 = ird; ip->rs2 = irs1; ip->rs3 = irs2;  ip->imv = 1;
    //            ip->rd = -1;  ip->rs1 = ird; ip->rs2 = irs1; ip->rs3 = irs2;  ip->imv = 2;
                ip->store = 1;
                break;
             case FORMAT_I3:
                ip->rd = ird; ip->rs1 = irs1; ip->rs2 = irs2; ip->rs3 = -1;   ip->imv = 0;
                break;
             case FORMAT_S3:
                ip->rd = -1;  ip->rs1 = irs1; ip->rs2 = irs2; ip->rs3 = ird;  ip->imv = 0;
                ip->store = 1;
                break;
             default:
                ip->rd = -1;  ip->rs1 = -1;   ip->rs2 = -1;   ip->rs3 = -1;   ip->imv = 0;
             }
             ip->optype = OPFUTYPE[iopcode];

             /* initialize instruction time fields */
             for (i = 0; i <= MAX_STAGE; ++i) {
                ip->t[i] = G.start_ck-1; /* -1 indicates not started */
             }
             sprintf(buf1, "%03d) %2d %2d %2d %2d", icount, iopcode, ird, irs1, irs2);
             sprintf(buf2, "%-4s", OPNAME[iopcode]);
             print_lregs_args(ip, buf3, "");
             Display("  %s --> %s %s", buf1, buf2, buf3);
             Queue__Insert((char *)ip, PI);
             ++icount;
         }
      }
   } else {
      Display("Cannot open '%s' for reading.", pn);
   }
   PI_n = Queue__Count(PI);
   Display("* TOTAL_ INSTRUCTIONS=%d", PI_n);
   Display("* NUMBER_OF_ITERATIONS=%d", G.iterations);
}

/*---------------------------------------------------------------------------*
 NAME      : print_RBEntry
 PURPOSE   : Pretty-printing of ROB status
 PARAMETERS:
 RETURN    :
 *---------------------------------------------------------------------------*/
void print_RBEntry(Instruction *ip, char *buf)
{
   char buf0[32];
   char buf1[32];
   char buf2[32];
   char buf3[32];

   sprintf(buf0, "x%-2d", RB[ip->robn].ri);
   sprintf(buf1, "P%-2d", RB[ip->robn].piold);
   if (RB[ip->robn].rbbusy == 0) sprintf(buf2, "----"); else sprintf(buf2, "%03d)", ip->robn);
   sprintf(buf3, "%2d", RB[ip->robn].cc);
//   sprintf(buf, "%-4s %03d %-3s %-3s %2d %2d %-2s", buf2, RB[ip->robn].PC, RB[ip->robn].ri != -1 ? buf0 : "-", RB[ip->robn].piold != -1 ? buf1 : "-", RB[ip->robn].st, RB[ip->robn].exc, RB[ip->robn].cplt == 0 ? "-" : buf3);
   sprintf(buf, "%-4s %03d %-3s %-3s %1d %1d %1d", buf2, RB[ip->robn].PC, RB[ip->robn].ri != -1 ? buf0 : "-", RB[ip->robn].piold != -1 ? buf1 : "-", RB[ip->robn].st, RB[ip->robn].exc, RB[ip->robn].cplt);
}

/*---------------------------------------------------------------------------*
 NAME      : fmt_preg
 PURPOSE   : Helper: Format operand as "Pxx" or "-"
 PARAMETERS:
 RETURN    :
 *---------------------------------------------------------------------------*/
const char* fmt_preg(int val, char *buf, size_t buflen) {
    if (val == -1)
        return "-";
    snprintf(buf, buflen, "P%-2d", val);
    return buf;
}

/*---------------------------------------------------------------------------*
 NAME      : fmt_idx
 PURPOSE   : Helper: Format index, e.g. "003)" or "003>"
 PARAMETERS:
 RETURN    :
 *---------------------------------------------------------------------------*/
void fmt_idx(int winn, char suffix, char *buf, size_t buflen) {
    snprintf(buf, buflen, "%03d%c", winn, suffix);
}

/*---------------------------------------------------------------------------*
 NAME      : fmt_cond_field
 PURPOSE   : Helper: Format condition field
 PARAMETERS:
 RETURN    :
 *---------------------------------------------------------------------------*/
const char* fmt_cond_field(int cond, const char *cond_buf) {
    if (cond == -1)
        return "-";
    else if (cond == -2)
        return ".";
    else
        return cond_buf;
}

/*---------------------------------------------------------------------------*
 NAME      : build_iwout
 PURPOSE   : Core formatting for IW or IWB
 PARAMETERS:
 RETURN    :
 *---------------------------------------------------------------------------*/
void build_iwout(char *out2, size_t outlen, int winn, char suffix,
	         const char *opcode, const WindowEntry *entry)
{
    char buf0[8], buf1[8], buf2[8], buf3[8], buf4[8], buf5[8];
    char buf_imm[8], buf6[8], buf7[8], buf8[8];

    // Format preg fields
    const char *op0 = fmt_preg(entry->pi, buf0, sizeof(buf0));
    const char *op1 = fmt_preg(entry->pj, buf1, sizeof(buf1));
    const char *op2 = fmt_preg(entry->pk, buf2, sizeof(buf2));
    snprintf(buf_imm, sizeof(buf_imm), "%-3d", entry->pl);
    const char *op3 = fmt_preg(entry->pl, buf3, sizeof(buf3));
    fmt_idx(winn, suffix, buf4, sizeof(buf4));

    // Immediate or register selection
    const char *op_imm_or_reg = (entry->imv == 1) ? buf_imm : (entry->pl != -1 ? op3 : "-");
    const char *op5;
    if (entry->imv == 2) {
        snprintf(buf5, sizeof(buf5), "%-3d", entry->pk);
        op5 = buf5;
    } else {
        op5 = (entry->pk != -1) ? op2 : "-";
    }

    // Conditions
    snprintf(buf6, sizeof(buf6), "%2d", entry->cj);
    snprintf(buf7, sizeof(buf7), "%2d", entry->ck);
    snprintf(buf8, sizeof(buf8), "%2d", entry->cl);

    const char *cond6 = fmt_cond_field(entry->cj, buf6);
    const char *cond7 = fmt_cond_field(entry->ck, buf7);
    const char *cond8 = fmt_cond_field(entry->cl, buf8);

    // Compose final output line safely
    snprintf(out2, outlen, "%4s %4s %-3s %-3s %-3s %-3s %2s %2s %2s",
             buf4, opcode,
             (entry->pi != -1) ? op0 : "-",
             (entry->pj != -1) ? op1 : "-",
             op5,
             op_imm_or_reg,
             cond6, cond7, cond8);
}

/*---------------------------------------------------------------------------*
 NAME      : dump_pipeline
 PURPOSE   : Pretty-printing of pipeline status
 PARAMETERS:
 RETURN    :
 *---------------------------------------------------------------------------*/
void dump_pipeline(int stage, int ic_ini, int ic_end)
{
   int p, n1, m1, k, n, st, ic, st_ini, st_end, tc, j, count, fa, first = 1;
   int row, regn, lat;
   Instruction *ip, *ip1;
   char out[SCREENWIDTH+2];
   char out1[SCREENWIDTH+2];
   #define MAXIWOUT (SCREENWIDTH+2)
   char out2[MAXIWOUT];
   char out3[SCREENWIDTH+2];
   char out4[SCREENWIDTH+2];
   char reg[SCREENWIDTH+2];
   char buf[32];
   char buf0[32];
   char buf1[32];
   char buf2[32];
   char buf3[32];
   char buf4[32];
   char buf5[32];
   char buf6[32];
   char buf7[32];
   char buf8[32];

   if (stage == 0) { st_ini = 1; st_end = MAX_STAGE; }
   else { st_ini = stage; st_end = stage; }

   //==============================================================
   strcpy(out1, ""); for (k = 1; k <= SCREENWIDTH; ++k) { strcat(out1, "="); } Display("%s", out1);

   strcpy(out, "PHYSICAL REGS:");
   size_t len = strlen(out);
   for (k = 1; k <= AA.pregs; ++k) {
      len += snprintf(out + len, sizeof(out) - len, " %2d", k);
   }
   Display("%s", out);
   if (CO != NULL) fprintf(CO, "%s\n", out);
//   sprintf(out1, "(%d/%d)", PRAllocated, AA.pregs);
//   sprintf(out, "%23s", out1);
   sprintf(out, "%14s", "");
   len = strlen(out);
   for (k = 1; k <= AA.pregs; ++k) {
     len += snprintf(out + len, sizeof(out) - len, " %2s", RM[k].busy ? "*" : " ");
   }
   Display("%s", out);
   if (CO != NULL) fprintf(CO, "%s\n", out);

   strcpy(out, "          qi: ");
   len = strlen(out);
   for (k = 1; k <= AA.pregs; ++k) {
      len += snprintf(out + len, sizeof(out) - len, " %2d", RM[k].qi);
   }
   Display("%s", out);
   if (CO != NULL) fprintf(CO, "%s\n", out);

   strcpy(out, "          vi: ");
   len = strlen(out);
   for (k = 1; k <= AA.pregs; ++k) {
      len += snprintf(out + len, sizeof(out) - len, " %02X", RM[k].vi % 256);
   }

   Display("%s", out);
   if (CO != NULL) fprintf(CO, "%s\n", out);

   //==============================================================
   strcpy(out1, ""); for (k = 1; k <= SCREENWIDTH; ++k) { strcat(out1, "="); } Display("%s", out1);

   strcpy(out, "REG.FILE: xi: ");
   len = strlen(out);
   for (k = 1; k <= AA.lregs; ++k) {
      len += snprintf(out + len, sizeof(out) - len, " %8d", k);
   }
   Display("%s", out);
   if (CO != NULL) fprintf(CO, "%s\n", out);

   strcpy(out, "          Pi: ");
   len = strlen(out);
   for (k = 1; k <= AA.lregs; ++k) {
      sprintf(buf, "%8d", RF[k].pn);
      len += snprintf(out + len, sizeof(out) - len, " %8s", RF[k].pn == -1 ? "-" : buf);
   }
   Display("%s", out);
   if (CO != NULL) fprintf(CO, "%s\n", out);

   strcpy(out, "          Qi: ");
   len = strlen(out);
   for (k = 1; k <= AA.lregs; ++k) {
      len += snprintf(out + len, sizeof(out) - len, " %8d", RF[k].qi);
   }
   Display("%s", out);
   if (CO != NULL) fprintf(CO, "%s\n", out);

   strcpy(out, "          Vi: ");
   len = strlen(out);
   for (k = 1; k <= AA.lregs; ++k) {
      len += snprintf(out + len, sizeof(out) - len, " %08X", RF[k].vi);
   }
   Display("%s", out);
   if (CO != NULL) fprintf(CO, "%s\n", out);

   //==============================================================
   /* Print header */
   strcpy(out1, ""); for (k = 1; k <= SCREENWIDTH; ++k) { strcat(out1, "="); } Display("%s", out1);

   strcpy(out, "STAGES:               ");
   // Stage acronyms
   len = strlen(out);
   for (st = st_ini; st <= st_end; ++st) {
      len += snprintf(out + len, sizeof(out) - len, "  %c", STAGE_ACR[st][0]);
   }
   // FUs acronyms
   strcpy(out1, "");
   len = strlen(out1);
   for (fa = 1; fa <= MAXFT; ++fa) {
      len += snprintf(out1 + len, sizeof(out1) - len, "%3s", FUNAME[fa]);
   }
   Display("%s RENAMED-STR    INSTRUCTION-WINDOW                   REORDER-BUFFER         %s", out, out1);
   if (CO != NULL) fprintf(CO, "%s RENAMED-STR    INSTRUCTION-WINDOW                REORDER-BUFFER         %s\n", out, out1);

   // TOTAL SLOTS
   strcpy(out, "TOTAL SLOTS:          ");
   len = strlen(out);
   for (st = st_ini; st <= st_end; ++st) {
      count = 0;
      len += snprintf(out + len, sizeof(out) - len, "%3d", STAGE_SIZ[st]);
   }

   strcpy(out1, "");
   len = 0;
   for (fa = 1; fa <= MAXFT; ++fa) {
      len += snprintf(out1 + len, sizeof(out1) - len, "%3d", FA[fa].tot);
   }
   Display("%s %-14d %-36d %-16d       %20s", out, AA.pregs, AA.win_size, AA.rob_size, out1);
   if (CO != NULL) fprintf(CO, "%s %-13d %-36d %-16d       %20s\n", out, AA.pregs, AA.win_size, AA.rob_size, out1);

   // BUSY SLOTS
   strcpy(out, "BUSY SLOTS:           ");
   len = strlen(out);
   for (st = st_ini; st <= st_end; ++st) {
      count = 0;
      for (j = 0; j < STAGE_SIZ[st]; ++j)
         if (STAGE_BUF[st][j].delay > 0)
            ++count;
      len += snprintf(out + len, sizeof(out) - len, "%3d", count);
   }

   strcpy(out1, "");
   len = 0;
   for (fa = 1; fa <= MAXFT; ++fa) {
      len += snprintf(out1 + len, sizeof(out1) - len, "%3d", FA[fa].busy2);
   }
   Display("%s %-14d %-36d %-16d       %20s", out, PRAllocated, IWAllocated, RBAllocated, out1);
   if (CO != NULL) fprintf(CO, "%s %-13d %-36d %-16d       %20s\n", out, PRAllocated, IWAllocated, RBAllocated, out1);

   // STALLS
   strcpy(out, "STALLS:               ");
   len = strlen(out);
   for (st = st_ini; st <= st_end; ++st) {
      len += snprintf(out + len, sizeof(out) - len, "%3d", Stall[st]);
   }

   strcpy(out1, "");
   len = 0;
   for (fa = 1; fa <= MAXFT; ++fa) {
      len += snprintf(out1 + len, sizeof(out1) - len, "%3d", FA[fa].stalls);
   }
   Display("%s %-14d %-36d %-16d       %20s", out, PRStalls, IWStalls, RBStalls, out1);
   if (CO != NULL) fprintf(CO, "%s %-13d %-36d %-16d       %20s\n", out, PRStalls, IWStalls, RBStalls, out1);

   n1 = LQElems;
   m1 = SQElems;

   //==============================================================
   strcpy(out1, ""); for (k = 1; k <= SCREENWIDTH; ++k) { strcat(out1, "="); } Display("%s", out1);

   Display("PC   INSTRUCTION        F  D  P  I  X  W  C Pi,Pj Pk Pl    IW#  OPCD Pi  Pj  Pk I/Pl  Cj Ck Cl  ROB# PC  xi  oPi s x c  +-------------------+");

   // First pass for LQ
   row = 0;
   sprintf(LSQCOLUMN[row++], "|LQ(%-2d)             |", LQElems);
   sprintf(LSQCOLUMN[row++], "|PC   OP Pi  EFAD Ci|");
   for (ic = ic_ini; ic <= ic_end; ++ic) {
      ip = (Instruction *)Queue__Read(ic, DYNSTREAM);
      if (ip->optype == L_FU && ip->inqueue > 0) {
         regn = ip->pd; lat = ip->cj; //lat = CK+AA.l_lat;
         sprintf(buf2, "%03d]", ip->CIC);
         sprintf(buf0, "P%-2d", regn);
         sprintf(buf1, "%2d", lat);
         sprintf(LSQCOLUMN[row++], "|%4s %2s %-3s %04X %2s|", ip->inqueue == 1 ? buf2 : "----", OPNAME[ip->opcode], (regn != -1) ? buf0 : "-", ip->efad, ip->cj == -2 ? "." : buf1);
      }
   }
   sprintf(LSQCOLUMN[row++], "+-------------------+");
//   sprintf(LSQCOLUMN[row++], "");
   LSQCOLUMN[row++][0] = '\0';

   // Second pass for SQ
   sprintf(LSQCOLUMN[row++], "+-------------------+");
   sprintf(LSQCOLUMN[row++], "|SQ(%-2d)             |", SQElems);
   sprintf(LSQCOLUMN[row++], "|PC   OP Pl  EFAD Cl|");
   for (ic = ic_ini; ic <= ic_end; ++ic) {
      ip = (Instruction *)Queue__Read(ic, DYNSTREAM);
      if (ip->optype == S_FU && ip->inqueue > 0) {
//         regn = ip->ps1; lat = ip->cj;
         regn = (ip->opcode == STORE2) ? ip->ps3 : ip->ps1;
         lat  = (ip->opcode == STORE2) ? ip->cl  : ip->cj;
         sprintf(buf2, "%03d]", ip->CIC);
         sprintf(buf0, "P%-2d", regn);
         sprintf(buf1, "%2d", lat);
         sprintf(buf2, "%s", ip->cl == -2 ? "." : buf1);
         sprintf(buf3, "%s", ip->cj == -2 ? "." : buf1);
         sprintf(LSQCOLUMN[row++], "|%4s %2s %-3s %04X %2s|", ip->inqueue == 1 ? buf2 : "----", OPNAME[ip->opcode], (regn != -1) ? buf0 : "-", ip->efad, (ip->opcode == STORE2) ? buf2 : buf3);
      }
   }
   sprintf(LSQCOLUMN[row++], "+-------------------+");

   // Third pass
   for (ic = ic_ini; ic <= ic_end; ++ic) {
      out[0] = '\0';
      ip = (Instruction *)Queue__Read(ic, DYNSTREAM);
      if (ip->opcode != NIN) {

         // printing CYCLE TABLE
         print_lregs_args(ip, reg, "");
         len = strlen(out);
         for (st = st_ini; st <= st_end; ++st) {
            tc = ip->t[st];
            if (tc >= G.start_ck) {
               len += snprintf(out + len, sizeof(out) - len, " %2d", tc);
            } else {
               len += snprintf(out + len, sizeof(out) - len, "   ");
            }
         }
         print2_pregs_args(ip, out1);

         /* IW */
         strcpy(out2, "");
	 if (ip->winn >= 0) {
            if (IW[ip->winn].busy) {
              build_iwout(out2, MAXIWOUT, ip->winn, ')', OPNAME[IW[ip->winn].opcode], &IW[ip->winn]);
            }
            if (IWB[ip->winn].busy) {
               build_iwout(out2, MAXIWOUT, ip->winn, '>', OPNAME[IWB[ip->winn].opcode], &IWB[ip->winn]);

               // Copy to ip->iws
               IWB[ip->winn].busy = 0;
               snprintf(ip->iws, 6, "---- ");
               safecat(ip->iws + 5, out2 + 5, MAXIWOUT - 7);
               ip->winn = -1;
            }
         } else {
           strncpy(out2, ip->iws, MAXIWOUT - 1);
           out2[MAXIWOUT - 1] = '\0';
         }

         /* ROB */
         strcpy(out3, "");
         if (ip->robn >= 0) {
            print_RBEntry(ip, out3);
         } else {
            strcpy(out3, ip->rbs);
         }

         /* LQ, SQ */
         strcpy(out4, LSQCOLUMN[ic]);

         Display("%03d] %-4s %-11s %-20s %-14s %-35s  %22s  %s", ic, OPNAME[ip->opcode], reg, out, out1, out2, out3, out4);
         if (CO != NULL) fprintf(CO, "%03d] %-4s %-11s %-20s %-14s %-35s  %22s  %s\n", ic, OPNAME[ip->opcode], reg, out, out1, out2, out3, out4);
      }
   }
}
