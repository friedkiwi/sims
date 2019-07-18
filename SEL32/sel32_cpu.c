/* sel32_cpu.c: Sel 32 CPU simulator

   Copyright (c) 2018, James C. Bevier
   Portions provided by Richard Cornwell and other SIMH contributers

   Permission is hereby granted, free of charge, to any person obtaining a
   copy of this software and associated documentation files (the "Software"),
   to deal in the Software without restriction, including without limitation
   the rights to use, copy, modify, merge, publish, distribute, sublicense,
   and/or sell copies of the Software, and to permit persons to whom the
   Software is furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in
   all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
   JAMES C. BEVIER BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
   IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
   CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/

#include "sel32_defs.h"

/* 32/7x PSW/PSD Mode Trap/Interrupt Priorities */
/* Relative Logical  Int Vect TCW  IOCD Description */
/* Priority Priority Location Addr Addr             */
/*   0                 0F4              Power Fail Safe Trap */
/*   1                 0FC              System Override Trap (Not Used) */
/*   2                 0E8*             Memory Parity Trap */
/*   3                 190              Nonpresent Memory Trap */
/*   4                 194              Undefined Instruction Trap */
/*   5                 198              Privilege Violation Trap */
/*   6                 180              Supervisor Call Trap (SVC) */
/*   7                 184              Machine Check Trap */
/*   8                 188              System Check Trap */
/*   9                 18C              Map Fault Trap */
/*   A                                  Not Used */
/*   B                                  Not Used */
/*   C                                  Not Used */
/*   D                                  Not Used */
/*   E                 0E4              Block Mode Timeout Trap */
/*   F                 1A4*             Arithmetic Exception Trap */
/*  10        00       0F0              Power Fail Safe Interrupt */
/*  11        01       0F8              System Override Interrupt */
/*  12        12       0E8*             Memory Parity Trap */
/*  13        13       0EC              Attention Interrupt */
/*  14        14       140    100  700  I/O Channel 0 interrupt */  
/*  15        15       144    104  708  I/O Channel 1 interrupt */  
/*  16        16       148    108  710  I/O Channel 2 interrupt */  
/*  17        17       14C    10C  718  I/O Channel 3 interrupt */  
/*  18        18       150    110  720  I/O Channel 4 interrupt */  
/*  19        19       154    114  728  I/O Channel 5 interrupt */  
/*  1A        1A       158    118  730  I/O Channel 6 interrupt */  
/*  1B        1B       15C    11C  738  I/O Channel 7 interrupt */  
/*  1C        1C       160    120  740  I/O Channel 8 interrupt */  
/*  1D        1D       164    124  748  I/O Channel 9 interrupt */  
/*  1E        1E       168    128  750  I/O Channel A interrupt */  
/*  1F        1F       16C    12C  758  I/O Channel B interrupt */  
/*  20        20       170    130  760  I/O Channel C interrupt */  
/*  21        21       174    134  768  I/O Channel D interrupt */  
/*  22        22       178    138  770  I/O Channel E interrupt */  
/*  23        23       17C    13C  778  I/O Channel F interrupt */  
/*  24        24       190*             Nonpresent Memory Trap */
/*  25        25       194*             Undefined Instruction Trap */
/*  26        26       198*             Privlege Violation Trap */
/*  27        27       19C              Call Monitor Interrupt */
/*  28        28       1A0              Real-Time Clock Interrupt */
/*  29        29       1A4*             Arithmetic Exception Interrupt */
/*  2A        2A       1A8              External/Software Interrupt */
/*  2B        2B       1AC              External/Software Interrupt */
/*  2C        2C       1B0              External/Software Interrupt */
/*  2D        2D       1B4              External/Software Interrupt */
/*  2E        2E       1B8              External/Software Interrupt */
/*  2F        2F       1BC              External/Software Interrupt */
/*  30        30       1C0              External/Software Interrupt */
/*  31        31       1C4              External/Software Interrupt */
/* THRU      THRU     THRU                        THRU              */ 
/*  77        77       2DC              External/Software Interrupt */
/*  78                 2E0              End of IPU Processing Trap (CPU) */
/*  79                 2E4              Start IPU Processing Trap (IPU) */
/*  7A                 2E8              Supervisor Call Trap (IPU) */
/*  7B                 2EC              Error Trap (IPU) */
/*  7C                 2F0              Call Monitor Trap (IPU) */
/*  7D        7D       2F4              Stop IPU Processing Trap (IPU) */
/*  7E        7E       2F8              External/Software Interrupt */
/*  7F        7F       2FC              External/Software Interrupt */

/* Concept 32 PSD Mode Trap/Interrupt Priorities */
/* Relative|Logical |Int Vect|TCW |IOCD|Description */
/* Priority|Priority|Location|Addr|Addr             */
/*   -                 080              Power Fail Safe Trap */
/*   -                 084              Power On Trap */
/*   -                 088              Memory Parity Trap */
/*   -                 08C              Nonpresent Memory Trap */
/*   -                 090              Undefined Instruction Trap */
/*   -                 094              Privilege Violation Trap */
/*   -                 098              Supervisor Call Trap (SVC) */
/*   -                 09C              Machine Check Trap */
/*   -                 0A0              System Check Trap */
/*   -                 0A4              Map Fault Trap */
/*   -                 0A8              Undefined IPU Instruction Trap */
/*   -                 0AC              Signal CPU or Signal IPU Trap */
/*   -                 0B0              Address Specification Trap */
/*   -                 0B4              Console Attention Trap */
/*   -                 0B8              Privlege Mode Halt Trap */
/*   -                 0BC              Arithmetic Exception Trap */
/*                                                                */
/*   0        00       100              External/software Interrupt 0 */
/*   1        01       104              External/software Interrupt 1 */
/*   2        02       108              External/software Interrupt 2 */
/*   3        03       10C              External/software Interrupt 3 */
/*   4        04       110    704  700  I/O Channel 0 interrupt */  
/*   5        05       114    70C  708  I/O Channel 1 interrupt */  
/*   6        06       118    714  710  I/O Channel 2 interrupt */  
/*   7        07       11C    71C  718  I/O Channel 3 interrupt */  
/*   8        08       120    724  720  I/O Channel 4 interrupt */  
/*   9        09       124    72C  728  I/O Channel 5 interrupt */  
/*   A        0A       128    734  730  I/O Channel 6 interrupt */  
/*   B        0B       12C    73C  738  I/O Channel 7 interrupt */  
/*   C        0C       130    744  740  I/O Channel 8 interrupt */  
/*   D        0D       134    74C  748  I/O Channel 9 interrupt */  
/*   E        0E       138    754  750  I/O Channel A interrupt */  
/*   F        0F       13C    75C  758  I/O Channel B interrupt */  
/*  10        10       140    764  760  I/O Channel C interrupt */  
/*  11        11       144    76C  768  I/O Channel D interrupt */  
/*  12        12       148    774  770  I/O Channel E interrupt */  
/*  13        13       14c    77C  778  I/O Channel F interrupt */  
/*  14        14       150              External/Software Interrupt */
/*  15        15       154              External/Software Interrupt */
/*  16        16       158              External/Software Interrupt */
/*  17        17       15C              External/Software Interrupt */
/*  18        18       160              Real-Time Clock Interrupt */
/*  19        19       164              External/Software Interrupt */
/*  1A        1A       1A8              External/Software Interrupt */
/*  1B        1B       1AC              External/Software Interrupt */
/*  1C        1C       1B0              External/Software Interrupt */
/* THRU      THRU     THRU                        THRU              */ 
/*  6C        6C       2B0              External/Software Interrupt */
/*  6D        6D       2B4              External/Software Interrupt */
/*  6E        6E       2B8              External/Software Interrupt */
/*  6F        6F       2BC              Interval Timer Interrupt */

/* IVL ------------> ICB   Trap/Interrupt Vector Location points to Interrupt Context Block */
/*                   Wd 0 - Old PSD Word 1  points to return location */
/*                   Wd 1 - Old PSD Word 2 */
/*                   Wd 2 - New PSD Word 1  points to first instruction of service routine */
/*                   Wd 3 - New PSD Word 2 */
/*                   Wd 4 - CPU Status word at time of interrupt/trap */
/*                   Wd 5 - N/U For Traps/Interrupts */

/* IVL ------------> ICB   XIO Interrupt Vector Location */
/*                   Wd 0 - Old PSD Word 1  points to return location */
/*                   Wd 1 - Old PSD Word 2 */
/*                   Wd 2 - New PSD Word 1  points to first instruction of service routine */
/*                   Wd 3 - New PSD Word 2 */
/*                   Wd 4 - Input/Output Command List Address (IOCL) for the Class F I/O CHannel */
/*                   Wd 5 - 24 bit real address of the channel status word */

/* CPU registers, map cache, spad, and other variables */
int traceme = 0;        /* dynamic trace function */
int trstart = 9981910;  /* count of when to start tracing */
//int trstart = 37; /* count of when to start tracing */
int             cpu_index;                  /* Current CPU running */
uint32          PSD[2];                     /* the PC for the instruction */
#define PSD1 PSD[0]                         /* word 1 of PSD */
#define PSD2 PSD[1]                         /* word 2 of PSD */
uint32          M[MAXMEMSIZE] = { 0 };      /* Memory */
uint32          GPR[8];                     /* General Purpose Registers */
uint32          BR[8];                      /* Base registers */
uint32          PC;                         /* Program counter */
uint32          CC;                         /* Condition codes, bits 1-4 of PSD1 */
uint32          SPAD[256];                  /* Scratch pad memory */
uint32          INTS[112];                  /* Interrupt status flags */
uint32          CPUSTATUS;                  /* cpu status word */
uint32          TRAPSTATUS;                 /* trap status word */
/* CPU mapping cache entries */
/* 32/55 has none */
/* 32/7x has 32 8KW maps per task */
/* Concept/32 has 2048 2KW maps per task */
uint32          MAPC[1024];                 /* maps are 16bit entries on word bountries */
uint32          HIWM = 0;                   /* max maps loaded so far */

uint32          modes;                      /* Operating modes, bits 0, 5, 6, 7 of PSD1 */
uint8           wait4int = 0;               /* waiting for interrupt if set */

/* define traps */
uint32          TRAPME = 0;                 /* trap to be executed */
uint32          attention_trap = 0;         /* set when trap is requested */

struct InstHistory
{
    uint32   psd1;      /* the PC for the instruction */
    uint32   psd2;      /* the PC for the instruction */
    uint32   inst;      /* the instruction itself */
    uint32   ea;        /* computed effective address of data */
    uint32   reg;       /* reg for operation */
    uint32   dest;      /* destination value */
    uint32   src;       /* source value */
    uint8    cc;        /* cc's */
};

/* forward definitions */
t_stat cpu_ex(t_value * vptr, t_addr addr, UNIT * uptr, int32 sw);
t_stat cpu_dep(t_value val, t_addr addr, UNIT * uptr, int32 sw);
t_stat cpu_reset(DEVICE * dptr);
t_stat cpu_set_size(UNIT * uptr, int32 val, CONST char *cptr, void *desc);
t_stat cpu_show_hist(FILE * st, UNIT * uptr, int32 val, CONST void *desc);
t_stat cpu_set_hist(UNIT * uptr, int32 val, CONST char *cptr, void *desc);
uint32 cpu_cmd(UNIT * uptr, uint16 cmd, uint16 dev);
t_stat cpu_help (FILE *st, DEVICE *dptr, UNIT *uptr, int32 flag, const char *cptr);
const char *cpu_description (DEVICE *dptr);
t_stat RealAddr(uint32 addr, uint32 *realaddr, uint32 *prot);
t_stat load_maps(uint32 thepsd[2]);
t_stat read_instruction(uint32 thepsd[2], uint32 *instr);
t_stat Mem_read(uint32 addr, uint32 *data);
t_stat Mem_write(uint32 addr, uint32 *data);

/* external definitions */
extern t_stat startxio(uint16 addr, uint32 *status);    /* XIO start in chan.c */
extern t_stat testxio(uint16 addr, uint32 *status);     /* XIO test in chan.c */
extern t_stat stopxio(uint16 addr, uint32 *status);     /* XIO stop in chan.c */
extern t_stat rschnlxio(uint16 addr, uint32 *status);   /* reset channel XIO */
extern t_stat haltxio(uint16 addr, uint32 *status); /* halt XIO */
extern t_stat grabxio(uint16 addr, uint32 *status); /* grab XIO n/u */
extern t_stat rsctlxio(uint16 addr, uint32 *status);    /* reset controller XIO */
extern t_stat chan_set_devs();              /* set up the defined devices on the simulator */
extern uint32 scan_chan(void);              /* go scan for I/O int pending */
extern uint16 loading;                      /* set when doing IPL */
extern int fprint_inst(FILE *of, uint32 val, int32 sw); /* instruction print function */
extern int irq_pend;                        /* go scan for pending interrupt */
extern void rtc_setup(int ss, uint32 level);    /* tell rtc to start/stop */
extern void itm_setup(int ss, uint32 level);    /* tell itm to start/stop */
extern int32 itm_rdwr(uint32 cmd, int32 cnt, uint32 level); /* read/write the interval timer */
extern int32 itm_srv(UNIT *uptr);           /* read/write the interval timer */
extern UNIT itm_unit;                       /* fake interval timer */

/* History information */
int32               hst_p = 0;       /* History pointer */
int32               hst_lnt = 0;     /* History length */
struct InstHistory *hst = NULL;      /* History stack */

/* CPU data structures

   cpu_dev      CPU device descriptor
   cpu_unit     CPU unit descriptor
   cpu_reg      CPU register list
   pu_mod      CPU modifiers list
*/

#ifdef DEFINED_IN_SIM_DEFS_H
/* Unit data structure from sim_defs.h

   Parts of the unit structure are device specific, that is, they are
   not referenced by the simulator control package and can be freely
   used by device simulators.  Fields starting with 'buf', and flags
   starting with 'UF', are device specific.  The definitions given here
   are for a typical sequential device.
*/

struct UNIT {
    UNIT                *next;                          /* next active */
    t_stat              (*action)(UNIT *up);            /* action routine */
    char                *filename;                      /* open file name */
    FILE                *fileref;                       /* file reference */
    void                *filebuf;                       /* memory buffer */
    uint32              hwmark;                         /* high water mark */
    int32               time;                           /* time out */
    uint32              flags;                          /* flags */
    uint32              dynflags;                       /* dynamic flags */
    t_addr              capac;                          /* capacity */
    t_addr              pos;                            /* file position */
    void                (*io_flush)(UNIT *up);          /* io flush routine */
    uint32              iostarttime;                    /* I/O start time */
    int32               buf;                            /* buffer */
    int32               wait;                           /* wait */
    int32               u3;                             /* device specific */
    int32               u4;                             /* device specific */
    int32               u5;                             /* device specific */
    int32               u6;                             /* device specific */
    void                *up7;                           /* device specific */
    void                *up8;                           /* device specific */
    uint16              us9;                            /* device specific */
    uint16              us10;                           /* device specific */
    void                *tmxr;                          /* TMXR linkage */
    t_bool              (*cancel)(UNIT *);
    double              usecs_remaining;                /* time balance for long delays */
    char                *uname;                         /* Unit name */
#ifdef SIM_ASYNCH_IO
    void                (*a_check_completion)(UNIT *);
    t_bool              (*a_is_active)(UNIT *);
    UNIT                *a_next;                        /* next asynch active */
    int32               a_event_time;
    ACTIVATE_API        a_activate_call;
    /* Asynchronous Polling control */
    /* These fields should only be referenced when holding the sim_tmxr_poll_lock */
    t_bool              a_polling_now;                  /* polling active flag */
    int32               a_poll_waiter_count;            /* count of polling threads */
                                                        /* waiting for this unit */
    /* Asynchronous Timer control */
    double              a_due_time;                     /* due time for timer event */
    double              a_due_gtime;                    /* due time (in instructions) for timer event */
    double              a_usec_delay;                   /* time delay for timer event */
#endif /* SIM_ASYNCH_IO */
    };

/* Unit flags */

#define UNIT_V_UF_31    12              /* dev spec, V3.1 */
#define UNIT_V_UF       16              /* device specific */
#define UNIT_V_RSV      31              /* reserved!! */
#define FEAT_TIMER      (1<<(UNIT_V_UF+12)) /* interval timer */
#define TMR_RTC         0

#define UNIT_ATTABLE    0000001         /* attachable */
#define UNIT_RO         0000002         /* read only */
#define UNIT_FIX        0000004         /* fixed capacity */
#define UNIT_SEQ        0000010         /* sequential */
#define UNIT_ATT        0000020         /* attached */
#define UNIT_BINK       0000040         /* K = power of 2 */
#define UNIT_BUFABLE    0000100         /* bufferable */
#define UNIT_MUSTBUF    0000200         /* must buffer */
#define UNIT_BUF        0000400         /* buffered */
#define UNIT_ROABLE     0001000         /* read only ok */
#define UNIT_DISABLE    0002000         /* disable-able */
#define UNIT_DIS        0004000         /* disabled */
#define UNIT_IDLE       0040000         /* idle eligible */
#endif /* DEFINED_IN_SIM_DEFS_H */

UNIT  cpu_unit =
    /* Unit data layout for CPU */
/*  { UDATA(rtc_srv, UNIT_BINK | MODEL(MODEL_27) | MEMAMOUNT(0), MAXMEMSIZE ), 120 }; */
    {
    NULL,               /* UNIT *next */             /* next active */
    NULL,               /* t_stat (*action) */       /* action routine */
    NULL,               /* char *filename */         /* open file name */
    NULL,               /* FILE *fileref */          /* file reference */
    NULL,               /* void *filebuf */          /* memory buffer */
    0,                  /* uint32 hwmark */          /* high water mark */
    0,                  /* int32 time */             /* time out */
    UNIT_BINK|MODEL(MODEL_27)|MEMAMOUNT(1),     /* uint32 flags */ /* flags */
    0,                  /* uint32 dynflags */        /* dynamic flags */
    MAXMEMSIZE,         /* t_addr capac */           /* capacity */
    0,                  /* t_addr pos */             /* file position */
    NULL,               /* void (*io_flush) */       /* io flush routine */
    0,                  /* uint32 iostarttime */     /* I/O start time */
    0,                  /* int32 buf */              /* buffer */
    80,                 /* int32 wait */             /* wait */
};

/* Register data structure definition from sim_defs.h */
#ifdef DEFINED_IN_SIM_DEFS_H
struct REG {
    CONST char          *name;                          /* name */
    void                *loc;                           /* location */
    uint32              radix;                          /* radix */
    uint32              width;                          /* width */
    uint32              offset;                         /* starting bit */
    uint32              depth;                          /* save depth */
    const char          *desc;                          /* description */
    BITFIELD            *fields;                        /* bit fields */
    uint32              qptr;                           /* circ q ptr */
    size_t              str_size;                       /* structure size */
    /* NOTE: Flags MUST always be last since it is initialized outside of macro definitions */
    uint32              flags;                          /* flags */
    };

/* Register flags  from sim_defs.h */
#define REG_FMT         00003                           /* see PV_x */
#define REG_RO          00004                           /* read only */
#define REG_HIDDEN      00010                           /* hidden */
#define REG_NZ          00020                           /* must be non-zero */
#define REG_UNIT        00040                           /* in unit struct */
#define REG_STRUCT      00100                           /* in structure array */
#define REG_CIRC        00200                           /* circular array */
#define REG_VMIO        00400                           /* use VM data print/parse */
#define REG_VMAD        01000                           /* use VM addr print/parse */
#define REG_FIT         02000                           /* fit access to size */
#define REG_HRO         (REG_RO | REG_HIDDEN)           /* hidden, read only */

#define REG_V_UF        16                              /* device specific */
#define REG_UFMASK      (~((1u << REG_V_UF) - 1))       /* user flags mask */
#define REG_VMFLAGS     (REG_VMIO | REG_UFMASK)         /* call VM routine if any of these are set */
#endif /* DEFINED_IN_SIM_DEFS_H */

//UNIT cpu_unit = { UDATA (&rtc_srv, UNIT_BINK, MAXMEMSIZE) };

REG                 cpu_reg[] = {
    {HRDATAD(PC, PC, 24, "Program Counter"), REG_FIT},
    {BRDATAD(PSD, PSD, 16, 32, 2, "Progtam Status Doubleword"), REG_FIT},
    {BRDATAD(GPR, GPR, 16, 32, 8, "Index registers"), REG_FIT},
    {BRDATAD(BR, BR, 16, 32, 8, "Base registers"), REG_FIT},
    {BRDATAD(SPAD, SPAD, 16, 32, 256, "CPU Scratchpad memory"), REG_FIT},
    {BRDATAD(MAPC, MAPC, 16, 32, 1024, "CPU mape cache"), REG_FIT},
    {HRDATAD(CPUSTATUS, CPUSTATUS, 32, "CPU Status Word"), REG_FIT},
    {HRDATAD(TRAPSTATUS, TRAPSTATUS, 32, "TRAP Status Word"), REG_FIT},
    {HRDATAD(CC, CC, 32, "Condition Codes"), REG_FIT},
    {BRDATAD(INTS, INTS, 16, 32, 112, "Interrupt Status"), REG_FIT},
//    {ORDATAD(BASE, baseaddr, 16, "Relocation base"), REG_FIT},
    {NULL}
};

/* Modifier table layout (MTAB) - only extended entries have disp, reg, or flags */
MTAB cpu_mod[] = {
    {
    /* MTAB table layout for cpu type */
    /* {UNIT_MODEL, MODEL(MODEL_55), "32/55", "32/55", NULL, NULL, NULL, "Concept 32/55"}, */
    UNIT_MODEL,          /* uint32 mask */            /* mask */
    MODEL(MODEL_55),     /* uint32 match */           /* match */
    "32/55",             /* cchar  *pstring */        /* print string */
    "32/55",             /* cchar  *mstring */        /* match string */
    NULL,                /* t_stat (*valid) */        /* validation routine */
    NULL,                /* t_stat (*disp)  */        /* display routine */
    NULL,                /* void *desc      */        /* value descriptor, REG* if MTAB_VAL, int* if not */
    "Concept 32/55",     /* cchar *help     */        /* help string */
    },
    {UNIT_MODEL, MODEL(MODEL_75), "32/75", "32/75", NULL, NULL, NULL, "Concept 32/75"},
    {UNIT_MODEL, MODEL(MODEL_27), "32/27", "32/27", NULL, NULL, NULL, "Concept 32/27"},
    {UNIT_MODEL, MODEL(MODEL_67), "32/67", "32/67", NULL, NULL, NULL, "Concept 32/67"},
    {UNIT_MODEL, MODEL(MODEL_87), "32/87", "32/87", NULL, NULL, NULL, "Concept 32/87"},
    {UNIT_MODEL, MODEL(MODEL_97), "32/97", "32/97", NULL, NULL, NULL, "Concept 32/97"},
    {UNIT_MODEL, MODEL(MODEL_V6), "V6", "V6", NULL, NULL, NULL, "Concept V6"},
    {UNIT_MODEL, MODEL(MODEL_V9), "V9", "V9", NULL, NULL, NULL, "Concept V9"},
    {
    /* MTAB table layout for cpu memory size */
    /* {UNIT_MSIZE, MEMAMOUNT(0), "128K", "128K", &cpu_set_size}, */
    UNIT_MSIZE,          /* uint32 mask */            /* mask */
    MEMAMOUNT(0),        /* uint32 match */           /* match */
    "128K",              /* cchar  *pstring */        /* print string */
    "128K",              /* cchar  *mstring */        /* match string */
    &cpu_set_size,       /* t_stat (*valid) */        /* validation routine */
    NULL,                /* t_stat (*disp)  */        /* display routine */
    NULL,                /* void *desc      */        /* value descriptor, REG* if MTAB_VAL, int* if not */
    NULL,                /* cchar *help     */        /* help string */
    },
    {UNIT_MSIZE, MEMAMOUNT(1), "256K", "256K", &cpu_set_size},
    {UNIT_MSIZE, MEMAMOUNT(2), "512K", "512K", &cpu_set_size},
    {UNIT_MSIZE, MEMAMOUNT(3),   "1M",   "1M", &cpu_set_size},
    {UNIT_MSIZE, MEMAMOUNT(4),   "2M",   "2M", &cpu_set_size},
    {UNIT_MSIZE, MEMAMOUNT(5),   "3M",   "3M", &cpu_set_size},
    {UNIT_MSIZE, MEMAMOUNT(6),   "4M",   "4M", &cpu_set_size},
    {UNIT_MSIZE, MEMAMOUNT(7),   "8M",   "8M", &cpu_set_size},
    {UNIT_MSIZE, MEMAMOUNT(8),  "16M",  "16M", &cpu_set_size},
    {MTAB_XTD | MTAB_VDV | MTAB_NMO | MTAB_SHP, 0, "HISTORY", "HISTORY",
     &cpu_set_hist, &cpu_show_hist},
    {0}
};

/* CPU device descriptor */
DEVICE cpu_dev = {
    /* "CPU", &cpu_unit, cpu_reg, cpu_mod,
    1, 8, 24, 1, 8, 32,
    &cpu_ex, &cpu_dep, &cpu_reset, NULL, NULL, NULL,
    NULL, DEV_DEBUG, 0, dev_debug,
    NULL, NULL, &cpu_help, NULL, NULL, &cpu_description */
    "CPU",               /* cchar *name */            /* device name */
    &cpu_unit,           /* UNIT *units */            /* unit array */
    cpu_reg,             /* REG *registers */         /* register array */
    cpu_mod,             /* MTAB *modifiers */        /* modifier array */
    1,                   /* uint32 numunits */        /* number of units */
    16,                  /* uint32 aradix */          /* address radix */
    24,                  /* uint32 awidth */          /* address width */
    2,                   /* uint32 aincr */           /* address increment */
    16,                  /* uint32 dradix */          /* data radix */
    32,                  /* uint32 dwidth */          /* data width */
    &cpu_ex,             /* t_stat (*examine) */      /* examine routine */
    &cpu_dep,            /* t_stat (*deposit) */      /* deposit routine */
    &cpu_reset,          /* t_stat (*reset) */        /* reset routine */
    NULL,                /* t_stat (*boot) */         /* boot routine */
    NULL,                /* t_stat (*attach) */       /* attach routine */
    NULL,                /* t_stat (*detach) */       /* detach routine */
    NULL,                /* void *ctxt */             /* (context) device information block pointer */
    DEV_DEBUG,           /* uint32 flags */           /* device flags */
    0,                   /* uint32 dctrl */           /* debug control flags */
    dev_debug,           /* DEBTAB *debflags */       /* debug flag name array */
    NULL,                /* t_stat (*msize) */        /* memory size change routine */
    NULL,                /* char *lname */            /* logical device name */
    &cpu_help,           /* t_stat (*help) */         /* help function */
    NULL,                /* t_stat (*attach_help) */  /* attach help function */
    NULL,                /* void *help_ctx */         /* Context available to help routines */
    &cpu_description,    /* cchar *(*description) */  /* Device description */
    NULL,                /* BRKTYPTB *brk_types */    /* Breakpoint types */
};

/* CPU Instruction decode flags */
#define INV     0x0000            /* Instruction is invalid */
#define HLF     0x0001            /* Half word instruction */
#define ADR     0x0002            /* Normal addressing mode */
#define IMM     0x0004            /* Immediate mode */
#define WRD     0x0008            /* Word addressing, no index */
#define SCC     0x0010            /* Sets CC */
#define RR      0x0020            /* Read source register */
#define R1      0x0040            /* Read destination register */
#define RB      0x0080            /* Read base register into dest */
#define SD      0x0100            /* Stores into destination register */
#define SDD     0x0200            /* Stores double into destination */
#define RM      0x0400            /* Reads memory */
#define SM      0x0800            /* Stores memory */
#define DBL     0x1000            /* Double word operation */
#define SB      0x2000            /* Store Base register */
#define BT      0x4000            /* Branch taken, no PC incr */

int nobase_mode[] = {
   /*    00            04             08             0C  */
   /*    00            ANR,           ORR,           EOR */ 
         HLF,        SCC|R1|RR|SD|HLF, SCC|R1|RR|SD|HLF, SCC|R1|RR|SD|HLF, 
   /*    10            14             18             1C */
   /*    CAR,          CMR,           SBR            ZBR */
         HLF,          HLF,           HLF,           HLF,
   /*    20            24             28             2C  */
   /*    ABR           TBR            REG            TRR  */
         HLF,          HLF,           HLF,           HLF, 
   /*    30            34             38             3C */
   /*    CALM          LA             ADR            SUR */
       HLF,            SD|ADR,        HLF,           HLF,
   /*    40            44             48             4C  */ 
   /*    MPR           DVR                             */
      SD|HLF,          HLF,           INV,           INV, 
   /*    50            54             58             5C */
   /*                                                 */
        INV,           INV,           INV,           INV,

   /*    60            64             68             6C   */
   /*    NOR           NORD           SCZ            SRA  */
      SD|HLF,          SDD|HLF,       HLF,           HLF, 

   /*    70            74             78             7C */
   /*    SRL           SRC            SRAD           SRLD */ 
      SD|HLF,          SD|HLF,        HLF,           HLF,

   /*    80            84             88             8C   */
   /*    LEAR          ANM            ORM            EOM  */
       SD|ADR,         SCC|SD|RR|RM|ADR,  SCC|SD|RR|RM|ADR,  SCC|SD|RR|RM|ADR,  

   /*    90            94             98             9C */
   /*    CAM           CMM            SBM            ZBM  */ 
      SCC|RR|RM|ADR,   RR|RM|ADR,     ADR,           ADR,

   /*    A0            A4             A8             AC  */
   /*    ABM           TBM            EXM            L    */
         ADR,          ADR,           ADR,        SCC|SD|RM|ADR,

   /*    B0            B4             B8             BC */
   /*    LM            LN             ADM            SUM  */ 
     SCC|SD|RM|ADR,    SCC|SD|RM|ADR,  SCC|SD|RR|RM|ADR,  SCC|SD|RR|RM|ADR,

   /*    C0            C4             C8             CC    */
   /*    MPM           DVM            IMM            LF  */
     SCC|SD|RM|ADR,    RM|ADR,        IMM,           ADR, 

   /*    D0            D4             D8             DC */
   /*    LEA           ST             STM            STF */ 
       SD|ADR,         RR|SM|ADR,     RR|SM|ADR,     ADR,  

   /*    E0            E4             E8             EC   */
   /*    ADF           MPF            ARM            BCT  */
     SCC|SD|RM|ADR,    SCC|RM|ADR,    SCC|SM|RR|RM|ADR,     ADR, 

   /*    F0            F4             F8             FC */
   /*    BCF           BI             MISC           IO */ 
        ADR,           RR|SD|WRD,     ADR,           IMM,  
};

int base_mode[] = {
   /* 00        04            08         0C      */
   /* 00        AND,          OR,        EOR  */
     HLF,      SCC|RR|SD|HLF,     SCC|RR|SD|HLF,     SCC|RR|SD|HLF,  
   /* 10        14           18        1C  */
   /* SACZ      CMR         xBR         SRx */
     HLF,       HLF,        HLF,        HLF,
   /* 20        24            28         2C   */
   /* SRxD      SRC          REG        TRR      */
    SD|HLF,   SD|HLF,         HLF,       HLF,    
   /* 30        34          38           3C */
   /*           LA          FLRop        SUR */
    INV,        INV,       SD|HLF,      SD|HLF,
   /* 40        44            48         4C     */
   /*                                        */
      INV,      INV,        INV,       INV,  

    /* 50       54          58            5C */
   /*  LA       BASE        BASE          CALLM */ 
    SD|ADR,     SM,ADR,     SB|RM|ADR,    ADR,

   /* 60        64            68         6C     */
   /*                                         */
      INV,      INV,         INV,      INV,   
   /* 70       74           78           7C */
   /*                                          */ 
    INV,     INV,        INV,        INV,  
   /* LEAR      ANM          ORM        EOM   */
   /* 80        84            88         8C   */
    SD|ADR,    SD|RM|ADR, SD|RM|ADR, SD|RM|ADR, 

   /* CAM       CMM           SBM        ZBM  */ 
   /* 90        94            98         9C   */
        RM|ADR, RM|ADR,       ADR,       ADR,

   /* A0        A4            A8         AC   */
   /* ABM       TBM           EXM        L    */
      ADR,      ADR,          ADR,    SD|RM|ADR,

   /* B0        B4            B8         BC   */
   /* LM        LN            ADM        SUM  */ 
      SD|RM|ADR,SD|RM|ADR,  SD|RM|ADR, SD|RM|ADR,

   /* C0        C4            C8         CC   */
   /* MPM       DVM           IMM        LF   */
    SD|RM|ADR,  RM|ADR,       IMM,       ADR, 

   /*  D0       D4            D8         DC */
   /*  LEA      ST            STM        STFBR */ 
    SD|ADR,     SM|ADR,       SM|ADR,    ADR,  
   /* E0        E4            E8         EC     */
   /* ADF       MPF           ARM        BCT      */
    SD|RM|ADR, RM|ADR,     SM|RM|ADR,    ADR, 
   /* F0        F4            F8         FC */
  /*  BCF       BI            MISC       IO */ 
      ADR,      RR|SB|WRD,    ADR,       IMM,  
};

/* set up the map registers for the current task in the cpu */
/* the PSD bpix and cpix are used to setup the maps */
/* return non-zero if mapping error */
t_stat load_maps(uint32 thepsd[2])
{
    uint32 num, sdc, spc;
    uint32 mpl, cpixmsdl, bpixmsdl, msdl, midl;
    uint32 cpix, bpix, i, j, map, osmidl;

    if (CPU_MODEL < MODEL_27)
    {
        /* 32/7x machine, 8KW maps */
        modes &= ~BASEBIT;                  /* no basemode on 7x */
        if ((thepsd[1] & 0xc0000000) == 0) {    /* mapped mode? */
            return ALLOK;                   /* no, all OK, no mapping required */
        }
        /* we are mapped, so load the maps for this task into the cpu map cache */
        cpix = (thepsd[1] >> 2) & 0xfff;    /* get cpix 12 bit offset from psd wd 2 */
        bpix = (thepsd[1] >> 18) & 0xfff;   /* get bpix 12 bit offset from psd wd 2 */
        num = 0;                            /* working map number */
        /* master process list is in 0x83 of spad for 7x */
        mpl = SPAD[0x83] >> 2;              /* get mpl from spad address */
        cpixmsdl = M[mpl + cpix];           /* get msdl from mpl for given cpix */
        /* if bit zero of mpl entry is set, use bpix first to load maps */
        if (cpixmsdl & BIT0)
        {
            /* load bpix maps first */
            bpixmsdl = M[mpl + bpix];           /* get bpix msdl word address */
            sdc = (bpixmsdl >> 24) & 0x3f;      /* get 6 bit segment description count */
            msdl = (bpixmsdl >> 2) & 0x3fffff;  /* get 24 bit real address of msdl */
            for (i = 0; i < sdc; i++)           /* loop through the msd's */
            {
                spc = (M[msdl + i] >> 24) & 0xff;       /* get segment page count from msdl */
                midl = (M[msdl + i] >> 2) && 0x3fffff;  /* get 24 bit real word address of midl */
                for (j = 0; j < spc; j++)
                {
                    /* load 16 bit map descriptors */
                    map = (M[midl + (j / 2)]);      /* get 2 16 bit map entries */
                    if (j & 1)
                        map = (map & RMASK);        /* use right half word map entry */
                    else
                        map = ((map >> 16) & RMASK);    /* use left half word map entry */
                    /* the map register contents is now in right 16 bits */
                    /* now load a 32 bit word with both maps from memory  */
                    /* and or in the new map entry data */
                    if (num & 1) {
                        /* entry going to rt hw, clean it first */
                        map = (MAPC[num/2] & LMASK) | map;  /* map is in rt hw */
                    }
                    else {
                        /* entry going to left hw, clean it first */
                        map = (MAPC[num/2] & RMASK) | (map << 16);  /* map is in left hw */
                    }
                    MAPC[num++/2] = map;        /* store the map reg contents into cache */
                    if (num >= 32)
                        return MAPFLT;          /* map loading overflow, map fault error */
                }
            }
        }
        /* now load cpix maps */
        cpixmsdl = M[mpl + cpix];               /* get cpix msdl word address */
        sdc = (cpixmsdl >> 24) & 0x3f;          /* get 6 bit segment description count */
        msdl = (cpixmsdl >> 2) & 0x3fffff;      /* get 24 bit real address of msdl */
        for (i = 0; i < sdc; i++)
        {
            spc = (M[msdl + i] >> 24) & 0xff;       /* get segment page count from msdl */
            midl = (M[msdl + i] >> 2) && 0x3fffff;  /* get 24 bit real word address of midl */
            for (j = 0; j < spc; j++)
            {
                /* load 16 bit map descriptors */
                map = (M[midl + (j / 2)]);      /* get 2 16 bit map entries */
                if (j & 1)
                    map = (map & RMASK);        /* use right half word map entry */
                else
                    map = ((map >> 16) & RMASK);   /* use left half word map entry */
                /* the map register contents is now in right 16 bits */
                /* now load a 32 bit word with both maps from memory  */
                /* and or in the new map entry data */
                if (num & 1) {
                    /* entry going to rt hw, clean it first */
                    map = (MAPC[num/2] & LMASK) | map;  /* map is in rt hw */
                }
                else {
                    /* entry going to left hw, clean it first */
                    map = (MAPC[num/2] & RMASK) | (map << 16);  /* map is in left hw */
                }
                MAPC[num++/2] = map;        /* store the map reg contents into cache */
                if (num >= 32)
                    return MAPFLT;          /* map loading overflow, map fault error */
            }
        }
        /* if none loaded, map fault */
        if (num == 0)
            return MAPFLT;              /* map fault error */
        if (num & 1) {                  /* clear rest of maps */
            /* left hw of map is good, zero right */
            map = (MAPC[num/2] & LMASK);    /* clean rt hw */
            MAPC[num++/2] = map;        /* store the map reg contents into cache */
        }
        /* num should be even at this point, so zero 32 bit word for remaining maps */
        if ((num/2) > HIWM)             /* largerst number of maps loaded so far* */
            HIWM = num/2;               /* yes, set new high watter mark */
//      for (i = num/2; i < 32/2; i++)  /* zero any remaining entries */
        for (i = num/2; i < HIWM; i++)  /* zero any remaining entries */
            MAPC[i] = 0;                /* clear the map entry to make not valid */
        return ALLOK;                   /* all cache is loaded, return OK */
    }
    else
    {
        char n[9];
        uint32 dqe;
        /* 32/27, 32/67, 32/87, 32/97 2KW maps */
        /* Concept/32 machine, 2KW maps */
        if ((modes & MAPMODE) == 0) {       /* mapped mode? */
//          fprintf(stderr, "not mapping PSD2 %x\r\n", PSD2);
            return ALLOK;               /* no, all OK, no mapping required */
        }
        /* we are mapped, so calculate real address from map information */
        cpix = (thepsd[1] >> 2) & 0xfff;    /* get word cpix 11 bit offset from psd wd 2 */
        num = 0;                        /* no maps loaded yet */
        /* master process list is in 0xf3 of spad for concept */
        mpl = SPAD[0xf3] >> 2;          /* get mpl from spad address */
        midl = M[mpl+cpix];             /* get mpl entry wd 0 for given cpix */
        msdl = M[mpl+cpix+1];           /* get mpl entry wd 0 for given cpix */
        dqe = M[0x8e8>>2];
        for (j=0; j<8; j++) {
            n[j] = (M[((dqe+0x18)>>2)+(j/4)] >> ((3-(j&7))*8)) & 0xff;
            if (n[j] == 0)
                n[j] = 0x20;
        }
        n[8] = 0;
//      fprintf(stderr, "mapping SPAD PSD2 %.8x PSD2 %x mpl %x osmidl %x umidl %x C.CURR %x LMN %s\r\n",
//          SPAD[0xf5], PSD2, mpl, M[mpl], midl, dqe, n);
        /* if bit zero of mpl entry is set, use msd entry 0 first to load maps */
        if (midl & BIT0)
        {
/* TODO do not load O/S if already loaded. Bit zero of O/S midl will be set by swapper */
            /* load msd 0 maps first (O/S) */
            osmidl = M[mpl];            /* get midl 0 word address */
            spc = osmidl & MASK16;      /* get 16 bit segment description count */
            if (osmidl & BIT0) {        /* see f O/S already loaded */
                num = spc;              /* set the number of o/s maps loaded */
                goto skipos;            /* skip load */
            }
            midl = M[mpl+1] & MASK24;   /* get 24 bit real address from mpl wd2 */
            midl = midl>>2;             /* make word address */
            for (j = 0; j < spc; j++)
            {
                /* load 16 bit map descriptors */
                map = (M[midl + (j / 2)]);      /* get 2 16 bit map entries */
                if (j & 1)
                    map = (map & RMASK);        /* use right half word map entry */
                else
                    map = ((map >> 16) & RMASK);   /* use left half word map entry */
                /* the map register contents is now in right 16 bits */
                /* now load a 32 bit word with both maps from memory  */
                /* and or in the new map entry data */
                if (num & 1) {
                    /* entry going to rt hw, clean it first */
                    map = (MAPC[num/2] & LMASK) | map;  /* map is in rt hw */
//                  fprintf(stderr, "mapping 0x%x O/S num 0x%x midl %x MAPC[%d] %x\r\n",
//                      spc, num-1, (midl+(j/2))<<2, num/2, map);
                }
                else {
                    /* entry going to left hw, clean it first */
                    map = (MAPC[num/2] & RMASK) | (map << 16);  /* map is in left hw */
                }
                MAPC[num/2] = map;      /* store the map reg contents into cache */
                if (++num >= 2048)
                    return MAPFLT;          /* map loading overflow, map fault error */
            }
#if 0
            if (num & 1)
                fprintf(stderr, "mapping 0x%x O/S num 0x%x midl %x MAPC[%d] %x\r\n",
                    spc, num-1, (midl+((spc-1)/2))<<2, num/2, map);
#endif
        }
skipos:
        /* now load cpix maps */
        midl = M[mpl+cpix];             /* get cpix midl word address */
        msdl = M[mpl+cpix+1];           /* get 24 bit real word address of midl */
        spc = midl & RMASK;             /* get segment page count from msdl */
        midl = M[mpl+cpix+1] & MASK24;  /* get 24 bit real word address of midl */
        midl = midl>>2;                 /* get word address of midl */
        for (j = 0; j < spc; j++)
        {
            /* load 16 bit map descriptors */
            map = M[midl + (j / 2)];    /* get 2 16 bit map entries */
            if (j & 1)
                map = (map & RMASK);    /* use right half word map entry */
            else
                map = ((map >> 16) & RMASK);    /* use left half word map entry */
            /* the map register contents is now in right 16 bits */
            /* now load a 32 bit word with both maps from memory  */
            /* and or in the new map entry data */
            if (num & 1) {
                /* entry going to rt hw, clean it first */
                map = (MAPC[num/2] & LMASK) | map;  /* map is in rt hw */
//              fprintf(stderr, "mapping 0x%x USER num 0x%x midl %x MAPC[%d] %x\r\n",
//                  spc, num-1, (midl+(j/2))<<2, num/2, map);
            }
            else {
                /* entry going to left hw, clean it first */
                map = (MAPC[num/2] & RMASK) | (map << 16);  /* map is in left hw */
            }
            MAPC[num/2] = map;              /* store the map reg contents into cache */
            if (++num >= 2048)
                return MAPFLT;              /* map loading overflow, map fault error */
        }
        /* if none loaded, map fault */
        /* we got here without map block found, return map fault error */
        if (num == 0)
            return MAPFLT;                  /* map fault error */
        if (num & 1) {
            /* left hw of map is good, zero right */
            map = (MAPC[num/2] & LMASK);    /* clean rt hw */
//      if (spc != 0)
//          fprintf(stderr, "mapping 0x%x USER num 0x%x midl %x MAPC[%d] %x\r\n",
//              spc, num-1, (midl+((spc-1)/2))<<2, num/2, map);
            MAPC[num++/2] = map;            /* store the map reg contents into cache */
        }
        /* num should be even at this point, so zero 32 bit words for remaining maps */
        if ((num/2) > HIWM)                 /* largerst number of maps loaded so far* */
            HIWM = num/2;                   /* yes, set new high watter mark */
//      for (i = num/2; i < 2048/2; i++)    /* zero any remaining entries */
        for (i = num/2; i < HIWM; i++)      /* zero any remaining entries */
            MAPC[i] = 0;                    /* clear the map entry to make not valid */
        return ALLOK;                       /* all cache is loaded, retun OK */
    }
}

/*
 * Return the real memory address from the logical address
 * Also return the protection status, 1 if write protected address
 */
t_stat RealAddr(uint32 addr, uint32 *realaddr, uint32 *prot)
{
    uint32 word, index, map, mask;

    *prot = 0;      /* show unprotected memory as default */
                    /* unmapped mode is unprotected */

    /* see what machine we have */
    if (CPU_MODEL < MODEL_27)
    {
        /* 32/7x machine with 8KW maps */
        if (modes & EXTDBIT)
            word = addr & 0xfffff;  /* get 20 bit logical word address */
        else
            word = addr & 0x7ffff;  /* get 19 bit logical word address */
        if ((modes & MAPMODE) == 0) {
            /* check if valid real address */
            if (word >= MEMSIZE)    /* see if address is within our memory */
                return NPMEM;       /* no, none present memory error */
            *realaddr = word;       /* return the real address */
            return ALLOK;           /* all OK, return instruction */
        }
        /* we are mapped, so calculate real address from map information */
        /* 32/7x machine, 8KW maps */
        index = word >> 15;         /* get 4 or 5 bit value */
        map = MAPC[index/2];        /* get two hw map entries */
        if (index & 1)
            /* entry is in rt hw, clear left hw */
            map &= RMASK;           /* map is in rt hw */
        else
            /* entry is in left hw, move to rt hw */
            map >>= 16;             /* map is in left hw */
        /* see if map is valid */
        if (map & 0x4000) {
            /* required map is valid, get 9 bit address and merge with 15 bit page offset */
            word = ((map & 0x1ff) << 15) | (word & 0x7fff);
            /* check if valid real address */
            if (word >= MEMSIZE)        /* see if address is within our memory */
                return NPMEM;           /* no, none present memory error */
            if ((modes & PRIVBIT) == 0) {   /* see if we are in unprivileged mode */
                if (map & 0x2000)       /* check if protect bit is set in map entry */
                    *prot = 1;          /* return memory write protection status */
            }
            *realaddr = word;           /* return the real address */
            return ALLOK;               /* all OK, return instruction */
        }
        /* map is invalid, so return map fault error */
        return MAPFLT;                  /* map fault error */
    }
    else {
        /* 32/27, 32/67, 32/87, 32/97 2KW maps */
        /* Concept 32 machine, 2KW maps */
        if (modes & (BASEBIT | EXTDBIT))
            word = addr & 0xffffff; /* get 24 bit address */
        else
            word = addr & 0x7ffff;  /* get 19 bit address */
        if ((modes & MAPMODE) == 0) {
            /* check if valid real address */
            if (word >= MEMSIZE)    /* see if address is within our memory */
                return NPMEM;       /* no, none present memory error */
            *realaddr = word;       /* return the real address */
//fprintf(stderr, "Read Real unmapped addr %x\r\n", addr, index, map);
            return ALLOK;           /* all OK, return instruction */
        }
        /* replace bits 8-18 with 11 bits from memory map register */
        /* we are mapped, so calculate real address from map information */
        index = (word >> 13) & 0x7ff;   /* get 11 bit value */
        map = MAPC[index/2];        /* get two hw map entries */
//fprintf(stderr, "read addr %x index %x map %x\r\n", addr, index, map);
        if (index & 1)
            /* entry is in rt hw, clear left hw */
            map &= RMASK;           /* map is in rt hw */
        else
            /* entry is in left hw, move to rt hw */
            map >>= 16;             /* map is in left hw */
//fprintf(stderr, "read 2 addr %x index %x map %x\r\n", word, index, map);
        if (map & 0x8000) {         /* see if map is valid */
            /* required map is valid, get 11 bit address and merge with 13 bit page offset */
            word = ((map & 0x7ff) << 13) | (word & 0x1fff);
            /* check if valid real address */
            if (word >= MEMSIZE)    /* see if address is within our memory */
                return NPMEM;       /* no, none present memory error */
            if ((modes & PRIVBIT) == 0) {   /* see if we are in unprivileged mode */
                mask = (word & 0x1800) >> 11;   /* get offset of 2kb block for map being addressed */
                if (map & (0x4000 >> mask))     /* check if protect bit is set in map entry */
                    *prot = 1;      /* return memory write protection status */
            }
//fprintf(stderr, "Read Real addr %x index %x map %x reala %x PSD2 %x\r\n", addr, index, map, word, SPAD[0xf5]);
            *realaddr = word;       /* return the real address */
            return ALLOK;           /* all OK, return instruction */
        }
        /* map is invalid, so return map fault error */
//      fprintf(stderr, "RealAddr MAP FAIL %x MAPC %x word %x addr %x index %x\n",
//          map, MAPC[index/2], word, addr, index);
        sim_debug(DEBUG_CMD, &cpu_dev, "RealAddr MAP FAIL %x MAPC %x word %x addr %x index %x\n",
            map, MAPC[index/2], word, addr, index);
        return MAPFLT;              /* map fault error */
    }
}

/* fetch the current instruction from the PC address */
t_stat read_instruction(uint32 thepsd[2], uint32 *instr)
{
    uint32 addr, status;

    if (CPU_MODEL < MODEL_27)
    {
        /* 32/7x machine with 8KW maps */
        /* instruction must be in first 512KB of address space */
        addr = thepsd[0] & 0x7fffc;     /* get 19 bit logical word address */
    }
    else
    {
        /* 32/27, 32/67, 32/87, 32/97 2KW maps */
        /* Concept 32 machine, 2KW maps */
        if (thepsd[0] & BASEBIT) {          /* bit 6 is base mode? */
            addr = thepsd[0] & 0xfffffc;    /* get 24 bit address */
        }
        else
            addr = thepsd[0] & 0x7fffc;     /* get 19 bit address */
    }
    status = Mem_read(addr, instr);         /* get the instruction at the specified address */
    sim_debug(DEBUG_DETAIL, &cpu_dev, "read_instr status = %x\n", status);
    return status;                          /* return ALLOK or ERROR */
}

/*
 * Read a full word from memory
 * Return error type if failure, ALLOK if
 * success.  Addr is logical address.
 */
t_stat Mem_read(uint32 addr, uint32 *data)
{
    uint32 status, realaddr, prot;

    status = RealAddr(addr, &realaddr, &prot);  /* convert address to real physical address */
    sim_debug(DEBUG_DETAIL, &cpu_dev, "Mem_read status = %x\n", status);
    if (status == ALLOK) {
        *data = M[realaddr >> 2];       /* valid address, get physical address contents */
        status = ALLOK;                 /* good status return */
        sim_debug(DEBUG_DETAIL, &cpu_dev, "Mem_read addr %.8x realaddr %.8x data %.8x prot %d\n",
            addr, realaddr, *data, prot);
    }
    return status;                      /* return ALLOK or ERROR */
}

/*
 * Write a full word to memory, checking protection
 * and alignment restrictions. Return 1 if failure, 0 if
 * success.  Addr is logical address, data is 32bit word
 */
t_stat Mem_write(uint32 addr, uint32 *data)
{
    uint32 status, realaddr, prot;

    status = RealAddr(addr, &realaddr, &prot);  /* convert address to real physical address */
    sim_debug(DEBUG_DETAIL, &cpu_dev, "Mem_write addr %.8x realaddr %.8x data %.8x prot %d\n",
        addr, realaddr, *data, prot);
    if (status == ALLOK) {
        if (prot)                   /* check for write protected memory */
            return MPVIOL;          /* return memory protection violation */
        M[realaddr >> 2] = *data;   /* valid address, put physical address contents */
        status = ALLOK;             /* good status return */
    }
    return status;                  /* return ALLOK or ERROR */
}

/* function to set the CCs in PSD1 */
/* ovr is setting for CC1 */
void set_CCs(uint32 value, int ovr)
{
    PSD1 &= 0x87FFFFFE;         /* clear the old CC's */
    if (ovr)
        CC = CC1BIT;            /* CC1 value */
    else
        CC = 0;                 /* CC1 off */
    if (value & FSIGN)
        CC |= CC3BIT;           /* CC3 for neg */
    else if (value == 0)
        CC |= CC4BIT;           /* CC4 for zero */
    else 
        CC |= CC2BIT;           /* CC2 for greater than zero */
    PSD1 |= (CC & 0x78000000);  /* update the CC's in the PSD */
}

/* Opcode definitions */
/* called from simulator */
t_stat
sim_instr(void)
{
    t_stat              reason = 0;       /* reason for stopping */
    t_uint64            dest;             /* Holds destination/source register */
    t_uint64            source;           /* Holds source or memory data */
    t_uint64            td;               /* Temporary */
    uint32              addr;             /* Holds address of last access */
    uint32              temp;             /* General holding place for stuff */
    uint32              IR;               /* Instruction register */
    uint32              i_flags;          /* Instruction description flags from table */
    uint32              t;                /* Temporary */
    uint32              temp2;            /* Temporary */
    uint32              bc;               /* Temporary bit count */
    uint16              opr;              /* Top half of Instruction register */
    uint16              OP;               /* Six bit instruction opcode */
    uint16              chan;             /* I/O channel address */
    uint16              lchan;            /* Logical I/O channel address */
    uint16              suba;             /* I/O subaddress */
    uint8               FC;               /* Current F&C bits */
    uint8               EXM_EXR=0;        /* PC Increment for EXM/EXR instructions */
    int                 reg;              /* GPR or Base register bits 6-8 */
    int                 sreg;             /* Source reg in from bits 9-11 reg-reg instructions */
    int                 ix;               /* index register */
    int                 dbl;              /* Double word */
    int                 ovr;              /* Overflow flag */
    int                 stopnext = 0;     /* Stop on next instruction */
    int                 skipinstr = 0;    /* Skip test for interrupt on this instruction */
    uint32              int_icb;          /* interrupt context block address */
    uint32              OIR;              /* Original Instruction register */
    uint32              OPSD1;            /* Original PSD1 */
    uint32              OPC;              /* Original PC */

wait_loop:
    while (reason == 0) {       /* loop until halted */

    // wait_loop:
        if (sim_interval <= 0) {            /* event queue? */
            reason = sim_process_event();   /* process */
            if (reason != SCPE_OK) {
                if (reason == SCPE_STEP)
                    stopnext = 1;
                else
                    break;      /* process */
            }
        }

        if (sim_brk_summ && sim_brk_test(PC, SWMASK('E'))) {
            reason = STOP_IBKPT;
            break;
        }

        /* call our fake interval timer to count down 64 instructions per count */
        itm_srv(&itm_unit);     /* service interval timer */
        sim_interval--;         /* count down */

        if (skipinstr) {        /* need to skip interrupt test? */
            skipinstr = 0;      /* skip only once */
            goto skipi;         /* skip int test */
        }

        /* process pending I/O interrupts */
        if (!loading && (wait4int || irq_pend)) {   /* see if ints are pending */
            int_icb = scan_chan();      /* no, go scan for I/O int pending */
            if (int_icb != 0) {         /* was ICB returned for an I/O or interrupt */
                int il;
                /* find interrupt level for icb address */
                for (il=0; il<112; il++) {
                    /* get the address of the interrupt IVL table in main memory */
                    uint32 civl = SPAD[0xf1] + (il<<2); /* contents of spad f1 points to chan ivl in mem */
                    civl = M[civl >> 2];            /* get the interrupt context block addr in memory */
                    if (civl == int_icb)
                        break;
                }
//sim_debug(DEBUG_EXP, &cpu_dev, "Normal int scan return icb %x\n", int_icb);
//fprintf(stderr, "Normal Interrupt scan return icb %x irq_pend %x wait4int %x\r\n", int_icb, irq_pend, wait4int);
                /* take interrupt, store the PSD, fetch new PSD */
                bc = PSD2 & 0x3ffc;             /* get copy of cpix */
                M[int_icb>>2] = PSD1&0xfffffffe;    /* store PSD 1 */
                M[(int_icb>>2)+1] = PSD2;       /* store PSD 2 */
                PSD1 = M[(int_icb>>2)+2];       /* get new PSD 1 */
                PSD2 = (M[(int_icb>>2)+3] & ~0x3ffc) | bc;  /* get new PSD 2 w/old cpix */
                /* I/O status DW address will be in WD 6 */
                /* set new map mode and interrupt blocking state in CPUSTATUS */
                modes = PSD1 & 0x87000000;      /* extract bits 0, 5, 6, 7 from PSD 1 */
                if (PSD2 & MAPBIT) {
                    CPUSTATUS |= 0x00800000;    /* set bit 8 of cpu status */
                    modes |= MAPMODE;           /* set mapped mode */
                } else
                    CPUSTATUS &= 0xff7fffff;    /* reset bit 8 of cpu status */
                if ((PSD2 & 0x8000) == 0) {     /* is it retain blocking state */
                    if (PSD2 & 0x4000)          /* no, is it set blocking state */
                        CPUSTATUS |= 0x80;      /* yes, set blk state in cpu status bit 24 */
                    else
                        CPUSTATUS &= ~0x80;     /* no, reset blk state in cpu status bit 24 */
                }
                PSD2 &= ~0x0000c000;            /* clear bit 48 & 49 to be unblocked */
                if (CPUSTATUS & 0x80)           /* see if old mode is blocked */
                    PSD2 |= 0x00004000;         /* set to blocked state */
                PSD2 &= ~RETMBIT;               /* turn off retain bit in PSD2 */
                SPAD[0xf5] = PSD2;              /* save the current PSD2 */
                sim_debug(DEBUG_INST, &cpu_dev,
                    "Interrupt %x OPSD1 %.8x OPSD2 %.8x NPSD1 %.8x NPSD2 %.8x ICBA %x\r\n",
                    il, M[int_icb>>2], M[(int_icb>>2)+1], PSD1, PSD2, int_icb);
                wait4int = 0;                   /* wait is over for int */
                skipinstr = 1;                  /* skip next interrupt test only once */
            }
            /* see if waiting at a wait instruction */
            if (wait4int || loading) {
// ????         sim_interval++;                 /* restore count */
                goto wait_loop;                 /* continue waiting */
            }
        } else {
            if (loading) {
                uint32 chsa  = scan_chan();     /* go scan for load complete pending */
                if (chsa != 0) {                /* see if a boot channel/subaddress were returned */
                    /* take interrupt, store the PSD, fetch new PSD */
                    PSD1 = 0x80000000;          /* get new PSD 1, priv and addr is 0 */
                    PSD2 = 0x00004000;          /* get new PSD 2, blocked */
                    modes = PSD1 & 0x87000000;  /* extract bits 0, 5, 6, 7 from PSD 1 */
                    sim_debug(DEBUG_INST, &cpu_dev, "Boot Loading PSD1 %.8x PSD2 %.8x\n", PSD1, PSD2);
                    /* set interrupt blocking state in CPUSTATUS */
                    CPUSTATUS |= 0x80;          /* set blocked state in cpu status, bit 24 too */
                    PSD2 &= ~RETMBIT;           /* turn off retain bit in PSD2 */
                    SPAD[0xf5] = PSD2;          /* save the current PSD2 */
                    loading = 0;                /* we are done loading */
                    skipinstr = 1;              /* skip next interrupt test only once */
                }
                goto wait_loop;                 /* continue waiting */
            }
            /* see if in wait instruction */
            if (wait4int)                       /* TRY this */
                goto wait_loop;                 /* continue waiting */
        }

        /* Check for external interrupt here */
        /* see if we have an attention request from console */
        if (!skipinstr && attention_trap) {
            TRAPME = attention_trap;    /* get trap number */
            attention_trap = 0;         /* do only once */
//          fprintf(stderr, "Attention TRAP %x\r\n", TRAPME);
            skipinstr = 1;              /* skip next interrupt test only once */
            goto newpsd;                /* got process trap */
        }

skipi:
        if (sim_brk_summ && sim_brk_test(PC, SWMASK('E'))) {
            reason = STOP_IBKPT;
            break;
        }

        /* fill IR from logical memory address */
        if ((TRAPME = read_instruction(PSD, &IR))) {
            sim_debug(DEBUG_INST, &cpu_dev, "read_instr TRAPME = %x\n", TRAPME);
            goto newpsd;                /* got process trap */
        }

        /* If executing right half */
        if (PSD1 & 2) 
            IR <<= 16;
exec:
/*FIXME temp saves */
        OIR = IR;
        OPSD1 = PSD1;
        OPC = PSD1 & 0xfffffe;          /* get 24 bit addr from PSD1 */

        /* TODO Update history for this instruction */
        if (hst_lnt)
        {
            hst_p += 1;                 /* next history location */
            if (hst_p >= hst_lnt)       /* check for wrap */
                hst_p = 0;              /* start over at beginning */
            hst[hst_p].psd1 = PSD1;     /* set execution address */ 
            hst[hst_p].psd2 = PSD2;     /* set mapping/blocking status */ 
        }

        /* Split instruction into pieces */
        PC = PSD1 & 0xfffffe;           /* get 24 bit addr from PSD1 */
        sim_debug(DEBUG_DATA, &cpu_dev, "-----Instr @ PC %x PSD1 %.8x PSD2 %.8x IR %.8x\n", PC, PSD1, PSD2, IR);
        opr = (IR >> 16) & MASK16;      /* use upper half of instruction */
        OP = (opr >> 8) & 0xFC;         /* Get opcode (bits 0-5) left justified */
        FC =  ((IR & F_BIT) ? 0x4 : 0) | (IR & 3);  /* get F & C bits for addressing */
        reg = (opr >> 7) & 0x7;         /* dest reg or xr on base mode */
        sreg = (opr >> 4) & 0x7;        /* src reg for reg-reg instructions or BR instr */
        dbl = 0;                        /* no doubleword instruction */
        ovr = 0;                        /* no overflow or arithmetic exception either */
        dest = (t_uint64)IR;            /* assume memory address specified */
        CC = PSD1 & 0x78000000;         /* save CC's if any */

        if (modes & BASEBIT) {
            i_flags = base_mode[OP>>2]; /* set the instruction processing flags */
            addr = IR & RMASK;          /* get address offset from instruction */
//          sim_debug(DEBUG_INST, &cpu_dev, "Base i_flags %x addr %.8x\n", i_flags, addr);
            switch(i_flags & 0xf) {
            case HLF:
                source = GPR[sreg];     /* get the src reg from instruction */
                break;
            case IMM:
                if (PC & 02) {              /* if pc is on HW boundry, bad address */
                    TRAPME = ADDRSPEC_TRAP; /* bad address, error */
                    goto newpsd;            /* go execute the trap now */
                }
                break;
            case ADR:
            case WRD:
                if (PC & 02) {              /* if pc is on HW boundry, bad address */
                    TRAPME = ADDRSPEC_TRAP; /* bad address, error */
                    goto newpsd;            /* go execute the trap now */
                }
                ix = (IR >> 20) & 7;        /* get index reg from instruction */
                if (ix != 0)
                    addr += GPR[ix];        /* if not zero, add in reg contents */
                ix = (IR >> 16) & 7;        /* get base reg from instruction */
                if (ix != 0)     
                    addr += BR[ix];         /* if not zero, add to base reg contents */
                FC = ((IR & F_BIT) ? 4 : 0);    /* get F bit from original instruction */
                FC |= addr & 3;             /* set new C bits to address from orig or regs */
                break;
            case INV:
                TRAPME = UNDEFINSTR_TRAP;   /* Undefined Instruction Trap */
                goto newpsd;                /* handle trap */
                break;
             }
        } else {
            i_flags = nobase_mode[OP>>2];   /* set the instruction processing flags */
            addr = IR & 0x7ffff;            /* get 19 bit address from instruction */

//          sim_debug(DEBUG_INST, &cpu_dev, "Non Based i_flags %x addr %.8x\n", i_flags, addr);
            /* non base mode instructions have bit 0 of the instruction set */
            /* for word length instructions and zero for halfword instructions */
            /* the LA (op=0x34) is the only exception.  So test for PC on a halfword */
            /* address and trap if word opcode is in right hw */
            if (PC & 02) {          /* if pc is on HW boundry, addr trap if bit zero set */
                if ((OP == 0x34) || (OP & 0x80)) {
                    TRAPME = ADDRSPEC_TRAP; /* bad address, error */
                    goto newpsd;            /* go execute the trap now */
                }
            }
            switch(i_flags & 0xf) {
            case HLF:           /* halfword instruction */
                source = GPR[sreg]; /* get the src reg contents */
                break;

            case IMM:           /* Immediate mode */
                if (PC & 02) {              /* if pc is on HW boundry, bad address */
                    TRAPME = ADDRSPEC_TRAP; /* bad address, error */
                    goto newpsd;            /* go execute the trap now */
                }
                break;

            case ADR:           /* Normal addressing mode */
                ix = (IR >> 21) & 3;        /* get the index reg if specified */
                if (ix != 0) {
                    addr += GPR[ix];        /* if not zero, add in reg contents */
                    FC = ((IR & F_BIT) ? 4 : 0);    /* get F bit from original instruction */
                    FC |= addr & 3;         /* set new C bits to address from orig or regs */
                }

                /* wart alert! */
                /* the lea instruction requires special handling for indirection. */
                /* Bit 0,1 are set to 1 of result addr if indirect bit is zero in */
                /* instruction.  Bits 0 & 1 are set to the last word */
                /* or instruction in the chain bits 0 & 1 if indirect bit set */
                  /* if IX == 00 => dest = IR */
                  /* if IX == 0x => dest = IR + reg */
                  /* if IX == Ix => dest = ind + reg */

                /* fall through */

            case WRD:           /* Word addressing, no index */
                bc = 0xC0000000;                /* set bits 0, 1 for instruction if not indirect */
                t = IR;                         /* get current IR */
                while (t & IND) {               /* process indirection */
                    if ((TRAPME = Mem_read(addr, &temp)))   /* get the word from memory */
                        goto newpsd;            /* memory read error or map fault */
                    bc = temp & 0xC0000000;     /* save new bits 0, 1 from indirect location */
                    CC = (temp & 0x78000000);   /* save CC's from the last indirect word */
                    if (modes & EXTDBIT) {      /* check if in extended mode */
                        /* extended mode, so location has 24 bit addr, not X,I ADDR */
//                      fprintf(stderr, "Indirect EXT before addr %x contents %x\n", addr, temp);
                        addr = temp & MASK24;   /* get 24 bit addr */
                        /* if no C bits set, use original, else new */
                        if ((IR & F_BIT) || (addr & 3)) 
                            FC = ((IR & F_BIT) ? 0x4 : 0) | (addr & 3);
//                      fprintf(stderr, "Indirect EXT after addr %x temp %x\n", addr, temp);
                        t &= ~IND;              /* turn off IND bit to stop while loop */
                    } else {
                        /* non-extended mode, process new X, I, ADDR fields */
//                      fprintf(stderr, "Indirect NE before addr %x contents %x\n", addr, temp);
                        addr = temp & MASK19;   /* get just the addr */
                        ix = (temp >> 21) & 3;  /* get the index reg from indirect word */
                        if (ix != 0)
                            addr += (GPR[ix] & MASK19); /* add the register to the address */
                        /* if no F or C bits set, use original, else new */
                        if ((temp & F_BIT) || (addr & 3)) 
                            FC = ((temp & F_BIT) ? 0x4 : 0) | (addr & 3);
//                      fprintf(stderr, "Indirect NE after addr %x temp %x\n", addr, temp);
                        t = temp;                   /* go process next indirect location */
                    }
                } 
                if (OP == 0xD0)             /* test for LEA op */
                    addr |= bc;             /* insert bits 0,1 values into address */
                dest = (t_uint64)addr;      /* make into 64 bit variable */
                break;
            case INV:                       /* Invalid instruction */
                    if ((TRAPME = Mem_read(addr, &temp)))   /* get the word from memory */
                        goto newpsd;            /* memory read error or map fault */
                TRAPME = UNDEFINSTR_TRAP;   /* Undefined Instruction Trap */
                goto newpsd;                /* handle trap */
                break;
            }
        }

        /* Read memory operand */
        if (i_flags & RM) { 
            if ((TRAPME = Mem_read(addr, &temp))) { /* get the word from memory */
                goto newpsd;            /* memory read error or map fault */
            }
            source = (t_uint64)temp;    /* make into 64 bit value */
            switch(FC) {
            case 0:                     /* word address, extend sign */
                source |= (source & MSIGN) ? D32LMASK : 0;
                break;
            case 1:                     /* left hw */
                source >>= 16;          /* move left hw to right hw*/
                /* Fall through */
            case 3:                     /* right hw or right shifted left hw */
                source &= 0xffff;       /* use just the right hw */
                if (source & 0x8000) {  /* check sign of 16 bit value */
                    /* sign extend the value to leftmost 48 bits */
                    source = 0xFFFF0000 | (source & 0xFFFF);    /* extend low 32 bits */
                    source |= (D32LMASK);   /* extend hi bits */
                }
                break;
            case 2:                     /* double word address */
                if ((addr & 2) != 2) {      /* must be double word adddress */
                    TRAPME = ADDRSPEC_TRAP; /* bad address, error */
                    goto newpsd;        /* go execute the trap now */
                }
                if ((TRAPME = Mem_read(addr+4, &temp))) {   /* get the 2nd word from memory */
                    goto newpsd;        /* memory read error or map fault */
                }
                source = (source << 32) | (t_uint64)temp;   /* merge in the low order 32 bits */
                dbl = 1;                /* double word instruction */
                break;
            case 4:         /* byte mode, byte 0 */
            case 5:         /* byte mode, byte 1 */
            case 6:         /* byte mode, byte 2 */
            case 7:         /* byte mode, byte 3 */
                source = (source >> (8*(7-FC))) & 0xff; /* right justify addressed byte */
                break;
           }
        }

        /* Read in if from register */
        if (i_flags & RR) {
            if (FC == 2 && (i_flags & HLF) == 0)    /* double dest? */
                dbl = 1;                    /* src must be dbl for dbl dest */
            dest = (t_uint64)GPR[reg];      /* get the register content */
            if (dbl) {                      /* is it double regs */
                if (reg & 1) {              /* check for odd reg load */
                    TRAPME = ADDRSPEC_TRAP; /* bad address, error */
                    goto newpsd;            /* go execute the trap now */
                }
                /* merge the regs into the 64bit value */
                dest = (((t_uint64)dest) << 32) | ((t_uint64)GPR[reg+1]);
            } else {
                /* sign extend the data value */
                dest |= (dest & MSIGN) ? D32LMASK : 0;
            }
        }

        /* For Base mode */
        if (i_flags & RB) {
            dest = (t_uint64)BR[reg];       /* get base reg contents */
        }

        /* For register instructions */
        if (i_flags & R1) {
            source = (t_uint64)GPR[sreg];
            if (dbl) {
                if (sreg & 1) {
                    TRAPME = ADDRSPEC_TRAP; /* bad address, error */
                    goto newpsd;            /* go execute the trap now */
                }
                /* merge the regs into the 64bit value */
                source = (source << 32) | ((t_uint64)GPR[reg+1]);
            } else {
                /* sign extend the data value */
                source |= (source & MSIGN) ? ((t_uint64)MASK32) << 32: 0;
            }
        }

        /* TODO Update other history information for this instruction */
        if(hst_lnt) {
            hst[hst_p].inst = IR;       /* save the instruction */
            hst[hst_p].dest = dest;     /* save the destination */
            hst[hst_p].src = source;    /* save the source */
            hst[hst_p].reg = reg;       /* save the src/dst reg */
        }

//      sim_debug(DEBUG_INST, &cpu_dev, "SW OP %x Non Based i_flags %x addr %.8x\n", OP, i_flags, addr);
        switch (OP>>2) {
/*
 *        For op-codes=00,04,08,0c,10,14,28,2c,38,3c,40,44,60,64,68
 */
        /* Reg - Reg instruction Format (16 bit) */
        /* |--------------------------------------| */
        /* |0 1 2 3 4 5|6 7 8 |9 10 11|12 13 14 15| */
        /* | Op Code   | DReg | SReg  | Aug Code  | */
        /* |--------------------------------------| */
        case 0x00>>2:               /* HLF - HLF */ /* CPU General operations */
                switch(opr & 0xF) {                 /* switch on aug code */
                case 0x0:   /* HALT */
                        if ((modes & PRIVBIT) == 0) {   /* must be privileged to halt */
                            TRAPME = PRIVVIOL_TRAP; /* set the trap to take */
                            goto newpsd;            /* Privlege violation trap */
                        }
                        if (CPUSTATUS & 0x00000100) {   /* Priv mode halt must be enabled */
                            TRAPME = PRIVHALT_TRAP; /* set the trap to take */
                            goto newpsd;            /* Privlege mode halt trap */
                        }
/*FIXME*/               reason = STOP_HALT;         /* do halt for now */
                        break;
                case 0x1:   /* WAIT */
                        if (modes & PRIVBIT == 0) { /* must be privileged to wait */
                            TRAPME = PRIVVIOL_TRAP; /* set the trap to take */
                            goto newpsd;            /* Privlege violation trap */
                        }
                        if (wait4int == 0) {
                            time_t result = time(NULL);
//                          fprintf(stderr, "Starting WAIT mode @%ju\r\n", (intmax_t)result);
                            sim_debug(DEBUG_CMD, &cpu_dev, "Starting WAIT mode %ju\n", (intmax_t)result);
                        }
                        wait4int = 1;               /* show we are waiting for interrupt */
                        i_flags |= BT;              /* keep PC from being incremented while waiting */
                        break;
                case 0x2:   /* NOP */
                        break;
                case 0x3:   /* LCS */
                        GPR[reg] = M[0x780 >> 2];   /* get console switches from memory loc 0x780 */
                        set_CCs(GPR[reg], 0);       /* set the CC's, CC1 = 0 */
                        break;
                case 0x4:   /* ES */
                        if (reg & 1) {          /* see if odd reg specified */
                            TRAPME = ADDRSPEC_TRAP; /* bad reg address, error */
                            goto newpsd;        /* go execute the trap now */
                        }
                        /* reg is reg to extend sign into from reg+1 */
                        GPR[reg] = (GPR[reg+1] & FSIGN) ? FMASK : 0;
                        set_CCs(GPR[reg], 0);   /* set CCs, CC2 & CC3 */
                        break;
                case 0x5:   /* RND */
                        if (reg & 1) {              /* see if odd reg specified */
                            TRAPME = ADDRSPEC_TRAP; /* bad reg address, error */
                            goto newpsd;            /* go execute the trap now */
                        }
                        temp = GPR[reg];            /* save the current contents of specified reg */
                        if (GPR[reg+1] & FSIGN) {   /* if sign of R+1 is set, incr R by 1 */
                            temp++;                 /* incr temp R value */
                            if (temp < GPR[reg])    /* if temp R less than R, we have overflow */
                                ovr = 1;            /* show we have overflow */
                            GPR[reg] = temp;        /* update the R value */
                        }
                        set_CCs(temp, ovr);         /* set the CC's, CC1 = ovr */
                        /* the arithmetic exception will be handled */
                        /* after instruction is completed */
                        /* check for arithmetic exception trap enabled */
                        if (ovr && (modes & AEXPBIT)) {
                            TRAPME = AEXPCEPT_TRAP; /* set the trap type */
                        }
                        break;
                case 0x6:   /* BEI */
                        if ((modes & PRIVBIT) == 0) {   /* must be privileged to BEI */
                            TRAPME = PRIVVIOL_TRAP; /* set the trap to take */
                            goto newpsd;            /* Privlege violation trap */
                        }
                        CPUSTATUS |= 0x80;          /* into status word bit 24 too */
                        PSD2 &= ~0x0000c000;        /* clear bit 48 & 49 */
                        PSD2 |= 0x00004000;         /* set bit 49 only */
                        SPAD[0xf5] = PSD2;          /* save the current PSD2 */
                        break;
                case 0x7:   /* UEI */
                        if ((modes & PRIVBIT) == 0) {   /* must be privileged to UEI */
                            TRAPME = PRIVVIOL_TRAP; /* set the trap to take */
                            goto newpsd;            /* Privlege violation trap */
                        }
                        CPUSTATUS &= ~0x80;         /* into status word bit 24 too */
                        PSD2 &= ~0x0000c000;        /* clear bit 48 & 49 to be unblocked */
                        SPAD[0xf5] = PSD2;          /* save the current PSD2 */
                        irq_pend = 1;               /* start scanning interrupts again */
                        break;
                case 0x8:   /* EAE */
                        PSD1 |= AEXPBIT;            /* set the enable AEXP flag in PSD */
                        CPUSTATUS |= AEXPBIT;       /* into status word too */
                        break;
                case 0x9:   /* RDSTS */
                        GPR[reg] = CPUSTATUS;       /* get CPU status word */
                        break;
                case 0xA:   /* SIPU */              /* ignore for now */
                        break;
                case 0xB:   /* INV */               /* RWCS ignore for now */
                case 0xC:   /* INV */               /* WWCS ignore for now */
                        break;
                case 0xD:   /* SEA */
                        modes |= EXTDBIT;           /* set new Extended flag in modes & PSD */
                        PSD1 |= EXTDBIT;            /* set the enable AEXP flag in PSD */
                        CPUSTATUS |= EXTDBIT;       /* into status word too */
                        break;
                case 0xE:   /* DAE */
                        modes &= AEXPBIT;           /* set new extended flag in modes & PSD */
                        PSD1 &= ~AEXPBIT;           /* disable AEXP flag in PSD */
                        CPUSTATUS &= ~AEXPBIT;      /* into status word too */
                        break;

                case 0xF:   /* CEA */
                        modes &= ~EXTDBIT;          /* disable extended mode in modes and PSD */
                        PSD1 &= ~EXTDBIT;           /* disable extended mode flag in PSD */
                        CPUSTATUS &= ~EXTDBIT;      /* into status word too */
                        break;
                }
                break;
        case 0x04>>2:               /* 0x04 SCC|RR|R1|SD|HLF - SD|HLF */ /* ANR, SMC, CMC, RPSWT */
                switch(opr & 0xF) {
                case 0x0:   /* ANR */
                    dest &= source;     /* just an and reg to reg */
                    i_flags |=  SCC;    /* make sure we set CC's for dest value */
                    break;
                case 0xA:   /* CMC */   /* Cache Memory Control - Diag use only */
                                        /* write reg to cache memory controller */
                    break;
                case 0x7:   /* SMC */   /* Shared Memory Control Not Used */
                                        /* write reg to shared memory controller */
                    break;
                case 0xB:   /* RPSWT */ /* Read Processor Status Word 2 (PSD2) */
                    dest = PSD2;        /* get PSD2 for user */
                    break;
                default:    /* INV */   /* everything else is invalid instruction */
                    TRAPME = UNDEFINSTR_TRAP;   /* Undefined Instruction Trap */
                    goto newpsd;        /* handle trap */
                    break;
                }
                break;

        case 0x08>>2:               /* 0x08 SCC|RR|R1|SD|HLF - */ /* ORR or ORRM */
                dest |= source;                 /* or the regs into dest reg */
                if (opr & 0x8)                  /* is this ORRM op? */
                     dest &= GPR[4];            /* mask with reg 4 contents */
                break;

        case 0x0C>>2:               /* 0x0c SCC|RR|R1|SD|HLF - SCC|SD|HLF */ /* EOR or EORM */
//fprintf(stderr, "@EOR dest %.8x source %.8x\r\n", (uint32)dest, (uint32)source);
                dest ^= source;                 /* exclusive or the regs into dest reg */
                if (opr & 0x8)                  /* is this EORM op? */
                     dest &= GPR[4];            /* mask with reg 4 contents */
//fprintf(stderr, "@EOR dest %.8x source %.8x\r\n", (uint32)dest, (uint32)source);
                break;

        case 0x10>>2:               /* 0x10 HLF - HLF */ /* CAR or (basemode SACZ ) */
                if (modes & BASEBIT) {          /* handle basemode SACZ instruction */
sacz:                                           /* non basemode SCZ enters here */
                    temp = GPR[reg];            /* get destination reg contents to shift */
                    CC = 0;                     /* zero the CC's */
                    t = 0;                      /* start with zero shift count */
                    if (temp == 0) {
                        CC = CC4BIT;            /* set CC4 showing dest is zero & cnt is zero too */
                    }
                    else
                    if (temp & BIT0) {
                        CC = 0;                 /* clear CC4 & set count to zero */
                    }
                    else
                    if (temp != 0) {            /* shift non zero values */
                        while ((temp & FSIGN) == 0) {   /* shift the reg until bit 0 is set */
                            temp <<= 1;         /* shift left 1 bit */
                            t++;                /* increment shift count */
                        }
                        temp <<= 1;             /* shift the sign bit out */
                    }
                    GPR[reg] = temp;            /* save the shifted values */
                    GPR[sreg] = t;              /* set the shift cnt into the src reg */
                    PSD1 &= 0x87FFFFFE;         /* clear the old CC's */
                    PSD1 |= (CC & 0x78000000);  /* update the CC's in the PSD */
                } else {
                    /* handle non basemode CAR instr */
                    temp = GPR[reg] - GPR[sreg];    /* subtract src from destination value */
                    set_CCs(temp, ovr);         /* set the CC's, CC1 = ovr */
                }
                break;

        case 0x14>>2:               /* 0x14 HLF - HLF */ /* CMR compare masked with reg */
    //FIXME CMD needs both regs to be masked with R4
                temp = GPR[reg] ^ GPR[sreg];    /* exclusive or src and destination values */
                temp &= GPR[4];                 /* and with mask reg (GPR 4) */
                CC = 0;                         /* set all CCs zero */
                if (temp == 0)                  /* if result is zero, set CC4 */
                    CC = CC4BIT;                /* set CC4 to show result 0 */
                PSD1 &= 0x87FFFFFE;             /* clear the old CC's */
                PSD1 |= (CC & 0x78000000);      /* update the CC's in the PSD */
                break;

        case 0x18>>2:               /* 0x18 SD|HLF - SD|HLF */ /* SBR, (basemode ZBR, ABR, TBR */
                if (modes & BASEBIT) {          /* handle basemode ZBR, ABR, TBR */
                    if ((opr & 0xC) == 0x0)     /* SBR instruction */
                        goto sbr;               /* use nonbase SBR code */
                    if ((opr & 0xC) == 0x4)     /* ZBR instruction */
                        goto zbr;               /* use nonbase ZBR code */
                    if ((opr & 0xC) == 0x8)     /* ABR instruction */
                        goto abr;               /* use nonbase ABR code */
                    if ((opr & 0xC) == 0xC)     /* TBR instruction */
                        goto tbr;               /* use nonbase TBR code */
inv:
                    TRAPME = UNDEFINSTR_TRAP;   /* Undefined Instruction Trap */
                    goto newpsd;                /* handle trap */

                } else {                        /* handle non basemode SBR */
sbr:                                            /* handle basemode too */
                    /* move the byte field bits 14-15 to bits 27-28 */
                    /* or in the bit# from dest reg field bits 6-8 into bit 29-31 */
                    bc = (((opr << 3) & 0x18) | reg);   /* get # bits to shift right */
                    bc = BIT0 >> bc;            /* make a bit mask of bit number */
                    t = (PSD1 & 0x70000000) >> 1;   /* get old CC bits 1-3 into CCs 2-4*/
//fprintf(stderr, "SBR1 bc %.8x PSD1 %.8x GPR[%d] %.8x\r\n", bc, PSD1, sreg, GPR[sreg]);
                    PSD1 &= 0x87FFFFFE;         /* clear the old CC's */
                    if (GPR[sreg] & bc)         /* test the bit in src reg */
                        t |= CC1BIT;            /* set CC1 to the bit value */
                    GPR[sreg] |=  bc;           /* set the bit in src reg */
                    PSD1 |= t;                  /* update the CC's in the PSD */
//fprintf(stderr, "SBR2 bc %.8x PSD1 %.8x GPR[%d] %.8x\r\n", bc, PSD1, sreg, GPR[sreg]);
                }
                break;

        case 0x1C>>2:               /* 0x1C SD|HLF - SD|HLF */ /* ZBR (basemode SRA, SRL, SLA, SLL) */
                if (modes & BASEBIT) {          /* handle basemode SRA, SRL, SLA, SLL */
                    if ((opr & 0x60) == 0x00)   /* SRA instruction */
                        goto sra;               /* use nonbase SRA code */
                    if ((opr & 0x60) == 0x20)   /* SRL instruction */
                        goto srl;               /* use nonbase SRL code */
                    if ((opr & 0x60) == 0x40)   /* SLA instruction */
                        goto sla;               /* use nonbase SLA code */
                    if ((opr & 0x60) == 0x60)   /* SLL instruction */
                        goto sll;               /* use nonbase SLL code */
                } else {                        /* handle nonbase ZBR */
zbr:                                            /* handle basemode too */
                    /* move the byte field bits 14-15 to bits 27-28 */
                    /* or in the bit# from dest reg field bits 6-8 into bit 29-31 */
                    bc = (((opr << 3) & 0x18) | reg);   /* get # bits to shift right */
                    bc = BIT0 >> bc;            /* make a bit mask of bit number */
                    t = (PSD1 & 0x70000000) >> 1;   /* get old CC bits 1-3 into CCs 2-4*/
//fprintf(stderr, "ZBR1 bc %.8x PSD1 %.8x GPR[%d] %.8x\r\n", bc, PSD1, sreg, GPR[sreg]);
                    PSD1 &= 0x87FFFFFE;         /* clear the old CC's */
                    if (GPR[sreg] & bc)         /* test the bit in src reg */
                        t |= CC1BIT;            /* set CC1 to the bit value */
                    GPR[sreg] &= ~bc;           /* reset the bit in src reg */
                    PSD1 |= t;                  /* update the CC's in the PSD */
//fprintf(stderr, "ZBR2 bc %.8x PSD1 %.8x GPR[%d] %.8x\r\n", bc, PSD1, sreg, GPR[sreg]);
                }
                break;

        case 0x20>>2:               /* 0x20 HLF - HLF */    /* ABR (basemode SRAD, SRLD, SLAD, SLLD) */
                if (modes & BASEBIT) {              /* handle basemode SRA, SRL, SLA, SLL */
                    if ((opr & 0x60) == 0x00)       /* SRAD instruction */
                        goto sra;                   /* use nonbase SRAD code */
                    if ((opr & 0x60) == 0x20)       /* SRLD instruction */
                        goto srl;                   /* use nonbase SRLD code */
                    if ((opr & 0x60) == 0x40)       /* SLAD instruction */
                        goto sla;                   /* use nonbase SLAD code */
                    if ((opr & 0x60) == 0x60)       /* SLLD instruction */
                        goto sll;                   /* use nonbase SLLD code */
                } else {                            /* handle nonbase mode ABR */
abr:                                                /* basemode ABR too */
                    /* move the byte field bits 14-15 to bits 27-28 */
                    /* or in the bit# from dest reg field bits 6-8 into bit 29-31 */
                    bc = (((opr << 3) & 0x18) | reg);   /* get # bits to shift right */
                    bc = BIT0 >> bc;                /* make a bit mask of bit number */
                    temp = GPR[sreg];               /* get reg value to add bit to */
//fprintf(stderr, "ABR bc %.8x addr %.8x PSD1 %.8x temp %.8x\r\n", bc, addr, PSD1, temp);
                    ovr = ((temp & FSIGN) != 0);    /* set ovr to status of sign bit 0 */
                    temp += bc;                     /* add the bit value to the reg */
                    ovr ^= ((temp & FSIGN) != 0);   /* set ovr if sign bit changed */
//fprintf(stderr, "ABR bc %.8x addr %.8x PSD1 %.8x temp %.8x ovr %d\r\n", bc, addr, PSD1, temp, ovr);
                    GPR[sreg] = temp;               /* save the new value */
                    set_CCs(temp, ovr);             /* set the CC's, CC1 = ovr */
                    /* the arithmetic exception will be handled */
                    /* after instruction is completed */
                    /* check for arithmetic exception trap enabled */
                    if (ovr && (modes & AEXPBIT)) {
                        TRAPME = AEXPCEPT_TRAP;     /* set the trap type */
                    }
                }
                break;

                case 0x24>>2:               /* 0x24 HLF - SD|HLF */ /* TBR (basemode SRC)  */
                if (modes & BASEBIT) {              /* handle SRC basemode */
                    if ((opr & 0x60) == 0x00)   /* SRC instruction */
                        goto src;               /* use nonbase code */
                    if ((opr & 0x60) == 0x40)   /* SLC instruction */
                        goto slc;               /* use nonbase code */
                    goto inv;                   /* else invalid */
                } else {                        /* handle TBR non basemode */
tbr:                                            /* handle basemode TBR too */
                    /* move the byte field bits 14-15 to bits 27-28 */
                    /* or in the bit# from dest reg field bits 6-8 into bit 29-31 */
                    bc = (((opr << 3) & 0x18) | reg);   /* get # bits to shift right */
                    bc = BIT0 >> bc;            /* make a bit mask of bit number */
                    t = (PSD1 & 0x70000000) >> 1;   /* get old CC bits 1-3 into CCs 2-4*/
//fprintf(stderr, "TBR1 bc %.8x PSD1 %.8x GPR[%d] %.8x\r\n", bc, PSD1, sreg, GPR[sreg]);
                    PSD1 &= 0x87FFFFFE;         /* clear the old CC's */
                    if (GPR[sreg] & bc)         /* test the bit in src reg */
                        t |= CC1BIT;            /* set CC1 to the bit value */
                    PSD1 |= t;                  /* update the CC's in the PSD */
//fprintf(stderr, "TBR2 bc %.8x PSD1 %.8x GPR[%d] %.8x\r\n", bc, PSD1, sreg, GPR[sreg]);
                }
                break;

        case 0x28>>2:               /* 0x28 HLF - HLF */ /* Misc OP REG instructions */
                temp = GPR[reg];            /* get reg value */
                switch(opr & 0xF) {

                case 0x0:       /* TRSW */
                    if (modes & BASEBIT)
                        addr = temp & MASK24;       /* 24 bits for based mode */
                    else
                        addr = temp & 0x7FFFE;      /* 19 bits for non based mode */
                    /* we are returning to the addr in reg, set CC's from reg */
                    /* update the PSD with new address */
                    PSD1 = (PSD1 & 0xff000000) | (addr & 0xfffffe); /* set new PC */
                    PSD1 = ((PSD1 & 0x87ffffff) | (temp & 0x78000000)); /* insert CCs from reg */
                    i_flags |= BT;                  /* we branched, so no PC update */
                    break;

                case 0x2:           /* XCBR */  /* Exchange base registers */
                    if ((modes & BASEBIT) == 0) /* see if nonbased */
                        goto inv;               /* invalid instruction in nonbased mode */
                    temp = BR[reg];             /* get dest reg value */
                    BR[reg] = BR[sreg];         /* put source reg value int dest reg */
                    BR[sreg] = temp;            /* put dest reg value into src reg */
                    break;

                case 0x4:           /* TCCR */  /* Transfer condition codes to GPR */
                    if ((modes & BASEBIT) == 0) /* see if nonbased */
                        goto inv;               /* invalid instruction in nonbased mode */
                    temp = CC >> 27;            /* right justify CC's in reg */
                    break;

                case 0x5:           /* TRCC */  /* Transfer GPR to condition codes */
                    if ((modes & BASEBIT) == 0) /* see if nonbased */
                        goto inv;               /* invalid instruction in nonbased mode */
                    PSD1 = ((PSD1 & 0x87fffffe) | (GPR[reg] << 27));    /* insert CCs from reg */
                    break;

                case 0x8:           /* BSUB */  /* Procedure call */
                    if ((modes & BASEBIT) == 0) /* see if nonbased */
                        goto inv;               /* invalid instruction in nonbased mode */
                    /* if Rd field is 0 (reg is b6-8), this is a BSUB instruction */
                    /* otherwise it is a CALL instruction (Rd != 0) */
                    if (reg == 0) {
                        /* BSUB instruction */
                        uint32 fp = BR[2];              /* get dword bounded frame pointer from BR2 */
                        if ((BR[2] & 0x7) != 0)  {
                            /* Fault, must be dw bounded address */
                            TRAPME = ADDRSPEC_TRAP;     /* bad address, error */
                            goto newpsd;                /* go execute the trap now */
                        }
                        M[fp>>2] = (PSD1 + 2) & 0x01fffffe; /* save AEXP bit and PC into frame */
                        M[(fp>>2)+1] = 0x80000000;          /* show frame created by BSUB instr */
                        BR[3] = GPR[0];             /* GPR 0 to BR 3 (AP) */
                        BR[0] = BR[2];              /* set frame pointer from BR 2 into BR 0 */
                        BR[1] = BR[sreg];           /* Rs reg to BR 1 */
                        PSD1 = (PSD1 & 0xff000000) | (BR[sreg] & 0xffffff); /* New PSD address */
                    } else {
                        /* CALL instruction */
                        /* get frame pointer from BR2 - 16 words & make it a dword addr */
                        uint32 cfp = ((BR[2]-0x40) & 0xfffffff8);
                        M[cfp>>2] = (PSD1 + 2) & 0x01fffffe;    /* save AEXP bit and PC from PSD1 in to frame */
                        M[(cfp>>2)+1] = 0x00000000;         /* show frame created by CALL instr */
                        for (ix=0; ix<8; ix++)
                            M[(cfp>>2)+ix+2] = BR[ix];      /* save BRs 0-7 to call frame */
                        for (ix=2; ix<8; ix++)
                            M[(cfp>>2)+ix+10] = GPR[ix];    /* save GPRs 2-7 to call frame */
                        BR[3] = GPR[reg];           /* Rd to BR 3 (AP) */
                        BR[0] = cfp;                /* set current frame pointer into BR[0] */
                        BR[2] = cfp;                /* set current frame pointer into BR[2] */
                        BR[1] = BR[sreg];           /* Rs reg to BR 1 */
                        PSD1 = (PSD1 & 0xff000000) | (BR[sreg] & 0xffffff); /* New PSD address */
                    }
                    break;

                case 0xC:           /* TPCBR */ /* Transfer program Counter to Base Register */
                    if ((modes & BASEBIT) == 0) /* see if nonbased */
                        goto inv;               /* invalid instruction in nonbased mode */
                    BR[reg] = PSD1 & 0xfffffe;  /* save PC from PSD1 into BR */
                    break;

                case 0x1:           /* INV */
                case 0x3:           /* INV */
                case 0x6:           /* INV */
                case 0x7:           /* INV */
                case 0x9:           /* INV */
                case 0xA:           /* INV */
                case 0xB:           /* INV */
                case 0xD:           /* INV */
                case 0xE:           /* INV */
                case 0xF:           /* INV */
                    break;
                }
                break;

            case 0x2C>>2:               /* 0x2C HLF - HLF */    /* Reg-Reg instructions */
                temp = GPR[reg];                    /* reg contents specified by Rd */
                addr = GPR[sreg];                   /* reg contents specified by Rs */
                bc = 0;
//fprintf(stderr, "@0x2c temp %.8x addr %.8x\r\n", temp, addr);

                switch(opr & 0xF) {
                case 0x0:           /* TRR */       /* SCC|SD|R1 */
                    temp = addr;                    /* set value to go to GPR[reg] */
                    bc = 1;                         /* set CC's at end */
                    break;

                case 0x1:           /* TRBR */      /* Transfer GPR to BR  */
                    if ((modes & BASEBIT) == 0)     /* see if nonbased */
                        goto inv;                   /* invalid instruction in nonbased mode */
                    BR[reg] = GPR[sreg];            /* copy GPR to BR */
                    break;

                case 0x2:           /* TBRR */      /* transfer BR to GPR */
                    if ((modes & BASEBIT) == 0)     /* see if nonbased */
                        goto inv;                   /* invalid instruction in nonbased mode */
                    temp = BR[sreg];                /* set base reg value */
                    bc = 1;                         /* set CC's at end */
                    break;

                case 0x3:           /* TRC */       /* Transfer register complement */
                    temp = addr ^ FMASK;            /* complement Rs */
                    bc = 1;                         /* set CC's at end */
                    break;

                case 0x4:           /* TRN */       /* Transfer register negative */
                    temp = -addr;                   /* negate Rs value */
                    if (temp == addr)               /* overflow if nothing changed */
                        ovr = 1;                    /* set overflow flag */
                    bc = 1;                         /* set the CC's */
                    break;

                case 0x5:           /* XCR */       /* exchange registers Rd & Rs */
                    GPR[sreg] = temp;               /* Rd to Rs */
                    set_CCs(temp, ovr);             /* set the CC's from original Rd */
                    temp = addr;                    /* save the Rs value to Rd reg */
                    break;

                case 0x6:           /* INV */
                    goto inv;
                    break;

                case 0x7:           /* LMAP */      /* Load map reg - Diags only */
                    goto inv;
                    break;

                case 0x8:           /* TRRM */      /* SCC|SD|R1 */
                    temp = addr & GPR[4];           /* transfer reg-reg masked */
                    bc = 1;                         /* set CC's at end */
                    break;

                /* CPUSTATUS bits */
                /* Bits 0-19 reserved */
                /* Bit 20   =0 Write to writable control store is disabled */
                /*          =1 Write to writable control store is enabled */
                /* Bit 21   =0 Enable PROM mode */
                /*          =1 Enable Alterable Control Store Mode */
                /* Bit 22   =0 Enable High Speed Floating Point Accelerator */
                /*          =1 Disable High Speed Floating Point Accelerator */
                /* Bit 23   =0 Disable privileged mode halt trap */
                /*          =1 Enable privileged mode halt trap */
                /* Bit 24 is reserved */
                /* bit 25   =0 Disable software trap handling (enable automatic trap handling) */
                /*          =1 Enable software trap handling */
                /* Bits 26-31 reserved */
                case 0x9:           /* SETCPU */
                    CPUSTATUS &= 0xfffff0bf;        /* zero bits that can change */
                    CPUSTATUS |= (temp & 0x0f40);   /* or in the new status bits */
                    break;

                case 0xA:           /* TMAPR */     /* Transfer map to Reg - Diags only */
                    goto inv;                       /* not used */
                    break;

                case 0xB:           /* TRCM */      /* Transfer register complemented masked */
                    temp = (addr ^ FMASK) & GPR[4]; /* compliment & mask */
                    bc = 1;                         /* set the CC's */
                    break;

                case 0xC:           /* TRNM */      /* Transfer register negative masked */
                    temp = -addr;                   /* complement GPR[reg] */
                    if (temp == addr)               /* check for overflow */
                        ovr = 1;                    /* overflow */
                    temp &= GPR[4];                 /* and with negative reg */
                    bc = 1;                         /* set the CC's */
                    break;

                case 0xD:           /* XCRM */      /* Exchange registers masked */
                    addr &= GPR[4];                 /* and Rs with mask reg */
                    temp &= GPR[4];                 /* and Rd with mask reg */
                    GPR[sreg] = temp;               /* Rs to get Rd masked value */
                    set_CCs(temp, ovr);             /* set the CC's from original Rd */
                    temp = addr;                    /* save the Rs value to Rd reg */
                    break;

                case 0xE:           /* TRSC */      /* transfer reg to SPAD */
                    t = (GPR[reg] >> 16) & 0xff;    /* get SPAD address from Rd (6-8) */
                    temp2 = SPAD[t];                /* get old SPAD data */
                    SPAD[t] = GPR[sreg];            /* store Rs into SPAD */
//fprintf(stderr, "TRSC SPAD[%x] old %.8x new %.8x INTS[%x] %.8x\r\n", t, temp2, SPAD[t], t, INTS[t]);
                    break;

                case 0xF:           /* TSCR */      /* Transfer scratchpad to register */
                    t = (GPR[sreg] >> 16) & 0xff;   /* get SPAD address from Rs (9-11) */
                    temp = SPAD[t];                 /* get SPAD data into Rd (6-8) */
//fprintf(stderr, "TSCR SPAD[%x] %.8x\r\n", t, temp);
                    break;
                }
                GPR[reg] = temp;                    /* save the temp value to Rd reg */
                if (bc)                             /* set cc's if bc set */
                    set_CCs(temp, ovr);             /* set the CC's */
                /* the arithmetic exception will be handled */
                /* after instruction is completed */
                /* check for arithmetic exception trap enabled */
                if (ovr && (modes & AEXPBIT)) {
                    TRAPME = AEXPCEPT_TRAP;         /* set the trap type */
                }
                break;

/*TODO*/    case 0x30>>2:               /* 0x30 */ /* CALM */
                fprintf(stderr, "ERROR - CALM called\r\n");
                goto inv;           /* TODO */
                break;

            case 0x34>>2:               /* 0x34 SD|ADR - inv */ /* LA non-basemode */
                if (modes & BASEBIT)            /* see if based */
                    goto inv;                   /* invalid instruction in based mode */
                if (modes & EXTDBIT) {          /* see if extended mode */
                    dest = (t_uint64)addr;      /* just pure 24 bit address */
                } else {                        /* use bits 13-31 */
                    dest = (t_uint64)(addr | ((FC & 4) << 17));     /* F bit to bit 12 */
                }
                break;

        case 0x38>>2:                   /* 0x38 SCC|SD|HLF - SD|HLF */   /* REG - REG floating point instructions */
                temp = GPR[reg];                    /* reg contents specified by Rd */
                addr = GPR[sreg];                   /* reg contents specified by Rs */
                switch(opr & 0xF) {
                    case 0x0:   /* ADR */
                        t = (temp & FSIGN) != 0;    /* set flag for sign bit not set in temp value */
                        t |= ((addr & FSIGN) != 0) ? 2 : 0; /* ditto for the reg value */
                        temp = temp + addr;         /* add the values */
                        if ((t == 3 && (temp & FSIGN) == 0) ||
                            (t == 0 && (addr & FSIGN) != 0))
                            ovr = 1;                /* we have an overflow */
                        break;

                    case 0x1:   /* ADRFW */
                        goto inv;           /* TODO */
                        break;
                    case 0x2:   /* MPRBR */
                        goto inv;           /* TODO */
                        break;
                    case 0x3:   /* SURFW */
                        goto inv;           /* TODO */
                        break;
                    case 0x4:   /* DVRFW */
                        goto inv;           /* TODO */
                        break;
                    case 0x5:   /* FIXW */
                        goto inv;           /* TODO */
                        break;
                    case 0x6:   /* MPRFW */
                        goto inv;           /* TODO */
                        break;
                    case 0x7:   /* FLTW */
                        goto inv;           /* TODO */
                        break;
                    case 0x8:   /* ADRM */
                        t = (temp & FSIGN) != 0;    /* set flag for sign bit not set in temp value */
                        t |= ((addr & FSIGN) != 0) ? 2 : 0; /* ditto for the reg value */
                        temp = temp + addr;         /* add the values */
                        if ((t == 3 && (temp & FSIGN) == 0) ||
                            (t == 0 && (temp & FSIGN) != 0))
                            ovr = 1;                /* we have an overflow */
                        temp &= GPR[4];             /* mask the destination reg */
                        break;
                    case 0x9:   /* INV */
                        goto inv;           /* TODO */
                        break;
                    case 0xA:   /* DVRBR */
                        goto inv;           /* TODO */
                        break;
                    case 0xB:   /* SURFD */
                        goto inv;           /* TODO */
                        break;
                    case 0xC:   /* DVRFD */
                        goto inv;           /* TODO */
                        break;
                    case 0xD:   /* FIXD */
                        goto inv;           /* TODO */
                        break;
                    case 0xE:   /* MPRFD */
                        goto inv;           /* TODO */
                        break;
                    case 0xF:   /* FLTD */
                        goto inv;           /* TODO */
                        break;
                }
                GPR[reg] = temp;                /* temp has destination reg value */
                if ((opr & 0xF) < 6) {
                    set_CCs(temp, ovr);         /* set the CC's */
                }
                /* the arithmetic exception will be handled */
                /* after instruction is completed */
                /* check for arithmetic exception trap enabled */
                if (ovr && (modes & AEXPBIT)) {
                    TRAPME = AEXPCEPT_TRAP; /* set the trap type */
                }
                break;

            case 0x3C>>2:               /* 0x3C HLF - HLF */    /* SUR and SURM */
                temp = GPR[reg];                    /* get negative value to add */
                addr = -GPR[sreg];                  /* reg contents specified by Rs */
                switch(opr & 0xF) {
                    case 0x0:       /* SUR */
                        t = (temp & FSIGN) != 0;    /* set flag for sign bit not set in temp value */
                        t |= ((addr & FSIGN) != 0) ? 2 : 0; /* ditto for the reg value */
                        temp = temp + addr;         /* add the values */
                        if ((t == 3 && (temp & FSIGN) == 0) ||
                            (t == 0 && (addr & FSIGN) != 0))
                            ovr = 1;                /* we have an overflow */
                        break;
                    case 0x8:       /* SURM */
                        t = (temp & FSIGN) != 0;    /* set flag for sign bit not set in temp value */
                        t |= ((addr & FSIGN) != 0) ? 2 : 0; /* ditto for the reg value */
                        temp = temp + addr;         /* add the values */
                        if ((t == 3 && (temp & FSIGN) == 0) ||
                            (t == 0 && (temp & FSIGN) != 0))
                            ovr = 1;                /* we have an overflow */
                        temp &= GPR[4];             /* mask the destination reg */
                        break;
                    case 0x9:   /* INV */
                        goto inv;           /* TODO */
                        break;
                    case 0xA:   /* DVRBR */
                        goto inv;           /* TODO */
                        break;
                    case 0xB:   /* SURFD */
                        goto inv;           /* TODO */
                        break;
                    case 0xC:   /* DVRFD */
                        goto inv;           /* TODO */
                        break;
                    case 0xD:   /* FIXD */
                        goto inv;           /* TODO */
                        break;
                    case 0xE:   /* MPRFD */
                        goto inv;           /* TODO */
                        break;
                    case 0xF:   /* FLTD */
                        goto inv;           /* TODO */
                        break;
                }
                GPR[reg] = temp;            /* save the result */
                if ((opr & 0xF) < 6) {
                    set_CCs(temp, ovr);     /* set CCs for result */
                }
                /* the arithmetic exception will be handled */
                /* after instruction is completed */
                /* check for arithmetic exception trap enabled */
                if (ovr && (modes & AEXPBIT)) {
                    TRAPME = AEXPCEPT_TRAP; /* set the trap type */
                }
                break;

        case 0x40>>2:               /* 0x40 */ /* MPR */
                if (reg & 1) {
                    /* Spec fault */
                    TRAPME = ADDRSPEC_TRAP; /* bad reg address, error */
                    goto newpsd;        /* go execute the trap now */
                }
                temp = GPR[reg+1];              /* get multiplicand */
                addr = GPR[sreg];               /* multiplier */

                /* change immediate value into a 64 bit value */
                dest = ((t_uint64)(addr & FMASK)) | ((addr & FSIGN) ? D32LMASK : 0);
//fprintf(stderr, "MPR reg %d GPR[reg] %.8x GPR[reg+1] %.8x dest %llx\r\n", reg, GPR[reg], GPR[reg+1], dest);
                source = ((t_uint64)(temp & FMASK)) | ((temp & FSIGN) ? D32LMASK : 0);
//fprintf(stderr, "MPR reg %d GPR[reg] %.8x GPR[reg+1] %.8x source %llx\r\n", reg, GPR[reg], GPR[reg+1], source);
                dest = dest * source;       /* do the multiply */
//fprintf(stderr, "MPR dest result %llx\r\n", dest);
                i_flags |= SD|SCC;              /* save regs and set CC's */
                dbl = 1;                        /* double reg save */
                break;

        case 0x44>>2:               /* 0x44 ADR - ADR */ /* DVR */
                /* sreg has Rs */
                if (reg & 1) {
                    /* Spec fault */
                    TRAPME = ADDRSPEC_TRAP;     /* bad reg address, error */
                    goto newpsd;                /* go execute the trap now */
                }
                /* get Rs divisor value */
                source = (t_uint64)(GPR[sreg]) | ((GPR[sreg] & FSIGN) ? D32LMASK : 0);
//fprintf(stderr, "DVR reg %d GPR[reg] %.8x GPR[reg+1] %.8x source %llx\r\n", reg, GPR[reg], GPR[reg+1], source);
                /* merge the dividend regs into the 64bit value */
                dest = (((t_uint64)GPR[reg]) << 32) | ((t_uint64)GPR[reg+1]);
//fprintf(stderr, "DVR reg %d GPR[reg] %.8x GPR[reg+1] %.8x source %llx\r\n", reg, GPR[reg], GPR[reg+1], dest);
                if (source == 0) {
                    goto doovr3;
                    break;
                }
                td = (t_int64)dest % (t_int64)source;   /* remainder */
                dbl = (td < 0);                 /* double reg is neg remainder */
//fprintf(stderr, "DVR remainder %llx dbl %x \r\n", td, dbl);
                if (((td & DMSIGN) ^ (dest & DMSIGN)) != 0) /* Fix sign if needed */
                    td = -td;                       /* dividend and remainder must be same sign */
//fprintf(stderr, "DVR fix sign %llx dbl %x source %llx dest %llx\r\n", td, dbl, source, dest);
                dest = (t_int64)dest / (t_int64)source; /* now do the divide */
//fprintf(stderr, "DVR dest %llx\r\n", dest);
                if ((dest & D32LMASK) != 0 && (dest & D32LMASK) != D32LMASK) {  /* test for overflow */
doovr3:
                    ovr = 1;                    /* the quotient exceeds 31 bit, overflow */
                    /* the arithmetic exception will be handled */
                    /* after instruction is completed */
                    /* check for arithmetic exception trap enabled */
                    if (ovr && (modes & AEXPBIT)) {
                        TRAPME = AEXPCEPT_TRAP; /* set the trap type */
                    }
                    /* the original regs must be returned unchanged if aexp */
                    set_CCs(temp, ovr);         /* set the CC's */
//fprintf(stderr, "DVR ovr %d reg %d GPR[reg] %.8x GPR[reg+1] %.8x dest %llx\r\n", ovr, reg, GPR[reg], GPR[reg+1], dest);
                } else {
                    GPR[reg] = (uint32)(td & FMASK);    /* reg gets remainder, reg+1 quotient */
                    GPR[reg+1] = (uint32)(dest & FMASK);    /* store quotient in reg+1 */
                    set_CCs(GPR[reg+1], ovr);       /* set the CC's, CC1 = ovr */
//fprintf(stderr, "DVR good reg %d GPR[reg] %.8x GPR[reg+1] %.8x dest %llx rem %llx\r\n", reg, GPR[reg], GPR[reg+1], dest, td);
                }
                break;
//#endif /* SIMPLE_MODE*/

        case 0x48>>2:               /* 0x48 INV - INV */    /* unused opcodes */
        case 0x4C>>2:               /* 0x4C INV - INV */    /* unused opcodes */
        default:
fprintf(stderr, "place @ UI op = %.8x\r\n", IR);
                TRAPME = UNDEFINSTR_TRAP;   /* Undefined Instruction Trap */
                goto newpsd;                /* handle trap */
                break;

        case 0x50>>2:               /* 0x50 INV - SD|ADR */ /* LA basemode */
                if ((modes & BASEBIT) == 0)     /* see if nonbased */
                    goto inv;                   /* invalid instruction in nonbased mode */
                if (modes & (BASEBIT|EXTDBIT)) {
                    dest = (t_uint64)addr;      /* just pure 24 bit address */
                } else {
                    dest = (t_uint64)(addr | ((FC & 4) << 17));     /* F bit to bit 12 */
                }
                break;

        case 0x54>>2:               /* 0x54 SM|ADR - INV */ /* (basemode STWBR) */
                if ((modes & BASEBIT) == 0)     /* see if nonbased */
                    goto inv;                   /* invalid instruction in nonbased mode */
                if (FC != 0) {                  /* word address only */
                    TRAPME = ADDRSPEC_TRAP;     /* bad reg address, error */
                    goto newpsd;                /* go execute the trap now */
                }
                dest = BR[reg];                 /* save the BR to memory */
                break;

        case 0x58>>2:               /* 0x58 SB|ADR - INV */ /* (basemode SUABR and LABR) */
                if ((modes & BASEBIT) == 0)     /* see if nonbased */
                    goto inv;                   /* invalid instruction in nonbased mode */
                if ((FC & 4) != 0) {            /* see if SUABR F=0 */
                     dest = BR[reg] - addr;     /* subtract addr from the BR and store back to BR */
                  } else {                      /* LABR if F=1 */
                     dest = addr;               /* addr goes to specified BR */
                  }
                  break;
        case 0x5C>>2:               /* 0x5C RM|SB|ADR - INV */  /* (basemode LWBR and BSUBM) */
                if ((modes & BASEBIT) == 0)     /* see if nonbased */
                    goto inv;                   /* invalid instruction in nonbased mode */
                if (FC != 0) {                  /* word address only */
                    TRAPME = ADDRSPEC_TRAP;     /* bad reg address, error */
                    goto newpsd;                /* go execute the trap now */
                }
                if ((FC & 0x4) != 0x4) {        /* this is a LWBR instruction */
                    BR[reg] = source;           /* load memory location in BR */
                } else {                        /* this is a CALLM/BSUBM instruction */
                    /* if Rd field is 0 (reg is b6-8), this is a BSUBM instruction */
                    /* otherwise it is a CALLM instruction (Rd != 0) */
                    if (reg == 0) {
                        /* BSUBM instruction */
                        uint32 fp = BR[2];              /* get dword bounded frame pointer from BR2 */
                        if ((BR[2] & 0x7) != 0)  {
                            /* Fault, must be dw bounded address */
                            TRAPME = ADDRSPEC_TRAP;     /* bad address, error */
                            goto newpsd;                /* go execute the trap now */
                        }
                        M[fp>>2] = (PSD1+4) & 0x01fffffe;   /* save AEXP bit and PC into frame */
                        M[(fp>>2)+1] = 0x80000000;  /* show frame created by BSUB instr */
                        BR[3] = GPR[0];             /* GPR 0 to BR 3 (AP) */
                        BR[0] = BR[2];              /* set frame pointer from BR 2 into BR 0 */
                        BR[1] = (addr+0x80) & 0xfffffe; /* effective address to BR 1 */
                        PSD1 = (PSD1 & 0xff000000) | BR[1]; /* New PSD address */
                    } else {
                        /* CALM instruction */
                        /* get frame pointer from BR2 - 16 words & make it a dword addr */
                        uint32 cfp = ((BR[2]-0x40) & 0xfffffff8);
                        M[cfp>>2] = (PSD1+4) & 0x01fffffe;  /* save AEXP bit and PC from PSD1 in to frame */
                        M[(cfp>>2)+1] = 0x00000000;     /* show frame created by CALL instr */
                        for (ix=0; ix<8; ix++)
                            M[(cfp>>2)+ix+2] = BR[ix];  /* save BRs 0-7 to call frame */
                        for (ix=2; ix<8; ix++)
                            M[(cfp>>2)+ix+10] = GPR[ix];/* save GPRs 2-7 to call frame */
                        BR[3] = GPR[reg];           /* Rd to BR 3 (AP) */
                        BR[0] = (uint32)cfp;        /* set current frame pointer into BR[0] */
                        BR[2] = (uint32)cfp;        /* set current frame pointer into BR[2] */
                        BR[1] = (addr + 0x80) & 0xfffffe;   /* effective address to BR 1 */
                        PSD1 = (PSD1 & 0xff000000) | BR[1]; /* New PSD address */
                    }
                }
                break;

        case 0x60>>2:               /* 0x60 SH|HLF - INV */ /* NOR Rd,Rs */
                if ((modes & BASEBIT)) {            /* only for nonbased mode */
                    TRAPME = UNDEFINSTR_TRAP;   /* Undefined Instruction Trap */
                    goto newpsd;                /* handle trap */
                }
                temp = GPR[reg];                /* Rd */
                t = 0;                          /* no shifts yet */
                /* exponent must not be zero or all 1's */
                if (temp != 0 && temp != FMASK) {
                    /* non zero value or not all 1's, so normalize */
                    uint32 m = temp & 0xF8000000;
                    /* shift left 4 bits at a time while left most 5 bits are not zero or all 1's */
                    while ((m == 0) || (m == 0xF8000000)) {
                        temp <<= 4;             /* move left 4 bits */
                        t++;                    /* increment number times shifted */
                        m = temp & 0xF8000000;  /* get left most 5 bits again */
                    }
                    /* temp value is now normalized with non zero nor all 1's value */
                    GPR[reg] = temp;            /* save the normalized value */
                    GPR[(IR >> 20) & 7] = 0x40 - t; /* subtract shift count from 0x40 into Rs */
                } else {
                    GPR[(IR >> 20) & 7] = 0;    /* set exponent to zero for zero value */
                }
                break;

        case 0x64>>2:               /* 0x64 SD|HLF - INV */ /* NORD */
                if ((modes & BASEBIT)) {        /* only for nonbased mode */
                    TRAPME = UNDEFINSTR_TRAP;   /* Undefined Instruction Trap */
                    goto newpsd;                /* handle trap */
                }
                if (reg & 1) {                  /* see if odd reg specified */
                    TRAPME = ADDRSPEC_TRAP;     /* bad reg address, error */
                    goto newpsd;                /* go execute the trap now */
                }
                addr = GPR[reg];                /* high order 32 bits */
                temp = GPR[reg+1];              /* low order 32 bits */
                t = 0;                          /* zero shift count */
                if ((temp|addr) != 0 && (temp&addr) != FMASK) {
                    uint32 m = temp & 0xF8000000;   /* upper 5 bit mask */
                    /* shift until upper 5 bits are neither 0 or all 1's */
                    while ((m == 0) || (m == 0xF8000000)) {
                        temp <<= 4;             /* shift over 4 bits at a time */
                        m = temp & 0xF8000000;  /* upper 5 bits */
                        temp |= (addr >> 28) & 0xf; /* copy in upper 4 bits from R+1 */
                        addr <<= 4;             /* shift 4 bits of zero int R+1 */
                        t++;                    /* bump shift count */
                    }
                    GPR[reg] = addr;            /* save the new values */
                    GPR[reg|1] = temp;
                }
                if (t != 0)
                    t = 0x40 - t;               /* set the shift cnt */
                GPR[(IR >> 20) & 7] = t;        /* put 0x40 - shift count or 0 into RS */
                break;

        case 0x68>>2:           /* 0x68 SCC|SD|HLF - INV */ /* non basemode SCZ */
                if (modes & BASEBIT) 
                    goto inv;               /* invalid instruction */
                goto sacz;                  /* use basemode sacz instruction */

        case 0x6C>>2:           /* 0x6C SD|HLF - INV */ /* non basemode SRA & SLA */
                if (modes & BASEBIT) 
                    goto inv;           /* invalid instruction */
sra:
sla:
                bc = opr & 0x1f;        /* get bit shift count */
                temp = GPR[reg];        /* get reg value to shift */
                t = temp & FSIGN;       /* sign value */
                if (opr & 0x0040) {     /* is this SLA */
                    ovr = 0;            /* set ovr off */
                    for (ix=0; ix<bc; ix++) {
                        temp <<= 1;             /* shift bit into sign position */
                        if ((temp & FSIGN) ^ t) /* see if sign bit changed */
                            ovr = 1;            /* set arithmetic exception flag */
                    }
                    temp &= ~BIT0;          /* clear sign bit */
                    temp |= t;              /* restore original sign bit */
                    GPR[reg] = temp;        /* save the new value */
                    PSD1 &= 0x87FFFFFE;     /* clear the old CC's */
                    if (ovr)
                        PSD1 |= BIT1;       /* CC1 in PSD */
                    /* the arithmetic exception will be handled */
                    /* after instruction is completed */
                    /* check for arithmetic exception trap enabled */
                    if (ovr && (modes & AEXPBIT)) {
                        TRAPME = AEXPCEPT_TRAP; /* set the trap type */
                    }
                } else {                    /* this is a SRA */
                    for (ix=0; ix<bc; ix++) {
                        temp >>= 1;         /* shift bit 0 right one bit */
                        temp |= t;          /* restore original sign bit */
                    }
                    GPR[reg] = temp;        /* save the new value */
                }
                break;

       case 0x70>>2:             /* 0x70 SD|HLF - INV */ /* non-basemode SRL & SLL */
                if (modes & BASEBIT) 
                    goto inv;           /* invalid instruction in basemode */
sll:
srl:
                bc = opr & 0x1f;        /* get bit shift count */
                temp = GPR[reg];        /* get reg value to shift */
//fprintf(stderr, "before SLL/SRL dest %.8x cnt %d\r\n", temp, bc);
                if (opr & 0x0040)       /* is this SLL, bit 9 set */
                    temp <<= bc;        /* shift left #bits */
                else
                    temp >>= bc;        /* shift right #bits */
                dest = temp;            /* value to be output */
//fprintf(stderr, "SLL/SRL dest %.8x cnt %d\r\n", (uint32)dest, bc);
                break;

       case 0x74>>2:             /* 0x74 SD|HLF - INV */ /* non-basemode SRC & SLC */
                if (modes & BASEBIT) 
                    goto inv;               /* invalid instruction in basemode */
slc:
src:
                bc = opr & 0x1f;            /* get bit shift count */
                temp = GPR[reg];            /* get reg value to shift */
                if (opr & 0x0040) {         /* is this SLC, bit 9 set */
                    for (ix=0; ix<bc; ix++) {
                        t = temp & BIT0;    /* get sign bit status */
                        temp <<= 1;         /* shift the bit out */
                        if (t)
                            temp |= 1;      /* the sign bit status */
                    }
                } else {                     /* this is SRC, bit 9 not set */
                    for (ix=0; ix<bc; ix++) {
                        t = temp & 1;       /* get bit 31 status */
                        temp >>= 1;         /* shift the bit out */
                        if (t)
                            temp |= BIT0;   /* put in new sign bit */
                    }
                }
                dest = temp;                /* shift result */
                break;

/*TODO*/
        case 0x78>>2:               /* 0x78 HLF - INV */ /* non-basemode SRAD & SLAD */
                if (modes & BASEBIT)            /* Base mode? */
                    goto inv;                   /* invalid instruction in basemode */
                if (reg & 1) {                  /* see if odd reg specified */
                    TRAPME = ADDRSPEC_TRAP;     /* bad reg address, error */
                    goto newpsd;                /* go execute the trap now */
                }
                bc = opr & 0x1f;                /* get bit shift count */
                dest = (t_uint64)GPR[reg+1];            /* get low order reg value */
                dest |= (((t_uint64)GPR[reg]) << 32);   /* insert upper reg value */
                source = dest & DMSIGN;         /* 64 bit sign value */
                if (opr & 0x0040) {             /* is this SLAD */
                    ovr = 0;                    /* set ovr off */
                    for (ix=0; ix<bc; ix++) {
                        dest <<= 1;             /* shift bit into sign position */
                        if ((dest & DMSIGN) ^ source)   /* see if sign bit changed */
                            ovr = 1;            /* set arithmetic exception flag */
                    }
                    dest &= ~DMSIGN;        /* clear sign bit */
                    dest |= source;         /* restore original sign bit */
                    GPR[reg+1] = (uint32)(dest & FMASK);    /* save the low order reg */
                    GPR[reg] = (uint32)((dest>>32) & FMASK);/* save the hi order reg */
                    PSD1 &= 0x87FFFFFE;     /* clear the old CC's */
                    if (ovr)
                        PSD1 |= BIT1;       /* CC1 in PSD */
                    /* the arithmetic exception will be handled */
                    /* after instruction is completed */
                    /* check for arithmetic exception trap enabled */
                    if (ovr && (modes & AEXPBIT)) {
                        TRAPME = AEXPCEPT_TRAP; /* set the trap type */
                    }
                } else {                    /* this is a SRAD */
                    for (ix=0; ix<bc; ix++) {
                        dest >>= 1;         /* shift bit 0 right one bit */
                        dest |= source;     /* restore original sign bit */
                    }
                    GPR[reg+1] = (uint32)(dest & FMASK);    /* save the low order reg */
                    GPR[reg] = (uint32)((dest>>32) & FMASK);/* save the hi order reg */
                }
                break;

        case 0x7C>>2:               /* 0x7C SDD|HLF - INV */ /* non-basemode SRLD & SLLD */
                if (modes & BASEBIT) 
                    goto inv;               /* invalid instruction in basemode */
                if (reg & 1) {              /* see if odd reg specified */
                    TRAPME = ADDRSPEC_TRAP; /* bad reg address, error */
                    goto newpsd;            /* go execute the trap now */
                }
                dest = (t_uint64)GPR[reg+1];            /* get low order reg value */
                dest |= (((t_uint64)GPR[reg]) << 32);   /* insert upper reg value */
                bc = opr & 0x1f;        /* get bit shift count */
                if (opr & 0x0040)       /* is this SLL, bit 9 set */
                    dest <<= bc;        /* shift left #bits */
                else
                    dest >>= bc;        /* shift right #bits */
                GPR[reg+1] = (uint32)(dest & FMASK);    /* save the low order reg */
                GPR[reg] = (uint32)((dest>>32) & FMASK);/* save the hi order reg */
                break;

        case 0x80>>2:           /* 0x80 SD|ADR - SD|ADR */ /* LEAR */
                /* convert address to real physical address */
                TRAPME = RealAddr(addr, &temp, &t);
                if (TRAPME != ALLOK)
                    goto newpsd;        /* memory read error or map fault */
                /* OS code says F bit is not transferred, so just ignore it */
//              if (FC & 4)             /* see if F bit was set */
//                  temp |= 0x01000000; /* set bit 7 of address */
                dest = temp;            /* put in dest to go out */
//              fprintf(stderr, "LEAR Inst %.8x addr %.8x raddr %.8x PSD %.8x %.8x SPAD PSD@ %x\r\n",
//                  IR, addr, temp, PSD1, PSD2, SPAD[0xf5]);
//              sim_debug(DEBUG_CMD, &cpu_dev, "LEAR Inst %.8x addr %.8x raddr %.8x PSD %.8x %.8x\n",
//                  IR, addr, temp, PSD1, PSD2);
                break;

        case 0x84>>2:               /* 0x84 SCC|SD|RR|RM|ADR - SD|RM|ADR */ /* ANMx */
                dest &= source;
                break;

        case 0x88>>2:               /* 0x88 SCC|SD|RR|RM|ADR - SD|RM|ADR */ /* ORMx */
                dest |= source;
                break;

        case 0x8C>>2:               /* 0x8C  SCC|SD|RR|RM|ADR - SD|RM|ADR */ /* EOMx */
                dest ^= source;
                break;

        case 0x90>>2:               /* 0x90 SCC|RR|RM|ADR - RM|ADR */ /* CAMx */
//fprintf(stderr, "CAMW src %llx dest %llx\r\n", source, dest);
                dest -= source;
                break;

        case 0x94>>2:               /* 0x94 RR|RM|ADR - RM|ADR */ /* CMMx */
    //FIXME CMMD need both regs to be masked with R4
                if (dbl) {
                    /* we need to and both regs */
                    t_uint64 nm = (((t_uint64)GPR[4]) << 32) | (((t_uint64)GPR[4]) & D32RMASK);
                    td = dest;              /* save dest */
                    dest ^= source;
                    dest &= nm;             /* mask both regs with reg 4 contents */
//      fprintf(stderr, "CMMD PSD %x src %llx dst %llx GPR[4] %llx result %llx CC4 %x\r\n",
//          PSD1, source, td, nm, dest, dest==0ll?1:0);
                } else {
                    td = dest;              /* save dest */
                    dest ^= source;         /* <= 32 bits, so just do lower 32 bits */
                    dest &= (((t_uint64)GPR[4]) & D32RMASK);        /* mask with reg 4 contents */
//      fprintf(stderr, "CMMW PSD %x src %llx dst %llx GPR[4] %llx result %llx CC4 %x\r\n",
//          PSD1, source, td, ((t_uint64)GPR[4]), dest, dest==0ll?1:0);
                }           
                CC = 0;
                if (dest == 0ll)
                    CC |= CC4BIT;
                PSD1 &= 0x87FFFFFE;     /* clear the old CC's from PSD1 */
                PSD1 |= CC;             /* update the CC's in the PSD */
                break;

        case 0x98>>2:               /* 0x98 ADR - ADR */ /* SBM */
                if ((FC & 04) == 0)  {
                    /* Fault, f-bit must be set for SBM instruction */
                    TRAPME = ADDRSPEC_TRAP;     /* bad reg address, error */
                    goto newpsd;                /* go execute the trap now */
                }
                if ((TRAPME = Mem_read(addr, &temp)))   /* get the word from memory */
                    goto newpsd;                /* memory read error or map fault */

                t = (CC & 0x70000000) >> 1;     /* get old CC bits 1-3 into CCs 2-4*/
                /* use C bits and bits 6-8 (reg) to generate shift bit count */
                bc = ((FC & 3) << 3) | reg;     /* get # bits to shift right */
                bc = BIT0 >> bc;                /* make a bit mask of bit number */
//fprintf(stderr, "SBM bc %.8x addr %.8x CC %0.2x PSD1 %.8x temp %.8x\r\n", bc, addr, CC, PSD1, temp);
                PSD1 &= 0x87FFFFFE;             /* clear the old CC's from PSD1 */
                if (temp & bc)                  /* test the bit in memory */
                    t |= CC1BIT;                /* set CC1 to the bit value */
                PSD1 |= t;                      /* update the CC's in the PSD */
                temp |= bc;                     /* set the bit in temp */
//fprintf(stderr, "SBM bc %.8x addr %.8x CC %0.2x PSD1 %.8x temp %.8x\r\n", bc, addr, CC, PSD1, temp);
                if ((TRAPME = Mem_write(addr, &temp)))  /* put word back into memory */
                    goto newpsd;                /* memory write error or map fault */
                break;
                  
        case 0x9C>>2:               /* 0x9C ADR - ADR */ /* ZBM */
                if ((FC & 04) == 0)  {
                    /* Fault, byte address not allowed */
                    TRAPME = ADDRSPEC_TRAP;     /* bad reg address, error */
                    goto newpsd;                /* go execute the trap now */
                }
                if ((TRAPME = Mem_read(addr, &temp)))   /* get the word from memory */
                    goto newpsd;                /* memory read error or map fault */

                t = (CC & 0x70000000) >> 1;     /* get old CC bits 1-3 into CCs 2-4*/
                /* use C bits and bits 6-8 (reg) to generate shift bit count */
                bc = ((FC & 3) << 3) | reg;     /* get # bits to shift right */
                bc = BIT0 >> bc;                /* make a bit mask of bit number */
//fprintf(stderr, "ZBM bc %.8x addr %.8x CC %0.2x PSD1 %.8x temp %.8x\r\n", bc, addr, CC, PSD1, temp);
                PSD1 &= 0x87FFFFFE;             /* clear the old CC's from PSD1 */
                if (temp & bc)                  /* test the bit in memory */
                    t |= CC1BIT;                /* set CC1 to the bit value */
                PSD1 |= t;                      /* update the CC's in the PSD */
                temp &= ~bc;                    /* reset the bit in temp */
//fprintf(stderr, "ZBM bc %.8x addr %.8x CC %0.2x PSD1 %.8x temp %.8x\r\n", bc, addr, CC, PSD1, temp);
                if ((TRAPME = Mem_write(addr, &temp)))  /* put word into memory */
                    goto newpsd;                /* memory write error or map fault */
                break;

        case 0xA0>>2:               /* 0xA0 ADR - ADR */ /* ABM */
                if ((FC & 04) == 0)  {
                    /* Fault, byte address not allowed */
                    TRAPME = ADDRSPEC_TRAP;     /* bad reg address, error */
                    goto newpsd;                /* go execute the trap now */
                }
                if ((TRAPME = Mem_read(addr, &temp)))   /* get the word from memory */
                    goto newpsd;                /* memory read error or map fault */

                /* use C bits and bits 6-8 (reg) to generate shift bit count */
                bc = ((FC & 3) << 3) | reg;     /* get # bits to shift right */
                bc = BIT0 >> bc;                /* make a bit mask of bit number */
//fprintf(stderr, "ABM B4 bc %.8x addr %.8x CC %0.2x PSD1 %.8x temp %.8x\r\n", bc, addr, CC, PSD1, temp);
                ovr = (temp & FSIGN) != 0;      /* set ovr to status of sign bit 0 */
                temp += bc;                     /* add the bit value to the reg */
                ovr ^= (temp & FSIGN) != 0;     /* set ovr if sign bit changed */
                set_CCs(temp, ovr);             /* set the CC's, CC1 = ovr */
//fprintf(stderr, "ABM AF bc %.8x addr %.8x CC %0.2x ovr %x PSD1 %.8x temp %.8x\r\n", bc, addr, CC, ovr, PSD1, temp);
                if ((TRAPME = Mem_write(addr, &temp)))  /* put word into memory */
                    goto newpsd;                /* memory write error or map fault */
                /* the arithmetic exception will be handled */
                /* after instruction is completed */
                /* check for arithmetic exception trap enabled */
                if (ovr && (modes & AEXPBIT))
                    TRAPME = AEXPCEPT_TRAP;     /* set the trap type */
                break;

        case 0xA4>>2:               /* 0xA4 ADR - ADR */ /* TBM */
                if ((FC & 04) == 0)  {
                    /* Fault, byte address not allowed */
                    TRAPME = ADDRSPEC_TRAP;     /* bad reg address, error */
                    goto newpsd;                /* go execute the trap now */
                }
                if ((TRAPME = Mem_read(addr, &temp)))   /* get the word from memory */
                    goto newpsd;                /* memory read error or map fault */

                t = (CC & 0x70000000) >> 1;     /* get old CC bits 1-3 into CCs 2-4*/
                /* use C bits and bits 6-8 (reg) to generate shift bit count */
                bc = ((FC & 3) << 3) | reg;     /* get # bits to shift right */
                bc = BIT0 >> bc;                /* make a bit mask of bit number */
//fprintf(stderr, "TBM bc %.8x addr %.8x CC %0.2x PSD1 %.8x temp %.8x\r\n", bc, addr, CC, PSD1, temp);
                PSD1 &= 0x87FFFFFE;             /* clear the old CC's from PSD1 */
                if (temp & bc)                  /* test the bit in memory */
                    t |= CC1BIT;                /* set CC1 to the bit value */
                PSD1 |= t;                      /* update the CC's in the PSD */
//fprintf(stderr, "TBM bc %.8x addr %.8x CC %0.2x PSD1 %.8x temp %.8x\r\n", bc, addr, CC, PSD1, temp);
                break;

        case 0xA8>>2:               /* 0xA8 RM|ADR - RM|ADR */ /* EXM */
                if ((FC & 04) != 0 || FC == 2) {    /* can not be byte or doubleword */
                    /* Fault */
                    TRAPME = ADDRSPEC_TRAP;     /* bad reg address, error */
                    goto newpsd;                /* go execute the trap now */
                }
                if ((TRAPME = Mem_read(addr, &temp)))   /* get the word from memory */
                    goto newpsd;                /* memory read error or map fault */

                IR = temp;                      /* get instruction from memory */
                if (FC == 2)                    /* see if right halfword specified */
                    IR <<= 16;                  /* move over the HW instruction */
                if ((IR & 0xFC7F0000) == 0xC8070000 ||
                    (IR & 0xFF800000) == 0xA8000000 ||
                    (IR & 0xFC000000) == 0x80000000) {
                    /* Fault, attempt to execute another EXR, EXRR, EXM, or LEAR  */
                    goto inv;           /* invalid instruction */
                }
                EXM_EXR = 4;            /* set PC increment for EXM */
                goto exec;              /* go execute the instruction */
 
        case 0xAC>>2:               /* 0xAC SCC|SD|RM|ADR - SD|RM|ADR */ /* Lx */
                dest = source;      /* set value to load into reg */
                break;

        case 0xB0>>2:               /* 0xB0 SCC|SD|RM|ADR - SD|RM|ADR */ /* LMx */
    //FIXME LMD need both regs to be masked with R4
                if (dbl) {
                    /* we need to and both regs */
                    t_uint64 nm = (((t_uint64)GPR[4]) << 32) | (((t_uint64)GPR[4]) & D32RMASK);
                    dest = source & nm; /* mask both regs with reg 4 contents */
//  fprintf(stderr, "LMD PSD %x src %llx GPR[4] %llx result %llx\r\n", PSD1, source, nm, dest);
                } else {
                    dest = source;          /* <= 32 bits, so just do lower 32 bits */
                    dest &= (((t_uint64)GPR[4]) & D32RMASK);        /* mask with reg 4 contents */
//  fprintf(stderr, "LMW PSD %x src %llx GPR[4] %llx result %llx\r\n", PSD1, source, ((t_uint64)GPR[4]), dest);
                }           
                break;
 
        case 0xB4>>2:               /* 0xB4 SCC|SD|RM|ADR - SD|RM|ADR */ /* LNx */
                dest = -source;     /* set the value to load into reg */
                if (dest == source)
                    ovr = 1;    /* set arithmetic exception status */
                /* the arithmetic exception will be handled */
                /* after instruction is completed */
                /* check for arithmetic exception trap enabled */
                if (ovr && (modes & AEXPBIT)) {
                    TRAPME = AEXPCEPT_TRAP; /* set the trap type */
                }
                break;

        case 0xBC>>2:           /* 0xBC SCC|SD|RR|RM|ADR - SD|RM|ADR */ /* SUMx */
                source = -source;
                /* Fall through */

        case 0xE8>>2:           /* 0xE8 SCC|SM|RR|RM|ADR - SM|RM|ADR */ /* ARMx */
        case 0xB8>>2:           /* 0xB8 SCC|SD|RR|RM|ADR - SD|RM|ADR */ /* ADMx */
                t = (source & DMSIGN) != 0;
                t |= ((dest & DMSIGN) != 0) ? 2 : 0;
//fprintf(stderr, "ARMW t %x src %llx dest %llx", t, source, dest);
                dest = dest + source;
                if ((t == 3) && ((dest & DMSIGN) == 0)) {
                    ovr = 1;
//fprintf(stderr, " @ovr0 %d", ovr);
                }
                if ((t == 0) && ((dest & DMSIGN) != 0)) {
                    ovr = 1;
//fprintf(stderr, " @ovr1 %d", ovr);
                }
                if ((dbl == 0) && ((dest & D32LMASK) != 0) && ((dest & D32LMASK) != D32LMASK)) {
                    ovr = 1;
//fprintf(stderr, " @ovr2 %d", ovr);
                }

//fprintf(stderr, " result %llx\r\n", dest);
                /* the arithmetic exception will be handled */
                /* after instruction is completed */
                /* check for arithmetic exception trap enabled */
                if (ovr && (modes & AEXPBIT)) {
                    TRAPME = AEXPCEPT_TRAP; /* set the trap type */
                }
                break;

        case 0xC0>>2:               /* 0xC0 SCC|SD|RM|ADR - SD|RM|ADR */ /* MPMx */
                if (reg & 1) {                  /* see if odd reg specified */
                    TRAPME = ADDRSPEC_TRAP;     /* bad reg address, error */
                    goto newpsd;                /* go execute the trap now */
                }
                dest = (t_uint64)GPR[reg+1];    /* get low order reg value */
//fprintf(stderr, "MPM? before src %llx dest %llx\r\n", source, dest);
                dest = (t_uint64)((t_int64)dest * (t_int64)source);
//fprintf(stderr, "MPM? after src %llx dest %llx\r\n", source, dest);
                dbl = 1;
                break;

        case 0xC4>>2:               /* 0xC4 RM|ADR - RM|ADR */ /* DVMx */
                if (reg & 1) {                  /* see if odd reg specified */
                    TRAPME = ADDRSPEC_TRAP;     /* bad reg address, error */
                    goto newpsd;                /* go execute the trap now */
                }
//fprintf(stderr, "DVM reg %d GPR[reg] %.8x GPR[reg+1] %.8x source %llx\r\n", reg, GPR[reg], GPR[reg+1], source);
                dest = (((t_uint64)GPR[reg]) << 32);    /* insert upper reg value */
                dest |= (t_uint64)GPR[reg+1];       /* get low order reg value */
//fprintf(stderr, "DVM reg %d GPR[reg] %.8x GPR[reg+1] %.8x dest %llx\r\n", reg, GPR[reg], GPR[reg+1], dest);
                if (source == 0) {
                    goto doovr;                 /* we have div by zero */
                }
                td = ((t_int64)dest % (t_int64)source); /* remainder */
                dbl = (td < 0);                 /* double reg if neg remainder */
//fprintf(stderr, "DVM remainder %llx dbl %x \r\n", td, dbl);
                if (((td & DMSIGN) ^ (dest & DMSIGN)) != 0) /* Fix sign if needed */
                    td = -td;                       /* dividend and remainder must be same sign */
//fprintf(stderr, "DVM fix sign %llx dbl %x source %llx dest %llx\r\n", td, dbl, source, dest);
                dest = (t_int64)dest / (t_int64)source; /* now do the divide */
//fprintf(stderr, "DVM dest %llx\r\n", dest);
                if ((dest & D32LMASK) != 0 && (dest & D32LMASK) != D32LMASK) {  /* test for overflow */
doovr:
                    ovr = 1;                    /* the quotient exceeds 31 bit, overflow */
                    /* the arithmetic exception will be handled */
                    /* after instruction is completed */
                    /* check for arithmetic exception trap enabled */
                    if (modes & AEXPBIT)
                        TRAPME = AEXPCEPT_TRAP; /* set the trap type */
                    /* the original regs must be returned unchanged if aexp */
//                  i_flags &= ~SD;             /* remove the store to reg flag */
//                  dest = (t_uint64)GPR[reg+1];    /* get low order reg value */
//                  dest |= (((t_uint64)GPR[reg]) << 32);   /* insert upper reg value */
                    set_CCs(GPR[reg+1], ovr);       /* set the CC's, CC1 = ovr */
//fprintf(stderr, "DVM ovr %d reg %d GPR[reg] %.8x GPR[reg+1] %.8x dest %llx\r\n", ovr, reg, GPR[reg], GPR[reg+1], dest);
                } else {
                    GPR[reg] = (uint32)(td & FMASK);    /* reg gets remainder, reg+1 quotient */
                    GPR[reg+1] = (uint32)(dest & FMASK);    /* store quotient in reg+1 */
                    set_CCs(GPR[reg+1], ovr);       /* set the CC's, CC1 = ovr */
//fprintf(stderr, "DVM good reg %d GPR[reg] %.8x GPR[reg+1] %.8x dest %llx rem %llx\r\n", reg, GPR[reg], GPR[reg+1], dest, td);
                }
                break;

        case 0xC8>>2:               /* 0xC8 IMM - IMM */ /* Immedate */
                temp = GPR[reg];                /* get reg contents */
                addr = IR & RMASK;              /* sign extend 16 bit imm value from IR */
                if (addr & 0x8000)              /* negative */
                    addr |= LMASK;              /* extend sign */
//fprintf(stderr, "C8 IMM temp %.8x addr %.8x\r\n", temp, addr);

                switch(opr & 0xF) {             /* switch on aug code */
                case 0x0:       /* LI */  /* SCC | SD */
                    GPR[reg] = addr;            /* put immediate value into reg */
                    set_CCs(addr, ovr);         /* set the CC's, CC1 = ovr */
                    break;

                case 0x2:       /* SUI */
                    addr = -addr;               /* just make value a negative add */
                    /* drop through */
                case 0x1:       /* ADI */
                    t = (temp & FSIGN) != 0;    /* set flag for sign bit not set in reg value */
                    t |= ((addr & FSIGN) != 0) ? 2 : 0; /* ditto for the extended immediate value */
                    temp = temp + addr;         /* now add the numbers */
                    if ((t == 3 && (temp & FSIGN) == 0) ||
                        (t == 0 && (temp & FSIGN) != 0))
                        ovr = 1;                /* we have an overflow */
                    GPR[reg] = temp;            /* save the result */
                    set_CCs(temp, ovr);         /* set the CC's, CC1 = ovr */
                    /* the arithmetic exception will be handled */
                    /* after instruction is completed */
                    /* check for arithmetic exception trap enabled */
                    if (ovr && (modes & AEXPBIT)) {
                        TRAPME = AEXPCEPT_TRAP; /* set the trap type */
                    }
                    break;

                case 0x3:       /* MPI */
                    if (reg & 1) {              /* see if odd reg specified */
                        TRAPME = ADDRSPEC_TRAP; /* bad reg address, error */
                        goto newpsd;            /* go execute the trap now */
                    }
                    /* change immediate value into a 64 bit value */
                    source = ((t_uint64)(addr & FMASK)) | ((addr & FSIGN) ? D32LMASK : 0);
//fprintf(stderr, "MPI reg %d GPR[reg] %.8x GPR[reg+1] %.8x source %llx\r\n", reg, GPR[reg], GPR[reg+1], source);
                    temp = GPR[reg+1];          /* get reg multiplier */
                    dest = ((t_uint64)(temp & FMASK)) | ((temp & FSIGN) ? D32LMASK : 0);
//fprintf(stderr, "MPI reg %d GPR[reg] %.8x GPR[reg+1] %.8x dest %llx\r\n", reg, GPR[reg], GPR[reg+1], dest);
                    dest = dest * source;       /* do the multiply */
//fprintf(stderr, "MPI dest result %llx\r\n", dest);
                    i_flags |= SD|SCC;          /* save regs and set CC's */
                    dbl = 1;                    /* double reg save */
                    break;

                case 0x4:       /* DVI */
                    if (reg & 1) {              /* see if odd reg specified */
                        TRAPME = ADDRSPEC_TRAP; /* bad reg address, error */
                        goto newpsd;            /* go execute the trap now */
                    }
                    /* change immediate value into a 64 bit value */
                    source = ((t_uint64)(addr & FMASK)) | ((addr & FSIGN) ? D32LMASK : 0);
//fprintf(stderr, "DVI reg %d GPR[reg] %.8x GPR[reg+1] %.8x source %llx\r\n", reg, GPR[reg], GPR[reg+1], source);
                    if (source == 0) {
                        goto doovr2;
                    }
                    dest = (((t_uint64)GPR[reg]) << 32);    /* get upper reg value */
                    dest |= (t_uint64)GPR[reg+1];       /* insert low order reg value */
//fprintf(stderr, "DVI reg %d GPR[reg] %.8x GPR[reg+1] %.8x dest %llx\r\n", reg, GPR[reg], GPR[reg+1], dest);
                    td = ((t_int64)dest % (t_int64)source); /* remainder */
                    dbl = (td < 0);                 /* double reg if neg remainder */
//fprintf(stderr, "DVI remainder %llx dbl %x \r\n", td, dbl);
                    if (((td & DMSIGN) ^ (dest & DMSIGN)) != 0) /* Fix sign if needed */
                        td = -td;                       /* dividend and remainder must be same sign */
//fprintf(stderr, "DVI fix sign %llx dbl %x source %llx dest %llx\r\n", td, dbl, source, dest);
                    dest = (t_int64)dest / (t_int64)source; /* now do the divide */
//fprintf(stderr, "DVI dest %llx\r\n", dest);
                    if ((dest & D32LMASK) != 0 && (dest & D32LMASK) != D32LMASK) {  /* test for overflow */
doovr2:
                        ovr = 1;                    /* the quotient exceeds 31 bit, overflow */
                        /* the arithmetic exception will be handled */
                        /* after instruction is completed */
                        /* check for arithmetic exception trap enabled */
                        if (modes & AEXPBIT)
                            TRAPME = AEXPCEPT_TRAP;     /* set the trap type */
                        /* the original regs must be returned unchanged if aexp */
                        /* put reg values back in dest for CC test */
//                      dest = (t_uint64)GPR[reg+1];        /* get low order reg value */
//                      dest |= (((t_uint64)GPR[reg]) << 32);   /* insert upper reg value */
                        set_CCs(GPR[reg+1], ovr);       /* set the CC's, CC1 = ovr */
//                      i_flags |= SCC;                 /* set CC's */
//fprintf(stderr, "DVI ovr %d reg %d GPR[reg] %.8x GPR[reg+1] %.8x dest %llx\r\n", ovr, reg, GPR[reg], GPR[reg+1], dest);
                    } else {
                        GPR[reg] = (uint32)(td & FMASK);    /* reg gets remainder, reg+1 quotient */
                        GPR[reg+1] = (uint32)(dest & FMASK);    /* store quotient in reg+1 */
                        set_CCs(GPR[reg+1], ovr);       /* set the CC's, CC1 = ovr */
//fprintf(stderr, "DVI good reg %d GPR[reg] %.8x GPR[reg+1] %.8x dest %llx rem %llx\r\n", reg, GPR[reg], GPR[reg+1], dest, td);
                    }
                    break;

                case 0x5:           /* CI */    /* SCC */
                    temp = ((int)temp - (int)addr);     /* subtract imm value from reg value */
                    set_CCs(temp, ovr);         /* set the CC's, CC1 = ovr */
//fprintf(stderr, "CI IMM temp %.8x addr %.8x PSD1 %.8x flags %.8x\r\n", temp, addr, PSD1, i_flags);
                    break;

/* SVC instruction format C806 */
/* |-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------| */
/* |00 01 02 03 04 05|06 07 08|09 10 11|12 13 14 15|16 17 18 19|20 21 22 23 24 25 26 27 28 29 30 31| */
/* |     Op Code     |   N/U  |  N/U   |   Aug     | SVC Index |        SVC Call Number            | */
/* | 1  1  0  0  1  0| 0  0  0| 0  0  0| 0  1  1  0| x  x  x  x| x  x  x  x  x  x  x  x  x  x  x  x| */
/* |-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------| */
/* */
                case 0x6:           /* SVC  none - none */  /* Supervisor Call Trap */
    {
#if 0
        int j;
        char n[9];
        uint32 dqe = M[0x8e8>>2];
        for (j=0; j<8; j++) {
            n[j] = (M[((dqe+0x18)>>2)+(j/4)] >> ((3-(j&7))*8)) & 0xff;
            if (n[j] == 0)
                n[j] = 0x20;
        }
        n[8] = 0;
#endif
                    addr = SPAD[0xf0];              /* get trap table memory address from SPAD (def 80) */
//fprintf(stderr, "SVC 1 IMM temp %.8x addr %.8x PSD1 %.8x flags %.8x\r\n", temp, addr, PSD1, i_flags);
                    if (addr == 0 || addr == 0xffffffff) {  /* see if secondary vector table set up */
                        TRAPME = ADDRSPEC_TRAP;     /* Not setup, error */
                        goto newpsd;                /* program error */
                    }
                    addr = addr + (0x06 << 2);      /* addr has mem addr of SVC trap vector (def 98) */
                    temp = M[addr >> 2];            /* get the secondary trap table address from memory */
//fprintf(stderr, "SVC 2 IMM temp %.8x addr %.8x PSD1 %.8x temp2 %.8x\r\n", temp, addr, PSD1, temp2);
                    if (temp == 0 || temp == 0xffffffff) {  /* see if ICB set up */
                        TRAPME = ADDRSPEC_TRAP;     /* Not setup, error */
                        goto newpsd;                /* program error */
                    }
                    temp2 = ((IR>>12) & 0x0f) << 2;     /* get SVC index from IR */
//fprintf(stderr, "SVC 2a IMM temp %.8x addr %.8x PSD1 %.8x temp2 %.8x\r\n", temp, addr, PSD1, temp2>>2);
                    t = M[(temp+temp2)>>2];         /* get secondary trap vector address ICB address */
                    if (t == 0 || t == 0xffffffff) {    /* see if ICB set up */
                        TRAPME = ADDRSPEC_TRAP;     /* Not setup, error */
                        goto newpsd;                /* program error */
                    }
//fprintf(stderr, "SVC 3 IMM temp %.8x svca %.8x PSD1 %.8x temp2 %.8x\r\n", temp+temp2, t, PSD1, temp2);
                    bc = PSD2 & 0x3ffc;             /* get copy of cpix */
                    M[t>>2] = (PSD1+4) & 0xfffffffe;    /* store PSD 1 + 1W to point to next instruction */
                    M[(t>>2)+1] = PSD2;             /* store PSD 2 */
                    PSD1 = M[(t>>2)+2];             /* get new PSD 1 */
                    PSD2 = (M[(t>>2)+3] & ~0x3ffc) | bc;    /* get new PSD 2 w/old cpix */
                    M[(t>>2)+4] = IR&0xFFF;         /* store call number */
#if 0
fprintf(stderr, "SVC @ %.8x SVC %x,%x PSD1 %.8x PSD2 %.8x SPAD PSD@ %x C.CURR %x LMN %s\r\n", OPSD1, temp2>>2, IR&0xFFF, PSD1, PSD2, SPAD[0xf5], dqe, n);
fprintf(stderr, " R0=%x R1=%x R2=%x R3=%x", GPR[0], GPR[1], GPR[2], GPR[3]);
fprintf(stderr, " R4=%x R5=%x R6=%x R7=%x", GPR[4], GPR[5], GPR[6], GPR[7]);
fprintf(stderr, "\r\n");
    if (((temp2>>2) == 1) && ((IR&0xfff) == 0x75))
fprintf(stderr, "SVC %x,%x GPR[6] %x GPR[6] %x\r\n", temp2>>2, IR&0xfff, GPR[6], GPR[7]);
#endif
                    /* set the mode bits and CCs from the new PSD */
                    CC = PSD1 & 0x78000000;         /* extract bits 1-4 from PSD1 */
                    modes = PSD1 & 0x87000000;      /* extract bits 0, 5, 6, 7 from PSD 1 */
                    /* set new map mode and interrupt blocking state in CPUSTATUS */
                    if (PSD2 & MAPBIT) {
                        CPUSTATUS |= 0x00800000;    /* set bit 8 of cpu status */
                        modes |= MAPMODE;           /* set mapped mode */
                    } else
                        CPUSTATUS &= 0xff7fffff;    /* reset bit 8 of cpu status */
                    /* set interrupt blocking state */
                    if ((PSD2 & 0x8000) == 0) {     /* is it retain blocking state */
                        if (PSD2 & 0x4000)          /* no, is it set blocking state */
                            CPUSTATUS |= 0x80;      /* yes, set blk state in cpu status bit 24 */
                        else {
                            CPUSTATUS &= ~0x80;     /* no, reset blk state in cpu status bit 24 */
                            irq_pend = 1;           /* start scanning interrupts again */
                        }
                    }
                    PSD2 &= ~0x0000c000;            /* clear bit 48 & 49 to be unblocked */
                    if (CPUSTATUS & 0x80)           /* see if old mode is blocked */
                        PSD2 |= 0x00004000;         /* set to blocked state */

                    PSD2 &= ~RETMBIT;               /* turn off retain bit in PSD2 */
                    SPAD[0xf5] = PSD2;              /* save the current PSD2 */
                    goto newpsd;                    /* new psd loaded */
    }
                    break;

                case 0x7:           /* EXR */
                    IR = temp;          /* get instruction to execute */
                    if (addr & 2)       /* if bit 30 set, instruction is in right hw, do EXRR */
                        IR <<= 16;      /* move instruction to left HW */
                    if ((IR & 0xFC7F0000) == 0xC8070000 ||
                        (IR & 0xFF800000) == 0xA8000000) {
                        /* Fault, attempt to execute another EXR, EXRR, or EXM  */
                        goto inv;           /* invalid instruction */
                    }
                    EXM_EXR = 4;            /* set PC increment for EXR */
                    goto exec;
                    break;

                /* these instruction were never used by MPX, only diags */
                case 0x8:           /* SEM */
                case 0x9:           /* LEM */
                case 0xA:           /* CEMA */
                case 0xB:           /* INV */
                case 0xC:           /* INV */
                case 0xD:           /* INV */
                case 0xE:           /* INV */
                case 0xF:           /* INV */
                    goto inv;       /* invalid instruction */
                    break;
                }
                break;

        case 0xCC>>2:               /* 0xCC ADR - ADR */ /* LF */
                /* For machines with Base mode 0xCC08 stores base registers */
                if ((FC & 3) != 0) {            /* must be word address */
                    TRAPME = ADDRSPEC_TRAP;     /* bad reg address, error */
                    goto newpsd;                /* go execute the trap now */
                }
                bc = addr & 0x20;               /* bit 26 initial value */
                while (reg < 8) {
                    if (bc != (addr & 0x20)) {  /* test for crossing file boundry */
                        TRAPME = ADDRSPEC_TRAP; /* bad reg address, error */
                        goto newpsd;            /* go execute the trap now */
                    }
                    if (FC & 0x4)               /* LFBR? */
                        TRAPME = Mem_read(addr, &BR[reg]);  /* read the base reg */
                    else                        /* LF */
                        TRAPME = Mem_read(addr, &GPR[reg]); /* read the GPR reg */
                    if (TRAPME)                 /* TRAPME has error */
                        goto newpsd;            /* go execute the trap now */
                    reg++;                      /* next reg to write */
                    addr += 4;                  /* next addr */
                }
                break;

       case 0xD0>>2:             /* 0xD0 SD|ADR - INV */ /* LEA  none basemode only */
//fprintf(stderr, "Got LEA @ addr %x\\r\n", addr);
                  dest = (t_uint64)(addr);
                  break;

        case 0xD4>>2:               /* 0xD4 SM|ADR - SM|ADR */ /* STx */
                break;

        case 0xD8>>2:               /* 0xD8 SM|ADR - SM|ADR */ /* STMx */
    //FIXME STMD needs both regs to be masked with R4
                if (dbl) {
                    /* we need to and both regs */
                    td = dest;
                    t_uint64 nm = (((t_uint64)GPR[4]) << 32) | (((t_uint64)GPR[4]) & D32RMASK);
                    dest &= nm;     /* mask both regs with reg 4 contents */
//      fprintf(stderr, "STMD PSD %x src %llx GPR[4] %llx result %llx\r\n",
//          PSD1, td, nm, dest);
                } else {
                    td = dest;
                    dest &= (((t_uint64)GPR[4]) & D32RMASK);        /* mask with reg 4 contents */
//      fprintf(stderr, "STMW PSD %x src %llx GPR[4] %llx result %llx\r\n",
//          PSD1, td, ((t_uint64)GPR[4]), dest);
                }           
                break;

        case 0xDC>>2:               /* 0xDC INV - */ /* INV nonbasemode (STFx basemode) */
                /* DC00 STF */ /* DC08 STFBR */
                if ((FC & 0x4) && (CPU_MODEL <= MODEL_27))  {
                    /* basemode undefined for 32/7x & 32/27 */ /* TODO check this */
                    TRAPME = UNDEFINSTR_TRAP;   /* Undefined Instruction Trap */
                    goto newpsd;                /* handle trap */
                }
                /* For machines with Base mode 0xDC08 stores base registers */
                if ((FC & 3) != 0) {            /* must be word address */
                    TRAPME = ADDRSPEC_TRAP;     /* bad reg address, error */
                    goto newpsd;                /* go execute the trap now */
                }
                bc = addr & 0x20;               /* bit 26 initial value */
                while (reg < 8) {
                    if (bc != (addr & 0x20)) {  /* test for crossing file boundry */
                        TRAPME = ADDRSPEC_TRAP; /* bad reg address, error */
                        goto newpsd;            /* go execute the trap now */
                    }
                    if (FC & 0x4)               /* STFBR? */
                        TRAPME = Mem_write(addr, &BR[reg]);     /* store the base reg */
                    else                        /* STF */
                        TRAPME = Mem_write(addr, &GPR[reg]);    /* store the GPR reg */
                    if (TRAPME)                 /* TRAPME has error */
                        goto newpsd;            /* go execute the trap now */
                    reg++;                      /* next reg to write */
                    addr += 4;                  /* next addr */
                }
                break;

        /* TODO */
        case 0xE0>>2:               /* 0xE0 SCC|SD|RM|ADR - SD|RM|ADR */ /* ADFx, SUFx */
                if ((FC & 3) != 0) {            /* must be word address */
                    TRAPME = ADDRSPEC_TRAP;     /* bad reg address, error */
                    goto newpsd;                /* go execute the trap now */
                }
                if (opr & 0x0008) {             /* Was it ADFx? */
                    /* TODO ADF? call here */
                    goto inv;
                } else {
                    /* TODO SUF? call here */
                    goto inv;
                }                               /* it is SUFx */
                break;

        /* TODO */
        case 0xE4>>2:               /* 0xE4 SCC|RM|ADR - RM|ADR */ /* MPFx, DVFx */
                if ((FC & 3) != 0) {            /* must be word address */
                    TRAPME = ADDRSPEC_TRAP;     /* bad reg address, error */
                    goto newpsd;                /* go execute the trap now */
                }
                if (opr & 0x0008) {             /* Was it MPFx? */
                    /* TODO MPF? call here */
                    goto inv;
                } else {
                    /* TODO DVF? call here */
                    goto inv;
                }                               /* it is DVFx */
                break;

        case 0xEC>>2:               /* 0xEC ADR - ADR */ /* Branch unconditional or Branch True */
                /* GOOF alert, the assembler sets bit 31 to 1 so this test will fail*/
                /* so just test for F bit and go on */
                /* if ((FC & 5) != 0) { */
                if ((FC & 4) != 0) {
                    TRAPME = ADDRSPEC_TRAP;     /* bad address, error */
                    goto newpsd;                /* go execute the trap now */
                }
                CC = PSD1 & 0x78000000;         /* get CC's if any */
                switch(reg) {
                case 0:     t = 1; break;
                case 1:     t = (CC & CC1BIT) != 0; break;
                case 2:     t = (CC & CC2BIT) != 0; break;
                case 3:     t = (CC & CC3BIT) != 0; break;
                case 4:     t = (CC & CC4BIT) != 0; break;
                case 5:     t = (CC & (CC2BIT|CC4BIT)) != 0; break;
                case 6:     t = (CC & (CC3BIT|CC4BIT)) != 0; break;
                case 7:     t = (CC & (CC1BIT|CC2BIT|CC3BIT|CC4BIT)) != 0; break;
                }
//fprintf(stderr, "BCT t %.8x addr %.8x CC %.8x PSD1 %.8x\r\n", t, addr, CC, PSD1);
                if (t) {                /* see if we are going to branch */
                    /* we are taking the branch, set CC's if indirect, else leave'm */
                    if (IR & IND)       /* see if CCs from last indirect location are wanted */
                        PSD1 = (PSD1 & 0x87fffffe) | CC;    /* insert last CCs */
                    /* update the PSD with new address */
                    PSD1 = (PSD1 & 0xff000000) | (addr & 0xfffffe); /* set new PC */
                    i_flags |= BT;      /* we branched, so no PC update */
//fprintf(stderr, "BR t %.8x addr %.8x PSD1 %.8x\r\n", t, addr, PSD1);
                }
                /* branch not taken, go do next instruction */
                break;

        case 0xF0>>2:               /* 0xF0 ADR - ADR */ /* Branch False or Branch Function True */
                /* GOOF alert, the assembler sets bit 31 to 1 so this test will fail*/
                /* so just test for F bit and go on */
                /* if ((FC & 5) != 0) { */
                if ((FC & 4) != 0) {
                    TRAPME = ADDRSPEC_TRAP;     /* bad address, error */
                    goto newpsd;                /* go execute the trap now */
                }
                CC = PSD1 & 0x78000000;         /* get CC's if any */
                switch(reg) {
                case 0:     t = (GPR[4] & (0x8000 >> ((CC >> 27) & 0xf))) != 0; break;
                case 1:     t = (CC & CC1BIT) == 0; break;
                case 2:     t = (CC & CC2BIT) == 0; break;
                case 3:     t = (CC & CC3BIT) == 0; break;
                case 4:     t = (CC & CC4BIT) == 0; break;
                case 5:     t = (CC & (CC2BIT|CC4BIT)) == 0; break;
                case 6:     t = (CC & (CC3BIT|CC4BIT)) == 0; break;
                case 7:     t = (CC & (CC1BIT|CC2BIT|CC3BIT|CC4BIT)) == 0; break;
                }
//fprintf(stderr, "BCF reg %d t %.8x addr %.8x CC %.8x PSD1 %.8x\r\n", reg, t, addr, CC, PSD1);
                if (t) {                    /* see if we are going to branch */
                    /* we are taking the branch, set CC's if indirect, else leave'm */
                    /* update the PSD with new address */
                    PSD1 = (PSD1 & 0xff000000) | (addr & 0xfffffe); /* set new PC */
                    i_flags |= BT;          /* we branched, so no PC update */
                    if (IR & IND)           /* see if CCs from last indirect location are wanted */
                        PSD1 = (PSD1 & 0x87fffffe) | CC;    /* insert last CCs */
//fprintf(stderr, "BR t %.8x addr %.8x PSD1 %.8x\r\n", t, addr, PSD1);
                }
                break;

        case 0xF4>>2:               /* 0xF4 RR|SD|ADR - RR|SB|WRD */ /* Branch increment */
                dest += 1 << ((IR >> 21) & 3);  /* use bits 9 & 10 to incr reg */
                if (dest != 0) {                /* if reg is not 0, take the branch */
                    /* we are taking the branch, set CC's if indirect, else leave'm */
                    /* update the PSD with new address */
#if 1
/* FIXME */         if (PC == (addr & 0x7FFFC)) {       /* BIB to current PC, bump branch addr */
                        addr += 4;
//                      fprintf(stderr, "BI? stopping BIB $ addr %x PC %x\r\n", addr, PC);
                    }
#endif
                    PSD1 = (PSD1 & 0xff000000) | (addr & 0xfffffe); /* set new PC */
                    if (IR & IND)       /* see if CCs from last indirect location are wanted */
                        PSD1 = (PSD1 & 0x87fffffe) | CC;    /* insert last CCs */
                    i_flags |= BT;              /* we branched, so no PC update */
                }
                break;

        case 0xF8>>2:               /* 0xF8 SM|ADR - SM|ADR */ /* ZMx, BL, BRI, LPSD, LPSDCM, TPR, TRP */
                switch((opr >> 7) & 0x7) {      /* use bits 6-8 to determine instruction */
                case 0x0:       /* ZMx F80x */  /* SM */
                    dest = 0;       /* destination value is zero */
                    i_flags |= SM;  /* SM not set so set it to store value */
                    break;
                case 0x1:       /* BL F880 */
                    /* copy CC's from instruction and PC incremented by 4 */
                    GPR[0] = ((PSD1 & 0x78000000) | (PSD1 & 0x7fffe)) + 4;
                    if (IR & IND)       /* see if CC from last indirect loacation are wanted */
                        GPR[0] = (CC | (PSD1 & 0x7fffe)) + 4;   /* set CC's and incremented PC */
                    /* update the PSD with new address */
                    PSD1 = (PSD1 & 0xff000000) | (addr & 0xfffffe);
                    i_flags |= BT;          /* we branched, so no PC update */
                    break;

                case 0x3:       /* LPSD F980 */
                    /* fall through */;
                case 0x5:       /* LPSDCM FA80 */
    {
#if 0
int j;
char n[9];
uint32 dqe = M[0x8e8>>2];
for (j=0; j<8; j++) {
    n[j] = (M[((dqe+0x18)>>2)+(j/4)] >> ((3-(j&7))*8)) & 0xff;
    if (n[j] == 0)
        n[j] = 0x20;
}
n[8] = 0;
#endif
                    CPUSTATUS |= 0x40;              /* enable software traps */
                                                    /* this will allow attn and */
                                                    /* power fail traps */
                    if ((TRAPME = Mem_read(addr, &PSD1))) { /* get PSD1 from memory */
                        goto newpsd;                /* memory read error or map fault */
                    }
                    if ((TRAPME = Mem_read(addr+4, &PSD2))) {   /* get PSD2 from memory */
                        goto newpsd;                /* memory read error or map fault */
                    }
                    /* set the mode bits and CCs from the new PSD */
                    CC = PSD1 & 0x78000000;         /* extract bits 1-4 from PSD1 */
                    modes = PSD1 & 0x87000000;      /* extract bits 0, 5, 6, 7 from PSD 1 */
                    /* set new map mode and interrupt blocking state in CPUSTATUS */
                    if (PSD2 & MAPBIT) {
                        CPUSTATUS |= 0x00800000;    /* set bit 8 of cpu status */
                        modes |= MAPMODE;           /* set mapped mode */
                    } else
                        CPUSTATUS &= 0xff7fffff;    /* reset bit 8 of cpu status */
                    /* set interrupt blocking state */
                    if ((PSD2 & 0x8000) == 0) {     /* is it retain blocking state */
                        if (PSD2 & 0x4000)          /* no, is it set blocking state */
                            CPUSTATUS |= 0x80;      /* yes, set blk state in cpu status bit 24 */
                        else {
                            CPUSTATUS &= ~0x80;     /* no, reset blk state in cpu status bit 24 */
                            irq_pend = 1;           /* start scanning interrupts again */
                        }
                    }
                    PSD2 &= ~0x0000c000;            /* clear bit 48 & 49 to be unblocked */
                    if (CPUSTATUS & 0x80)           /* see if old mode is blocked */
                        PSD2 |= 0x00004000;         /* set to blocked state */

                    /* TRY if cpix is zero, copy cpix from PSD2 in SPAD[0xf5] */
                    if ((PSD2 & 0x3fff) == 0)
                        PSD2 |= (SPAD[0xf5] & 0x3fff);  /* use new cpix */

                    if (opr & 0x0200) {             /* Was it LPSDCM? */
                        /* map bit must be on to load maps */
                        if (PSD2 & MAPBIT) {
#if 0
    // FIXME DEBUG EDIT
//  if (n[0] == 'E' && n[1] == 'D' && n[2] == 'I' && n[3] == 'T')
//      traceme = trstart;          /* start tracing */
traceme++; /* start trace */
//if (traceme >= trstart) {
fprintf(stderr, "LPSDCM #%d LOAD MAPS PSD1 %x PSD2 %x SPAD PSD2 %x CPUSTATUS %x C.CURR %x LMN %s\r\n", traceme, PSD1, PSD2, SPAD[0xf5], CPUSTATUS, dqe, n);
fprintf(stderr, " R0=%x R1=%x R2=%x R3=%x", GPR[0], GPR[1], GPR[2], GPR[3]);
fprintf(stderr, " R4=%x R5=%x R6=%x R7=%x", GPR[4], GPR[5], GPR[6], GPR[7]);
fprintf(stderr, "\r\n");
//reason = STOP_HALT;           /* do halt for now */
//}
#endif
                            /* set mapped mode in cpu status */
                            CPUSTATUS |= 0x00800000;    /* set bit 8 of cpu status */
                            /* we need to load the new maps */
                            TRAPME = load_maps(&PSD1);  /* load maps for new PSD */
                            PSD2 &= ~RETMBIT;           /* turn off retain bit in PSD2 */
                            SPAD[0xf5] = PSD2;          /* save the current PSD2 */
                            sim_debug(DEBUG_EXP, &cpu_dev,
                                "LPSDCM MAPS LOADED TRAPME = %x PSD1 %x PSD2 %x CPUSTATUS %x\n",
                                TRAPME, PSD1, PSD2, CPUSTATUS);
                        }
                        PSD2 &= ~RETMBIT;               /* turn off retain bit in PSD2 */
                    } else {
                        /* LPSD */
//traceme++; /* start trace */
#if 0
sim_debug(DEBUG_EXP, &cpu_dev, "LPSD PSD1 %x PSD2 %x CPUSTATUS %x\n", PSD1, PSD2, CPUSTATUS);
fprintf(stderr, "LPSD PSD1 %x PSD2 %x SPAD PSD2 %x CPUSTATUS %x C.CURR %x LMN %s\r\n",
                PSD1, PSD2, SPAD[0xf5], CPUSTATUS, dqe, n);
fprintf(stderr, " R0=%x R1=%x R2=%x R3=%x", GPR[0], GPR[1], GPR[2], GPR[3]);
fprintf(stderr, " R4=%x R5=%x R6=%x R7=%x", GPR[4], GPR[5], GPR[6], GPR[7]);
fprintf(stderr, "\r\n");
#endif
                    }
}
                    /* TRAPME can be error from LPSDCM or OK here */
                    skipinstr = 1;                  /* skip next interrupt test only once */
                    goto newpsd;                    /* load the new psd */
                    break;

                case 0x4:   /* JWCS */  /* not used in simulator */
                case 0x2:   /* BRI */   /* TODO - only for 32/55 or 32/7X in PSW mode */
                case 0x6:   /* TRP */
                case 0x7:   /* TPR */
                    TRAPME = UNDEFINSTR_TRAP;   /* trap condition */
                    goto newpsd;                /* undefined instruction trap */
                    break;
                }
                break;

/* F Class I/O device instruction format */
/* |-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------| */
/* |00 01 02 03 04 05|06 07 08|09 10 11 12|13 14 15|16|17 18 19 20 21 22 23|24 25 26 27 28 29 30 31| */
/* |     Op Code     |  Reg   |  I/O type |  Aug   |0 |   Channel Address  |  Device Sub-address   | */
/* |-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------| */
/* */
/* E Class I/O device instruction format */
/* |-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------| */
/* |00 01 02 03 04 05|06 07 08 09 10 11 12|13 14 15|16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31| */
/* |     Op Code     |     Device Number  |  Aug   |                  Command Code                 | */
/* |-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------| */
/* */
        case 0xFC>>2:               /* 0xFC IMM - IMM */ /* XIO, CD, TD, Interrupt Control */
                if ((modes & PRIVBIT) == 0) {       /* must be privileged to do I/O */
                    TRAPME = PRIVVIOL_TRAP;     /* set the trap to take */
                    TRAPSTATUS |= 0x1000;       /* set Bit 19 of Trap Status word */
                    goto newpsd;                /* Privlege violation trap */
                }
                if ((opr & 0x7) != 0x07) {      /* aug is 111 for XIO instruction */
                    /* Process Non-XIO instructions */
                    uint32 status = 0;                  /* status returned from device */
                    uint32 device = (opr >> 3) & 0x7f;  /* get device code */
                    uint32 prior = device;              /* interrupt priority */
                    t = SPAD[prior+0x80];               /* get spad entry for interrupt */
                    addr = SPAD[0xf1] + (prior<<2);     /* vector address in SPAD */
                    addr = M[addr>>2];                  /* get the interrupt context block addr */
//fprintf(stderr, "CD/TD etc device %x intr %x spad %.8x addr %x\r\n", device, prior, t, addr);

                    switch(opr & 0x7) {             /* use bits 13-15 to determine instruction */
                    case 0x0:       /* EI  FC00  Enable Interrupt */
                        prior = (opr >> 3) & 0x7f;  /* get priority level */
                        /* SPAD entries for interrupts begin at 0x80 */
                        t = SPAD[prior+0x80];       /* get spad entry for interrupt */
                        if (t == 0 || t == 0xffffffff)  /* if not set up, die */
                            goto syscheck;          /* system check */
                        INTS[prior] |= INTS_ENAB;   /* enable specified int level */
                        SPAD[prior+0x80] |= SINT_ENAB;  /* enable in SPAD too */
                        irq_pend = 1;               /* start scanning interrupts again */
//fprintf(stderr, "EIO EI intr %.2x SPAD %.8x\r\n", prior, t);
                        if (prior == 0x18)          /* is this the clock starting */
                            rtc_setup(1, prior);    /* tell clock to start */
                        if (prior == 0x5f)          /* is this the initerval timer starting */
                            itm_setup(1, prior);    /* tell timer to start */
                        break;

                    case 0x1:       /* DI FC01 */
                        prior = (opr >> 3) & 0x7f;  /* get priority level */
                        /* SPAD entries for interrupts begin at 0x80 */
                        t = SPAD[prior+0x80];       /* get spad entry for interrupt */
                        if (t == 0 || t == 0xffffffff)  /* if not set up, die */
                            goto syscheck;          /* system check */
                        INTS[prior] &= ~INTS_ENAB;  /* disable specified int level */
                        INTS[prior] &= ~INTS_REQ;   /* clears any requests also */
                        SPAD[prior+0x80] &= ~SINT_ENAB; /* disable in SPAD too */
//fprintf(stderr, "EIO DI intr %.2x SPAD %.8x\r\n", prior, t);
                        if (prior == 0x18)          /* is this the clock stopping */
                            rtc_setup(0, prior);    /* tell clock to stop */
                        if (prior == 0x5f)          /* is this the initerval timer stopping */
                            itm_setup(0, prior);    /* tell timer to stop */
                        break;

                    case 0x2:       /* RI FC02 */
                        prior = (opr >> 3) & 0x7f;  /* get priority level */
                        /* SPAD entries for interrupts begin at 0x80 */
                        t = SPAD[prior+0x80];       /* get spad entry for interrupt */
                        if (t == 0 || t == 0xffffffff)  /* if not set up, die */
                            goto syscheck;          /* system check */
                        INTS[prior] |= INTS_REQ;    /* set the request flag for this level */
                        irq_pend = 1;               /* start scanning interrupts again */
//fprintf(stderr, "EIO RI intr %.2x SPAD %.8x\r\n", prior, t);
                        break;

                    case 0x3:       /* AI FC03 */
                        prior = (opr >> 3) & 0x7f;  /* get priority level */
                        /* SPAD entries for interrupts begin at 0x80 */
                        t = SPAD[prior+0x80];       /* get spad entry for interrupt */
                        if (t == 0 || t == 0xffffffff)  /* if not set up, die */
                            goto syscheck;          /* system check */
                        INTS[prior] |= INTS_ACT;    /* activate specified int level */
//fprintf(stderr, "EIO AI intr %.2x SPAD %.8x\r\n", prior, t);
                        break;

                    case 0x4:       /* DAI FC04 */
                        prior = (opr >> 3) & 0x7f;  /* get priority level */
                        /* SPAD entries for interrupts begin at 0x80 */
                        t = SPAD[prior+0x80];       /* get spad entry for interrupt */
                        if (t == 0 || t == 0xffffffff)  /* if not set up, die */
                            goto syscheck;          /* system check */
                        INTS[prior] &= ~INTS_ACT;   /* deactivate specified int level */
                        SPAD[prior+0x80] &= ~SINT_ACT;  /* deactivate in SPAD too */
//fprintf(stderr, "EIO DAI intr %.2x SPAD %.8x INTS[%x] %x\r\n", prior, t, prior, INTS[prior]);
                        irq_pend = 1;               /* start scanning interrupts again */
                        skipinstr = 1;              /* skip interrupt test */
                        /* instruction following a DAI can not be interrupted */
                        /* skip tests for interrupts if this is the case */
                        break;

                    case 0x5:       /* TD FC05 */   /* bits 13-15 is test code type */
                    case 0x6:       /* CD FC06 */
                        /* If CD or TD, make sure device is not F class device */
                        /* the channel must be defined as a non class F I/O channel in SPAD */
                        /* if class F, the system will generate a system check trap */
                        t = SPAD[device];               /* get spad entry for channel */
                        if (t == 0 || t == 0xffffffff)  /* if not set up, die */
                            goto syscheck;              /* system check */
//fprintf(stderr, "EIO TD/CD chan %.4x spad %.8x\r\n", device, t);
                        if ((t & 0x0f000000) == 0x0f000000) {   /* class in bits 4-7 */
syscheck:
                            TRAPME = SYSTEMCHK_TRAP;        /* trap condition if F class */
                            TRAPSTATUS &= ~BIT0;            /* class E error bit */
                            TRAPSTATUS &= ~BIT1;            /* I/O processing error */
                            goto newpsd;                    /* machine check trap */
                        }
                        if (opr & 0x1) {                    /* see if CD or TD */
                            /* process a TD */
//                          if ((TRAPME = testEIO(device, testcode, &status)))
//                              goto newpsd;                /* error returned, trap cpu */
                            /* return status has new CC's in bits 1-4 of status word */
                            PSD1 = ((PSD1 & 0x87fffffe) | (status & 0x78000000));   /* insert status CCs */
//fprintf(stderr, "EIO TD chan %.4x spad %.8x\r\n", device, t);
                            goto inv;                   /* invalid instruction until I fix it */
                        } else {
                            /* process a CD */
//                          if ((TRAPME = startEIO(device, &status)))
//                              goto newpsd;                /* error returned, trap cpu */
//                          t = SPAD[device];               /* get spad entry for channel */
                            /* t has spad entry for device */
                            /* get the 1's comp of interrupt address from bits 9-15 SPAD entry */
                            ix = (t & 0x007f0000) >> 16;    /* get the 1's comp of int level */
//fprintf(stderr, "XIO1 chan %8x intr %8x spad %.8x\r\n", chan, ix, bc);
                            ix = 127 - ix;                  /* get positive number for interrupt */
//fprintf(stderr, "XIO2 chan %x intr %x spad %.8x\r\n", chan, ix, bc);
                            temp = (IR & 0x7f);             /* get cmd from instruction */
                            if (device == 0x7f) {
//fprintf(stderr, "Interval Timer CD call cmd %x GPR[0] %x intr %x\r\n", temp, GPR[0], ix);
                                status = itm_rdwr(temp, GPR[0], ix);    /* read/write the interval timer */
//fprintf(stderr, "Interval Timer CD return status %x cmd %x GPR[0] %x intr %x\r\n", status, temp, GPR[0], ix);
                                if (temp != 0x39)           /* this cmd does not return value */
                                    GPR[0] = status;        /* return count in reg 0 */
                                /* No CC's going out */
                            } else {
                                goto inv;                   /* invalid instruction until I fix it */
                            }
                        }
                        break;
                    case 0x7:       /* XIO FC07*/   /* should never get here */
                        break;
                    }
                    break;          /* skip over XIO code */
                }

                /* Process XIO instructions */
                /* if reg is non-zero, add reg to 15 bits from instruction */
                if (reg)
                    temp2 = (IR & 0x7fff) + (GPR[reg] & 0x7fff);    /* set new chan/suba into IR */
                else
                    temp2 = (IR & 0x7fff);          /* set new chan/suba into IR */
                lchan = (temp2 & 0x7F00) >> 8;      /* get 7 bit logical channel address */
                suba = temp2 & 0xFF;                /* get 8 bit subaddress */
                /* the channel must be defined as a class F I/O channel in SPAD */
                /* if not class F, the system will generate a system check trap */
                t = SPAD[lchan];                    /* get spad entry for channel */
                if (t == 0 || t == 0xffffffff)      /* if not set up, die */
                    goto syscheck;                  /* machine check */
//fprintf(stderr, "XIO chan %x sa %x spad %.8x\r\n", chan, suba, t);
//sim_debug(DEBUG_EXP, &cpu_dev, "$$ XIO lchan %x sa %x spad %.8x\n", lchan, suba, t);
                if ((t & 0x0f000000) != 0x0f000000) {   /* class in bits 4-7 */
mcheck:
                    TRAPME = MACHINECHK_TRAP;       /* trap condition */
                    TRAPSTATUS |= BIT0;             /* class F error bit */
                    TRAPSTATUS &= ~BIT1;            /* I/O processing error */
                    goto newpsd;                    /* machine check trap */
                }
                /* get real channel from spad device entry */
                chan = (t & 0x7f00) >> 8;           /* real channel */
                /* get the 1's comp of interrupt address from bits 9-15 SPAD entry */
                ix = (t & 0x007f0000) >> 16;        /* get the 1's comp of int level */
//fprintf(stderr, "XIO1 chan %8x intr %8x spad %.8x\r\n", chan, ix, bc);
                ix = 127 - ix;                  /* get positive number for interrupt */
//fprintf(stderr, "XIO2 chan %x intr %x spad %.8x\r\n", chan, ix, bc);
                bc = SPAD[ix+0x80];             /* get interrupt entry for channel */
//fprintf(stderr, "XIO chan %x intr %x spad %.8x\r\n", chan, ix, bc);
                /* SPAD address F1 has interrupt table address */
                addr = SPAD[0xf1] + (ix<<2);    /* vector address in SPAD */
//fprintf(stderr, "XIOa spad %x intr %x spad %.8x addr %x\r\n", SPAD[0xf1], ix, bc, addr);
                addr = M[addr>>2];              /* get the interrupt context block addr */
//fprintf(stderr, "XIOb chan %x intr %x spad %.8x addr %x\r\n", chan, ix, bc, addr);
                                                /* the context block contains the old PSD, */
                                                /* new PSD, IOCL address, and I/O status address */
                if (addr == 0)                  /* must be initialized address */
                    goto mcheck;                /* bad int icb address */
//fprintf(stderr, "XIO chan %x intr %x addr %x iocla %x\r\n", chan, ix, addr, addr + 16);
                if ((TRAPME = Mem_read(addr+16, &temp))) {  /* get iocl address from icb wd 4 */
                    goto mcheck;                /* machine check if not there */
                }
//fprintf(stderr, "XIOx chan %x intr %x addr %x temp %x\r\n", chan, ix, addr, temp);
                /* iocla must be valid addr if it is a SIO instruction */
                if (((temp & MASK24) == 0) && (((opr >> 2) & 0xf) == 2)) 
                    goto mcheck;                /* bad iocl address */

//fprintf(stderr, "XIO ready chan %x intr %x icb %x iocla %x iocd1 %.8x iocd2 %0.x8\r\n",
//      chan, ix, addr, addr+16, M[temp>>2], M[(temp+4)>>2]);
//sim_debug(DEBUG_EXP, &cpu_dev, "XIO ready chan %x intr %x icb %x iocla %x iocd1 %.8x iocd2 %.8x\n",
//      chan, ix, addr, addr+16, M[temp>>2], M[(temp+4)>>2]);
                /* at this point, the channel has a valid SPAD channel entry */
                /* t is SPAD entry contents for chan device */
                /* temp2 has IR + reg contents if reg != 0 */
                /* lchan - logical channel address */
                /* chan - channel address */
                /* suba - channel device subaddress */
                /* ix - positive interrupt level */
                /* addr - ICB for specified interrupt level, points to 6 wd block */
                /* temp - First IOCD address */
//fprintf(stderr, "XIO switch %x lchan %x chan %x intr %x chsa %x IOCDa %x\r\n", ((opr>>3)&0xf), lchan, chan, ix, (chan<<8)|suba, temp);
//sim_debug(DEBUG_EXP, &cpu_dev, "XIO switch chan %x intr %x chsa %x IOCDa %.8x\n", chan, ix, (chan<<8)|suba, temp);
                switch((opr >> 3) & 0xf) {          /* use bits 9-12 to determine I/O instruction */
                    uint32 status;                  /* status returned by various functions */
                    uint16 chsa;                    /* logical device address */

                    case 0x00:                      /* Unassigned */
                    case 0x01:                      /* Unassigned */
                    case 0x0A:                      /* Unassigned */
                        TRAPME = UNDEFINSTR_TRAP;   /* trap condition */
                        goto newpsd;                /* undefined instruction trap */
                        break;

                    case 0x09:                      /* Enable write channel ECWCS */
                    case 0x0B:                      /* Write channel WCS WCWCS */
                        /* for now or maybe forever, return unsupported transaction */
                        PSD1 = ((PSD1 & 0x87fffffe) | (CC2BIT|CC4BIT)); /* insert status 5 */
                        TRAPME = UNDEFINSTR_TRAP;   /* trap condition */
                        goto newpsd;                /* undefined instruction trap */
                        break;

                    case 0x02:      /* Start I/O SIO */
                        chsa = temp2 & 0x7FFF;      /* get logical device address */
                        if ((TRAPME = startxio(chsa, &status)))
                            goto newpsd;            /* error returned, trap cpu */
                        PSD1 = ((PSD1 & 0x87fffffe) | (status & 0x78000000));   /* insert status */
//fprintf(stderr, "@@XIO SIO ret chan %x chsa %x status %x\r\n", chan, (chan<<8)|suba, status);
sim_debug(DEBUG_EXP, &cpu_dev, "XIO SIO ret chan %x chsa %x status %x\n", chan, (chan<<8)|suba, status);
                        break;
                            
                    case 0x03:      /* Test I/O TIO */
                        chsa = temp2 & 0x7FFF;      /* get logical device address */
                        if ((TRAPME = testxio(chsa, &status)))
                            goto newpsd;            /* error returned, trap cpu */
                        PSD1 = ((PSD1 & 0x87fffffe) | (status & 0x78000000));   /* insert status */
//fprintf(stderr, "@@XIO TIO ret chan %x chsa %x status %x\r\n", chan, (chan<<8)|suba, status);
sim_debug(DEBUG_EXP, &cpu_dev, "XIO TIO ret chan %x chsa %x status %x\n", chan, (chan<<8)|suba, status);
                        break;
                            
                    case 0x04:      /* Stop I/O STPIO */
                        chsa = temp2 & 0x7FFF;      /* get logical device address */
                        if ((TRAPME = stopxio(chsa, &status)))
                            goto newpsd;            /* error returned, trap cpu */
                        /* SPAD entries for interrupts begin at 0x80 */
/* 07-16-18 chg */      INTS[ix] &= ~INTS_REQ;      /* clears any requests also */
                        INTS[ix] &= ~INTS_ACT;      /* deactivate specified int level */
                        SPAD[ix+0x80] &= ~SINT_ACT; /* deactivate in SPAD too */
                        PSD1 = ((PSD1 & 0x87fffffe) | (status & 0x78000000));   /* insert status */
//fprintf(stderr, "@@XIO STPIO ret chan %x chsa %x status %x\r\n", chan, (chan<<8)|suba, status);
sim_debug(DEBUG_EXP, &cpu_dev, "XIO STPIO ret chan %x chsa %x status %x\n", chan, (chan<<8)|suba, status);
                        break;

                    /* TODO Finish XIO */
                    case 0x05:      /* Reset channel RSCHNL */
                        chsa = temp2 & 0x7FFF;      /* get logical device address */
                        /* SPAD entries for interrupts begin at 0x80 */
                        INTS[ix] &= ~INTS_REQ;      /* clears any requests */
                        INTS[ix] &= ~INTS_ACT;      /* deactivate specified int level */
                        SPAD[ix+0x80] &= ~SINT_ACT; /* deactivate in SPAD too */
//fprintf(stderr, "RSCHNL intr %.2x SPAD %.8x INTS[%x] %x\r\n", ix, bc, ix, INTS[ix]);
                        if ((TRAPME = rschnlxio(chsa, &status)))
                            goto newpsd;            /* error returned, trap cpu */
                        PSD1 = ((PSD1 & 0x87fffffe) | (status & 0x78000000));   /* insert status */
//fprintf(stderr, "XIO RSCHNL ret chan %x chsa %x status %x\r\n", chan, (chan<<8)|suba, status);
sim_debug(DEBUG_EXP, &cpu_dev, "XIO RSCHNL ret chan %x chsa %x status %x\n", chan, (chan<<8)|suba, status);
                        break;

                    case 0x06:      /* Halt I/O HIO */
                        chsa = temp2 & 0x7FFF;      /* get logical device address */
                        if ((TRAPME = haltxio(chsa, &status)))
                            goto newpsd;            /* error returned, trap cpu */
                        PSD1 = ((PSD1 & 0x87fffffe) | (status & 0x78000000));   /* insert status */
//fprintf(stderr, "@@XIO HALTXIO ret chan %x chsa %x status %x\r\n", chan, (chan<<8)|suba, status);
sim_debug(DEBUG_EXP, &cpu_dev, "HIO HALTXIO ret chan %x chsa %x status %x\n", chan, (chan<<8)|suba, status);
                        break;

                    case 0x07:      /* Grab controller GRIO n/u */
                        chsa = temp2 & 0x7FFF;      /* get logical device address */
                        if ((TRAPME = grabxio(chsa, &status)))
                            goto newpsd;            /* error returned, trap cpu */
                        PSD1 = ((PSD1 & 0x87fffffe) | (status & 0x78000000));   /* insert status */
//fprintf(stderr, "XIO GRIO ret chan %x chsa %x status %x\r\n", chan, (chan<<8)|suba, status);
sim_debug(DEBUG_EXP, &cpu_dev, "XIO GRIO ret chan %x chsa %x status %x\n", chan, (chan<<8)|suba, status);
                        break;

                    case 0x08:      /* Reset controller RSCTL */
                        chsa = temp2 & 0x7FFF;      /* get logical device address */
                        if ((TRAPME = stopxio(chsa, &status)))
                            goto newpsd;            /* error returned, trap cpu */
                        PSD1 = ((PSD1 & 0x87fffffe) | (status & 0x78000000));   /* insert status */
//fprintf(stderr, "XIO STPIO ret chan %x chsa %x status %x\r\n", chan, (chan<<8)|suba, status);
sim_debug(DEBUG_EXP, &cpu_dev, "XIO RSCTL ret chan %x chsa %x status %x\n", chan, (chan<<8)|suba, status);
                        break;

                    /* TODO Finish XIO interrupts */
                    case 0x0C:      /* Enable channel interrupt ECI */
                        chsa = temp2 & 0x7FFF;      /* get logical device address */
//fprintf(stderr, "XIO ECI chan %x sa %x spad %.8x INTS %x SPAD %x\r\n", chan, suba, t, INTS[ix], SPAD[ix+0x80]);
sim_debug(DEBUG_EXP, &cpu_dev, "XIO ECI chan %x sa %x spad %.8x\n", chan, suba, t);
                        /* SPAD entries for interrupts begin at 0x80 */
                        INTS[ix] |= INTS_ENAB;      /* enable specified int level */
                        SPAD[ix+0x80] |= SINT_ENAB; /* enable in SPAD too */
                        INTS[ix] &= ~INTS_REQ;      /* clears any requests also TRY 06-09-18 */
//TRY 06-09-18                      irq_pend = 1;               /* start scanning interrupts again */
//fprintf(stderr, "ECI intr %.2x SPAD %.8x INTS[%x] %x\r\n", ix, bc, ix, INTS[ix]);
                        break;

                    case 0x0D:      /* Disable channel interrupt DCI */
                        chsa = temp2 & 0x7FFF;      /* get logical device address */
sim_debug(DEBUG_EXP, &cpu_dev, "XIO DCI chan %x sa %x spad %.8x\n", chan, suba, t);
                        /* SPAD entries for interrupts begin at 0x80 */
                        INTS[ix] &= ~INTS_ENAB;     /* disable specified int level */
                        INTS[ix] &= ~INTS_REQ;      /* clears any requests also */
                        SPAD[ix+0x80] &= ~SINT_ENAB;    /* disable in SPAD too */
//fprintf(stderr, "DCI intr %.2x SPAD %.8x INTS[%x] %x\r\n", ix, bc, ix, INTS[ix]);
                        break;

                    case 0x0E:      /* Activate channel interrupt ACI */
                        chsa = temp2 & 0x7FFF;      /* get logical device address */
sim_debug(DEBUG_EXP, &cpu_dev, "XIO ACI chan %x sa %x spad %.8x\n", chan, suba, t);
                        /* SPAD entries for interrupts begin at 0x80 */
                        INTS[ix] |= INTS_ACT;   /* activate specified int level */
//fprintf(stderr, "ACI intr %.2x SPAD %.8x INTS[%x] %x\r\n", ix, bc, ix, INTS[ix]);
                        break;

                    case 0x0F:      /* Deactivate channel interrupt DACI */
                                    /* Note, instruction following DACI is not interruptable */
                        chsa = temp2 & 0x7FFF;      /* get logical device address */
sim_debug(DEBUG_EXP, &cpu_dev, "XIO DACI chan %x sa %x spad %.8x\n", chan, suba, t);
                        /* SPAD entries for interrupts begin at 0x80 */
/* 07-16-18 chg */      INTS[ix] &= ~INTS_REQ;      /* clears any requests also */
                        INTS[ix] &= ~INTS_ACT;      /* deactivate specified int level */
                        SPAD[ix+0x80] &= ~SINT_ACT; /* deactivate in SPAD too */
//fprintf(stderr, "DACI intr %.2x SPAD %.8x INTS[%x] %x\r\n", ix, bc, ix, INTS[ix]);
                        irq_pend = 1;               /* start scanning interrupts again */
                        skipinstr = 1;              /* skip interrupt test */
//NOTE CC must be returned */
                        break;
                }                   /* end of XIO switch */
                break;
        }                   /* End of Instruction Switch */

        /* any instruction with an arithmetic exception will still end up here */
        /* after the instruction is done and before incrementing the PC, */
        /* we will trap the cpu if ovl is set nonzero by an instruction */

        /* Store result to register */
        if (i_flags & SD) {
            if (dbl) {                  /* if double reg, store 2nd reg */
                GPR[reg+1] = (uint32)(dest & FMASK);    /* save the low order reg */
                GPR[reg] = (uint32)((dest>>32) & FMASK);/* save the hi order reg */
            } else {
                GPR[reg] = (uint32)(dest & FMASK);      /* save the reg */
            }
        }

        /* Store result to base register */
        if (i_flags & SB) {
            if (dbl)  {                     /* no dbl wd store to base regs */
                TRAPME = ADDRSPEC_TRAP;     /* bad address, error */
                goto newpsd;                /* go execute the trap now */
            }
            BR[reg] = (uint32)(dest & FMASK);   /* save the base reg */
//fprintf(stderr, "SB base reg %x dest(BR) %.8x\r\n", reg, BR[reg]);
        }

        /* Store result to memory */
        if (i_flags & SM) {
            /* Check if byte of half word */
            if (((FC & 04) || (FC & 5) == 1)) { /* hw or byte requires read first */
                if ((TRAPME = Mem_read(addr, &temp))) { /* get the word from memory */
                    goto newpsd;                /* memory read error or map fault */
                }
            }
            switch(FC) {
            case 2:         /* double word store */
                if ((addr & 2) != 2) {
                    TRAPME = ADDRSPEC_TRAP;     /* address not on dbl wd boundry, error */
                    goto newpsd;                /* go execute the trap now */
                }
                temp = (uint32)(dest & MASK32);/* get lo 32 bit */
                if ((TRAPME = Mem_write(addr + 4, &temp)))
                    goto newpsd;                /* memory write error or map fault */
                temp = (uint32)(dest >> 32);    /* move upper 32 bits to lo 32 bits */
                break;

            case 0:     /* word store */
                temp = (uint32)(dest & FMASK);  /* mask 32 bit of reg */
                if ((addr & 3) != 0) {
                    /* Address fault */
                    TRAPME = ADDRSPEC_TRAP;     /* address not on wd boundry, error */
                    goto newpsd;                /* go execute the trap now */
                }
                break;

            case 1:     /* left halfword write */
                temp &= RMASK;                  /* mask out 16 left most bits */
                temp |= (uint32)(dest & RMASK) << 16;   /* put into left most 16 bits */
                if ((addr & 1) != 1) {
                    /* Address fault */
                    TRAPME = ADDRSPEC_TRAP;     /* address not on hw boundry, error */
                    goto newpsd;                /* go execute the trap now */
                }
                break;

            case 3:     /* right halfword write */
                temp &= LMASK;                  /* mask out 16 right most bits */
                temp |= (uint32)(dest & RMASK); /* put into right most 16 bits */
                if ((addr & 3) != 3) {
                    TRAPME = ADDRSPEC_TRAP;     /* address not on hw boundry, error */
                    goto newpsd;                /* go execute the trap now */
                }
                break;

            case 4:
            case 5:
            case 6:
            case 7:     /* byte store operation */
                temp &= ~(0xFF << (8 * (7 - FC)));  /* clear the byte to store */
                temp |= (uint32)(dest & 0xFF) << (8 * (7 - FC));    /* insert new byte */
                break;
            }
            /* store back the modified memory location */
            if ((TRAPME = Mem_write(addr, &temp)))  /* store back to memory */
                goto newpsd;        /* memory write error or map fault */
        }

        /* Update condition code registers */
        if (i_flags & SCC) {
            PSD1 &= 0x87FFFFFe;     /* clear the old CC's */
            if (ovr)                /* if overflow, set CC1 */
                CC |= CC1BIT;       /* show we had AEXP */
            else
                CC = 0;             /* no CC's yet */   
            if (dest & MSIGN)       /* is it neg */
                CC |= CC3BIT;       /* if neg, set CC3 */
            else if (dest == 0)
                CC |= CC4BIT;       /* if zero, set CC4 */
            else
                CC |= CC2BIT;       /* if gtr than zero, set CC2 */
            PSD1 |= CC & 0x78000000;    /* update the CC's in the PSD */
        }

        /* Update instruction pointer to next instruction */
        if (EXM_EXR != 0) {             /* special handling for EXM, EXR, EXRR */
                PSD1 = (PSD1 + 4) | (((PSD1 & 2) >> 1) & 1);
                EXM_EXR = 0;            /* reset PC increment for EXR */
        } else
        if ((i_flags & BT) == 0) {      /* see if PSD was replaced on a branch instruction */
//fprintf(stderr, "@PCI1 temp %.8x addr %.8x PSD1 %.8x flags %.8x\r\n", temp, addr, PSD1, i_flags);
            /* branch not taken, so update the PC */
            if (i_flags & HLF) {
                PSD1 = (PSD1 + 2) | (((PSD1 & 2) >> 1) & 1);
            } else {
                PSD1 = (PSD1 + 4) | (((PSD1 & 2) >> 1) & 1);
            }
//fprintf(stderr, "@PCI2 temp %.8x addr %.8x PSD1 %.8x flags %.8x\r\n", temp, addr, PSD1, i_flags);
        }

        /* check if we had an arithmetic exception on the last instruction*/
        if (ovr && (modes & AEXPBIT)) {
            TRAPME = AEXPCEPT_TRAP;     /* trap the system now */
            goto newpsd;                /* process the trap */
        }
#if 0
        /* no trap, so continue with next instruction */
//PC = PSD1 & 0xfffffe;         /* get 24 bit addr from PSD1 */
// Get CCs from PSD1 and PC from original instruction */
//if (traceme >= 4) {
if (traceme >= trstart) {
OPSD1 &= 0x87FFFFFE;            /* clear the old CC's */
OPSD1 |= PSD1 & 0x78000000;     /* update the CC's in the PSD */
if (modes & MAPMODE)
    fprintf(stderr, "M%.8x %.8x ", OPSD1, OIR);
else
    fprintf(stderr, "U%.8x %.8x ", OPSD1, OIR);
fprint_inst(stderr, OIR, 0);    /* display instruction */
fprintf(stderr, " R0=%x R1=%x R2=%x R3=%x", GPR[0], GPR[1], GPR[2], GPR[3]);
fprintf(stderr, " R4=%x R5=%x R6=%x R7=%x", GPR[4], GPR[5], GPR[6], GPR[7]);
fprintf(stderr, "\r\n");
}
#endif
sim_debug(DEBUG_DATA, &cpu_dev, "R0=%08x R1=%08x R2=%08x R3=%08x\n", GPR[0], GPR[1], GPR[2], GPR[3]);
sim_debug(DEBUG_DATA, &cpu_dev, "R4=%08x R5=%08x R6=%08x R7=%08x\n", GPR[4], GPR[5], GPR[6], GPR[7]);
        continue;   /* keep running */
//      break;      /* quit for now after each instruction */

newpsd:
//fprintf(stderr, "place @ newpsd PSD1 %.8x PSD2 %.8x TRAPME %.4x\r\n", PSD1, PSD2, TRAPME);
       /* we get here from a LPSD, LPSDCM, INTR, or TRAP */
        if (TRAPME) {
            /* SPAD location 0xf0 has trap vector base address */
            uint32 tta = SPAD[0xf0];    /* get trap table address in memory */
            uint32 tvl;                 /* trap vector location */
            if (tta == 0 || tta == 0xffffffff)
                tta = 0x80;             /* if not set, assume 0x80 FIXME */
            /* Trap Table Address in memory is pointed to by SPAD 0xF0 */
            /* TODO update cpu status and trap status words with reason too */
            switch(TRAPME) {
            case POWERFAIL_TRAP:        /* 0x80 power fail trap */
            case POWERON_TRAP:          /* 0x84 Power-On trap */
            case MEMPARITY_TRAP:        /* 0x88 Memory Parity Error trap */
            case NONPRESMEM_TRAP:       /* 0x8C Non Present Memory trap */
            case UNDEFINSTR_TRAP:       /* 0x90 Undefined Instruction Trap */
            case PRIVVIOL_TRAP:         /* 0x94 Privlege Violation Trap */
//TODO      case SVCCALL_TRAP:          /* 0x98 Supervisor Call Trap */
            case MACHINECHK_TRAP:       /* 0x9C Machine Check Trap */
            case SYSTEMCHK_TRAP:        /* 0xA0 System Check Trap */
            case MAPFAULT_TRAP:         /* 0xA4 Map Fault Trap */
            case IPUUNDEFI_TRAP:        /* 0xA8 IPU Undefined Instruction Trap */
            case SIGNALIPU_TRAP:        /* 0xAC Signal IPU/CPU Trap */
            case ADDRSPEC_TRAP:         /* 0xB0 Address Specification Trap */
            case CONSOLEATN_TRAP:       /* 0xB4 Console Attention Trap */
            case PRIVHALT_TRAP:         /* 0xB8 Privlege Mode Halt Trap */
            case AEXPCEPT_TRAP:         /* 0xBC Arithmetic Exception Trap */
            default:
                tta = tta + (TRAPME - 0x80);        /* tta has mem addr of trap vector */
                tvl = M[tta>>2] & 0x7FFFC;          /* get trap vector address from trap vector loc */
//fprintf(stderr, "tvl %.8x, tta %.8x status %.8x\r\n", tvl, tta, CPUSTATUS);
                if (tvl == 0 || tvl == 0x7FFFC || (CPUSTATUS & 0x40) == 0) {
                    /* vector is zero or software has not enabled traps yet */
                    /* execute a trap halt */
                    /* set the PSD to trap vector location */
                    PSD1 = 0x80000000 + TRAPME;     /* just priv and PC to trap vector */
                    PSD2 = 0x00004000;              /* unmapped, blocked interrupts mode */
                    M[0x680>>2] = PSD1;             /* store PSD 1 */
                    M[0x684>>2] = PSD2;             /* store PSD 2 */
                    M[0x688>>2] = TRAPSTATUS;       /* store trap status */
                    M[0x68C>>2] = 0;                /* This will be device table entry later TODO */
fprintf(stderr, "[][][][][][][][][][][] HALT TRAP [][][][][][][][][][][][]\r\n");
fprintf(stderr, "PSD1 %.8x PSD2 %.8x TRAPME %.4x\r\n", PSD1, PSD2, TRAPME);
    for (ix=0; ix<8; ix+=2) {
        fprintf(stderr, "GPR[%d] %.8x GPR[%d] %.8x\r\n", ix, GPR[ix], ix+1, GPR[ix+1]);
    }
fprintf(stderr, "[][][][][][][][][][][] HALT TRAP [][][][][][][][][][][][]\r\n");
                    return STOP_HALT;               /* exit to simh for halt */
                } else {
                    /* valid vector, so store the PSD, fetch new PSD */
                    bc = PSD2 & 0x3ffc;             /* get copy of cpix */
                    M[tvl>>2] = PSD1 & 0xfffffffe;  /* store PSD 1 */
                    M[(tvl>>2)+1] = PSD2;           /* store PSD 2 */
                    PSD1 = M[(tvl>>2)+2];           /* get new PSD 1 */
                    PSD2 = (M[(tvl>>2)+3] & ~0x3ffc) | bc;  /* get new PSD 2 w/old cpix */
                    M[(tvl>>2)+4] = TRAPSTATUS;     /* store trap status */

                    /* set the mode bits and CCs from the new PSD */
                    CC = PSD1 & 0x78000000;         /* extract bits 1-4 from PSD1 */
                    modes = PSD1 & 0x87000000;      /* extract bits 0, 5, 6, 7 from PSD 1 */
                    /* set new map mode and interrupt blocking state in CPUSTATUS */
                    if (PSD2 & MAPBIT) {
                        CPUSTATUS |= 0x00800000;    /* set bit 8 of cpu status */
                        modes |= MAPMODE;           /* set mapped mode */
                    } else
                        CPUSTATUS &= 0xff7fffff;    /* reset bit 8 of cpu status */
                    /* set interrupt blocking state */
                    if ((PSD2 & 0x8000) == 0) {     /* is it retain blocking state */
                        if (PSD2 & 0x4000)          /* no, is it set blocking state */
                            CPUSTATUS |= 0x80;      /* yes, set blk state in cpu status bit 24 */
                        else
                            CPUSTATUS &= ~0x80;     /* no, reset blk state in cpu status bit 24 */
                    }
                    PSD2 &= ~0x0000c000;            /* clear bit 48 & 49 to be unblocked */
                    if (CPUSTATUS & 0x80)           /* see if old mode is blocked */
                        PSD2 |= 0x00004000;         /* set to blocked state */

                    PSD2 &= ~RETMBIT;               /* turn off retain bit in PSD2 */
                    SPAD[0xf5] = PSD2;              /* save the current PSD2 */
#if 0
if (TRAPME == UNDEFINSTR_TRAP) {
sim_debug(DEBUG_EXP, &cpu_dev, "TRAP PSD1 %x PSD2 %x CPUSTATUS %x\n", PSD1, PSD2, CPUSTATUS);
fprintf(stderr, "TRAPS %x LOAD MAPS PSD1 %x PSD2 %x CPUSTATUS %x\r\n", TRAPME, PSD1, PSD2, CPUSTATUS);
goto dumpi;
}
#endif
                    break;                          /* Go execute the trap */
                }
                break;
            }
        }
        skipinstr = 1;                              /* skip next instruction */
        /* we have a new PSD loaded via a LPSD or LPSDCM */
        /* TODO finish instruction history, then continue */
        /* update cpu status word too */
#if 0
if (traceme >= trstart) {
dumpi:
OPSD1 &= 0x87FFFFFE;                    /* clear the old CC's */
OPSD1 |= PSD1 & 0x78000000;             /* update the CC's in the PSD */
if (modes & MAPMODE)
    fprintf(stderr, "M%.8x %.8x ", OPSD1, OIR);
else
    fprintf(stderr, "U%.8x %.8x ", OPSD1, OIR);
fprint_inst(stderr, OIR, 0);    /* display instruction */
fprintf(stderr, " R0=%x R1=%x R2=%x R3=%x", GPR[0], GPR[1], GPR[2], GPR[3]);
fprintf(stderr, " R4=%x R5=%x R6=%x R7=%x", GPR[4], GPR[5], GPR[6], GPR[7]);
fprintf(stderr, "\r\n");
}
#endif
sim_debug(DEBUG_DATA, &cpu_dev, "R0=%08x R1=%08x R2=%08x R3=%08x\n", GPR[0], GPR[1], GPR[2], GPR[3]);
sim_debug(DEBUG_DATA, &cpu_dev, "R4=%08x R5=%08x R6=%08x R7=%08x\n", GPR[4], GPR[5], GPR[6], GPR[7]);
        continue; /* single step cpu just for now */
//      break;      /* quit for now after each instruction */
    }   /* end while */

    /* Simulation halted */
//fprintf(stderr, "@end PSD1 %.8x PSD2 %.8x addr %.8x\r\n", PSD1, PSD2, addr);
    return reason;
}

/* these are the default ipl devices defined by the CPU jumpers */
/* they can be overridden by specifying IPL device at ipl time */
uint32 def_disk = 0x0800;       /* disk channel 8, device 0 */
uint32 def_tape = 0x1000;       /* tape device 10, device 0 */
uint32 def_floppy = 0x7ef0;     /* IOP floppy disk channel 7e, device f0 */
//int once = 0;                 /* one time inits */

/* Reset routine */
/* do any one time initialization here for cpu */
t_stat cpu_reset(DEVICE * dptr)
{
    int i;

    /* leave regs alone so values can be passed to boot code */
    PSD1 = 0x80000000;              /* privileged, non mapped, non extended, address 0 */
    PSD2 = 0x00004000;              /* blocked interrupts mode */
    modes = (PRIVBIT | BLKMODE);    /* set modes to privileged and blocked interrupts */
    CC = 0;                         /* no CCs too */
    CPUSTATUS = CPU_MODEL;          /* clear all cpu status except cpu type */
    CPUSTATUS |= 0x80000000;        /*  set privleged state bit 0 */
    CPUSTATUS |= 0x00000080;        /* set blocked mode state bit 24 */
    TRAPSTATUS = CPU_MODEL;         /* clear all trap status except cpu type */

    chan_set_devs();                /* set up the defined devices on the simulator */

    /* set default breaks to execution tracing */
    sim_brk_types = sim_brk_dflt = SWMASK('E');
    /* zero regs */
    for (i = 0; i < 8; i++) {
        GPR[i] = 0;     /* clear the registers */
        BR[i] = 0;      /* clear the registers */
    }
    /* zero interrupt status words */
    for (i = 0; i < 112; i++)
        INTS[i] = 0;                /* clear interrupt status flags */

    /* add code here to initialize the SEL32 cpu scratchpad on initial start */
    /* see if spad setup by software, if yes, leave spad alone */
    /* otherwise set the default values into the spad */
    /* CPU key is 0xECDAB897, IPU key is 0x13254768 */
    /* Keys are loaded by the O/S software during the boot loading sequence */
    if (SPAD[0xf7] != 0xecdab897)
    {
        int ival = 0;               /* init value for concept 32 */

        if (CPU_MODEL < MODEL_27)
            ival = 0xfffffff;       /* init value for 32/7x int and dev entries */
        for (i = 0; i < 1024; i++)
            MAPC[i] = 0;            /* clear 2048 halfword map cache */
        for (i = 0; i < 224; i++)
            SPAD[i] = ival;         /* init 128 devices and 96 ints in the spad */
        for (i = 224; i < 256; i++) /* clear the last 32 extries */
            SPAD[i] = 0;            /* clear the spad */
        SPAD[0xf0] = 0x80;          /* default Trap Table Address (TTA) */
        SPAD[0xf1] = 0x100;         /* Interrupt Table Address (ITA) */
        SPAD[0Xf2] = 0x700;         /* IOCD Base Address */
        SPAD[0xf3] = 0x788;         /* Master Process List (MPL) table address */
        SPAD[0xf4] = def_tape;      /* Default IPL address from console IPL command or jumper */
        SPAD[0xf5] = 0x00004000;    /* current PSD2 defaults to blocked */
        SPAD[0xf6] = 0;             /* reserved (PSD1 ??) */
        SPAD[0xf7] = 0;             /* make sure key is zero */
        SPAD[0xf8] = 0x0000f000;    /* set DRT to class f (anything else is E) */
        SPAD[0xf9] = CPU_MODEL;     /* set default cpu type in cpu status word */
        SPAD[0xff] = 0x00ffffff;    /* interrupt level 7f 1's complament */
    }
    M[0] = 0x02000000;  /* 0x00 IOCD 1 read into address 0 */
    M[1] = 0x60000078;  /* 0x04 IOCD 1 CMD Chain, Suppress incor length, 120 bytes */
    M[2] = 0x53000000;  /* 0x08 IOCD 2 BKSR or RZR to re-read boot code */
    M[3] = 0x60000001;  /* 0x0C IOCD 2 CMD chain,Supress incor length, 1 byte */
    M[4] = 0x02000000;  /* 0x10 IOCD 3 Read into address 0 */
    M[5] = 0x000006EC;  /* 0x14 IOCD 3 Read 0x6EC bytes */
    loading = 0;                    /* not loading yet */
    /* we are good to go */
    return SCPE_OK;
}

/* Memory examine */
/* examine a 32bit memory location */
/* address is byte address with bits 30,31 = 0 */
t_stat cpu_ex(t_value *vptr, t_addr baddr, UNIT *uptr, int32 sw)
{
    uint32 temp, t;
    uint32 addr = (baddr & 0xfffffc) >> 2;  /* make 24 bit byte address into word address */

    /* MSIZE is in 32 bit words */
    if (addr >= MEMSIZE)    /* see if address is within our memory */
        return SCPE_NXM;    /* no, none existant memory error */
    if (vptr == NULL)       /* any address specified by user */
        return SCPE_OK;     /* no, just ignore the request */
    *vptr = M[addr];        /* return memory contents */
    return SCPE_OK;         /* we are all ok */
}

/* Memory deposit */
/* modify a 32bit memory location */
/* address is byte address with bits 30,31 = 0 */
t_stat cpu_dep(t_value val, t_addr baddr, UNIT *uptr, int32 sw)
{
    uint32 addr = (baddr & 0xfffffc) >> 2;  /* make 24 bit byte address into word address */

//fprintf(stderr, "cpu_dep baddr %0x, sw %x\b\n", baddr, sw);
    /* MSIZE is in 32 bit words */
    if (addr >= MEMSIZE)    /* see if address is within our memory */
        return SCPE_NXM;    /* no, none existant memory error */
    M[addr] = val;          /* set the new data value */
    return SCPE_OK;         /* all OK */
}

/* set the CPU memory size */
t_stat cpu_set_size(UNIT *uptr, int32 val, CONST char *cptr, void *desc)
{
    t_uint64            mc = 0;
    uint32              i;

//fprintf(stderr, "WE are here 4 \b\n");
    cpu_unit.flags &= ~UNIT_MSIZE;
    cpu_unit.flags |= val;      /* set new memory size */
    val >>= UNIT_V_MSIZE;       /* set size in 32bit words */
    val = (val + 1) * 128 * 1024;   /* KW's */
    if ((val < 0) || (val > MAXMEMSIZE))    /* is size valid */
        return SCPE_ARG;        /* nope, argument error */
    for (i = val; i < MEMSIZE; i++) /* see if memory contains anything */
        mc |= M[i];             /* or in any bits in memory */
    if ((mc != 0) && (!get_yn("Really truncate memory [N]?", FALSE)))
        return SCPE_OK;         /* return OK if user says no */
    MEMSIZE = val;      /* set new size */
    for (i = MEMSIZE; i < MAXMEMSIZE; i++)
        M[i] = 0;       /* zero all of the new memory */
    return SCPE_OK;     /* we done */
}

/* Handle execute history */

/* Set history */
t_stat
cpu_set_hist(UNIT *uptr, int32 val, CONST char *cptr, void *desc)
{
    int32               i, lnt;
    t_stat              r;

    if (cptr == NULL) {     /* check for any user options */
        for (i = 0; i < hst_lnt; i++)   /* none, so just zero the history */
            hst[i].psd1 = 0;    /* just psd1 for now */
        hst_p = 0;          /* start at teh beginning */
        return SCPE_OK;     /* all OK */
    }
    /* the user has specified options, process them */
    lnt = (int32) get_uint(cptr, 10, HIST_MAX, &r);
    if ((r != SCPE_OK) || (lnt && (lnt < HIST_MIN)))
        return SCPE_ARG;    /* arg error for bad input or too small a value */
    hst_p = 0;              /* start at beginning */
    if (hst_lnt) {          /* if a new length was input, resize history buffer */
        free(hst);          /* out with the old */
        hst_lnt = 0;        /* no length anymore */
        hst = NULL;         /* and no pointer either */
    }
    if (lnt) {              /* see if new size specified, if so get new resized buffer  */
        hst = (struct InstHistory *)calloc(sizeof(struct InstHistory), lnt);
        if (hst == NULL)
            return SCPE_MEM;    /* allocation error, so tell user */
        hst_lnt = lnt;  /* set new length */
    }
    return SCPE_OK;     /* we are good to go */
}

/* Show history */
t_stat cpu_show_hist(FILE * st, UNIT * uptr, int32 val, CONST void *desc)
{
    int32               k, di, lnt;
    char               *cptr = (char *) desc;
    t_stat              r;
    struct InstHistory *h;

    if (hst_lnt == 0)       /* see if show history is enabled */
        return SCPE_NOFNC;  /* no, so are out of here */
    if (cptr) {             /* see if user provided a display count */
        lnt = (int32)get_uint(cptr, 10, hst_lnt, &r);   /* get the count */
        if ((r != SCPE_OK) || (lnt == 0))   /* if error or 0 count */
            return SCPE_ARG;                /* report argument error */
    } else
        lnt = hst_lnt;      /* dump all the entries */
    di = hst_p - lnt;       /* work forward */
    if (di < 0)
        di = di + hst_lnt;  /* wrap */
    fprintf(st, "PSD1     PSD2     INST     DEST     SRC      CC\n");
    for (k = 0; k < lnt; k++) {         /* print specified entries */
        h = &hst[(++di) % hst_lnt];     /* entry pointer */
        /* display the instruction and results */
        fprintf(st, "%08x %08x %08x %08x %08x %1x",
            h->psd1, h->psd2, h->inst, h->dest, h->src, h->cc); 
            fputc('\n', st);    /* end line */
    }                           /* end for */
    return SCPE_OK;     /* all is good */
}

/* return description for the specified device */
const char *cpu_description (DEVICE *dptr) 
{
    return "SEL 32 CPU";    /* return description */
}

t_stat
cpu_help (FILE *st, DEVICE *dptr, UNIT *uptr, int32 flag, const char *cptr)
{
    fprintf (st, "The CPU can be set to \n");
    fprintf (st, "The CPU can maintain a history of the most recently executed instructions.\n");
    fprintf (st, "This is controlled by the SET CPU HISTORY and SHOW CPU HISTORY commands:\n\n");
    fprintf (st, "   sim> SET CPU HISTORY                 clear history buffer\n");
    fprintf (st, "   sim> SET CPU HISTORY=0               disable history\n");
    fprintf (st, "   sim> SET CPU HISTORY=n{:file}        enable history, length = n\n");
    fprintf (st, "   sim> SHOW CPU HISTORY                print CPU history\n");
    return SCPE_OK;
}
