/*
    Copyright 1983
    Alcyon Corporation
    8716 Production Ave.
    San Diego, Ca.  92121
*/


adirect const p1direct[DIRECT] = {
    hopd,       /*  0 DIR_OPD */
    hend,       /*  1 DIR_END */
    hdsect,     /*  2 DIR_DSECT */
    hpsect,     /*  3 DIR_PSECT */
    hequ,       /*  4 DIR_EQU */
    hequ,       /*  5 DIR_SET .set same as .equ */
    0,          /*  6 */
    hascii,     /*  7 DIR_ASCII */
    hdc,        /*  8 DIR_DC */
    hent,       /*  9 DIR_GLOBL */
    hext,       /* 10 DIR_COMM */
    hbss,       /* 11 DIR_BSS */
    hds,        /* 12 DIR_DS */
    heven,      /* 13 DIR_EVEN */
    horg,       /* 14 DIR_ORG */
    hmask2,     /* 15 DIR_MASK2 */
    hreg,       /* 16 DIR_REGEQU */
    hdcb,       /* 17 DIR_DCB */
    hcomline,   /* 18 DIR_COMLINE */
    hidnt,      /* 19 DIR_IDNT */
    hoffset,    /* 20 DIR_OFFSET */
    hsection,   /* 21 DIR_SECTION */
    hifeq,      /* 22 DIR_IFEQ */
    hifne,      /* 23 DIR_IFNE */
    hiflt,      /* 24 DIR_IFLT */
    hifle,      /* 25 DIR_IFLE */
    hifgt,      /* 26 DIR_IFGT */
    hifge,      /* 27 DIR_IFGE */
    hendc,      /* 28 DIR_ENDC */
    hifc,       /* 29 DIR_IFC */
    hifnc,      /* 30 DIR_IFNC */
    hopt,       /* 31 DIR_OPT */
    httl,       /* 32 DIR_TTL */
    hpage       /* 33 DIR_PAGE */
};

adirect const p2direct[DIRECT] = {
    0,          /*  0 */
    send,       /*  1 */
    sdsect,     /*  2 */
    spsect,     /*  3 */
    0,          /*  4 */
    0,          /*  5 */
    0,          /*  6 */
    sascii,     /*  7 */
    sdc,        /*  8 */
    0,          /*  9 */
    0,          /* 10 */
    sbss,       /* 11 */
    sds,        /* 12 */
    seven,      /* 13 */
    sorg,       /* 14 */
    0,          /* 15 */
    0,          /* 16 */
    sdcb,       /* 17 */
    sds,        /* 18  comline same as .ds.b */
    0,          /* 19 */
    0,          /* 20 */
    ssection,   /* 21 */
    0,          /* 22 */
    0,          /* 23 */
    0,          /* 24 */
    0,          /* 25 */
    0,          /* 26 */
    0,          /* 27 */
    0,          /* 28 */
    0,          /* 29 */
    0,          /* 30 */
    0,          /* 31 */
    0,          /* 32 */
    spage       /* 33 */
};
