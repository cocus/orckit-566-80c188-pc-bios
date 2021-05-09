/* 
 *  Copyright (c) 1995, Intel Corporation 
 * 
 *  $Workfile:   80c186eb.h  $ 
 *  $Revision:   1.1  $ 
 *  $Modtime:   Mar 22 1995 16:55:24  $ 
 * 
 *  Purpose: 
 * 
 * 
 * 
 * 
 * 
 *  Compiler:        
 * 
 *  Ext Packages:    
 * 
 *  
 * 
 */ 
/*  80C186EB REGISTER DEFINITIONS */ 
#ifndef _80C186EB_ 
#define _80C186EB_
/* 
  Modify the PCB_BASE symbol to represent 
  the contents of the Relocation register (RELREG). 
 
   Set Default condtion for placement of the Peripheral 
   Control Block. 
*/ 
#ifndef  _MEM_SPACE 
#define  _IO_SPACE_ 
#endif 
 
/*  Note: when placing the Peripheral Control Block(PCB) into 
    the memory space define the PCB_BASE in terms of segment:offset. 
    ie.  a PCB base address of 0x1f000 should define PCB_BASE as 
    #define PCB_BASE  0x1000f000L. 
*/ 
    /* Set Default condition for PCB address */ 
#ifndef PCB_BASE 
#ifndef _MEM_SPACE_ 
#define PCB_BASE    0xff00 
#else 
#define PCB_BASE    0xff00000L 
#endif 
#endif 
 
/* This typedef assumes that an unsigned int is a 16-bit value, if your 
   compiler's "unsigned int" is not a 16-bit value modify the typedef to 
   a data type that is a 16-bit value. 
*/ 
typedef unsigned int _WORD16_; 
 
  /* Note: The "far" keyword is used to create a far pointer to the 
           registers.  Some compilers may not support the "far" keyword, 
           you will need to modify "_WORD16_ far *" to match what your   
           compiler supports. 
  */ 
#ifdef _MEM_SPACE_ 
#define _MAKE_ADDRESS_(offset) (*((_WORD16_ far *) (PCB_BASE +(offset)))) 
#define _Set186Register(reg,val) ((reg) = (val)) 
#define _Set186RegisterByte(reg,val) _Set186Register(reg,val) 
#define _Get186Register(reg) (reg) 
#else 
#define _MAKE_ADDRESS_(offset) (PCB_BASE + (offset)) 
#define _Set186Register(reg,val) (outpw(reg,val)) 
#define _Set186RegisterByte(reg,val) (outp(reg,val)) 
#define _Get186Register(reg) (inpw(reg)) 
#endif 
 
    /*  INTERRUPT CONTROL REGISTERS  */ 
#define EOI     _MAKE_ADDRESS_(0x02) 
#define POLL	_MAKE_ADDRESS_(0x04) 
#define POLLSTS _MAKE_ADDRESS_(0x06) 
#define IMASK   _MAKE_ADDRESS_(0x08) 
#define PRIMSK  _MAKE_ADDRESS_(0x0A) 
#define INSERV  _MAKE_ADDRESS_(0x0C) 
#define REQST   _MAKE_ADDRESS_(0x0E) 
#define INTSTS  _MAKE_ADDRESS_(0x10) 
#define TCUCON  _MAKE_ADDRESS_(0x12) 
#define SCUCON  _MAKE_ADDRESS_(0x14) 
#define I4CON   _MAKE_ADDRESS_(0x16) 
#define I0CON   _MAKE_ADDRESS_(0x18) 
#define I1CON   _MAKE_ADDRESS_(0x1A) 
#define I2CON   _MAKE_ADDRESS_(0x1C) 
#define I3CON   _MAKE_ADDRESS_(0x1E) 
 
    /*  TIMER CONTROL REGISTERS  */ 
#define T0CNT   _MAKE_ADDRESS_(0x30) 
#define T0CMPA  _MAKE_ADDRESS_(0x32) 
#define T0CMPB  _MAKE_ADDRESS_(0x34) 
#define T0CON   _MAKE_ADDRESS_(0x36) 
#define T1CNT   _MAKE_ADDRESS_(0x38) 
#define T1CMPA  _MAKE_ADDRESS_(0x3A) 
#define T1CMPB  _MAKE_ADDRESS_(0x3C) 
#define T1CON   _MAKE_ADDRESS_(0x3E) 
#define T2CNT   _MAKE_ADDRESS_(0x40) 
#define T2CMPA  _MAKE_ADDRESS_(0x42) 
#define T2CON   _MAKE_ADDRESS_(0x46) 
 
    /* INPUT/OUTPUT PORT UNIT REGISTERS  */ 
#define P1DIR   _MAKE_ADDRESS_(0x50) 
#define P1PIN   _MAKE_ADDRESS_(0x52) 
#define P1CON   _MAKE_ADDRESS_(0x54) 
#define P1LTCH  _MAKE_ADDRESS_(0x56) 
#define P2DIR   _MAKE_ADDRESS_(0x58) 
#define P2PIN   _MAKE_ADDRESS_(0x5A) 
#define P2CON   _MAKE_ADDRESS_(0x5C) 
#define P2LTCH  _MAKE_ADDRESS_(0x5E) 
 
    /* SERIAL COMMUNICATION UNIT REGISTERS  */ 
#define B0CMP   _MAKE_ADDRESS_(0x60) 
#define B0CNT   _MAKE_ADDRESS_(0x62) 
#define S0CON   _MAKE_ADDRESS_(0x64) 
#define S0STS   _MAKE_ADDRESS_(0x66) 
#define R0BUF   _MAKE_ADDRESS_(0x68) 
#define T0BUF   _MAKE_ADDRESS_(0x6A) 
#define B1CMP   _MAKE_ADDRESS_(0x70) 
#define B1CNT   _MAKE_ADDRESS_(0x72) 
#define S1CON   _MAKE_ADDRESS_(0x74) 
#define S1STS   _MAKE_ADDRESS_(0x76) 
#define R1BUF   _MAKE_ADDRESS_(0x78) 
#define T1BUF   _MAKE_ADDRESS_(0x7A) 
 
    /* CHIP SELECT UNIT REGISTERS  */ 
#define GCS0ST  _MAKE_ADDRESS_(0x80) 
#define GCS0SP  _MAKE_ADDRESS_(0x82) 
#define GCS1ST  _MAKE_ADDRESS_(0x84) 
#define GCS1SP  _MAKE_ADDRESS_(0x86) 
#define GCS2ST  _MAKE_ADDRESS_(0x88) 
#define GCS2SP  _MAKE_ADDRESS_(0x8A) 
#define GCS3ST  _MAKE_ADDRESS_(0x8C) 
#define GCS3SP  _MAKE_ADDRESS_(0x8E) 
#define GCS4ST  _MAKE_ADDRESS_(0x90) 
#define GCS4SP  _MAKE_ADDRESS_(0x92) 
#define GCS5ST  _MAKE_ADDRESS_(0x94) 
#define GCS5SP  _MAKE_ADDRESS_(0x96) 
#define GCS6ST  _MAKE_ADDRESS_(0x98) 
#define GCS6SP  _MAKE_ADDRESS_(0x9A) 
#define GCS7ST  _MAKE_ADDRESS_(0x9C) 
#define GCS7SP  _MAKE_ADDRESS_(0x9E) 
#define LCSST   _MAKE_ADDRESS_(0xA0) 
#define LCSSP   _MAKE_ADDRESS_(0xA2) 
#define UCSST   _MAKE_ADDRESS_(0xA4) 
#define UCSSP   _MAKE_ADDRESS_(0xA6) 
 
    /* PERIPHERAL CONTROL BLOCK RELOCATION REGISTER  */ 
#define RELREG  _MAKE_ADDRESS_(0xA8) 
 
    /*  REFRESH CONTROL UNIT REGISTERS  */ 
#define RFBASE  _MAKE_ADDRESS_(0xB0) 
#define RFTIME  _MAKE_ADDRESS_(0xB2) 
#define RFCON   _MAKE_ADDRESS_(0xB4) 
#define RFADDR  _MAKE_ADDRESS_(0xB6) 
 
    /*  POWER MANAGEMENT REGISTERS  */ 
#define PWRCON  _MAKE_ADDRESS_(0xB8) 
 
    /*  STEPPING ID REGISTER  */ 
#define STEPID  _MAKE_ADDRESS_(0xBC) 
 
#endif 