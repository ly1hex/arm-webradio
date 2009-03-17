//*****************************************************************************
//
// cpu.c - Instruction wrappers for special CPU instructions needed by the
//         drivers.
//
// Copyright (c) 2006-2009 Luminary Micro, Inc.  All rights reserved.
// Software License Agreement
// 
// Luminary Micro, Inc. (LMI) is supplying this software for use solely and
// exclusively on LMI's microcontroller products.
// 
// The software is owned by LMI and/or its suppliers, and is protected under
// applicable copyright laws.  All rights are reserved.  You may not combine
// this software with "viral" open-source software in order to form a larger
// program.  Any use in violation of the foregoing restrictions may subject
// the user to criminal sanctions under applicable laws, as well as to civil
// liability for the breach of the terms and conditions of this license.
// 
// THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
// OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
// LMI SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR
// CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
// 
// This is part of revision 4053 of the Stellaris Peripheral Driver Library.
//
//*****************************************************************************

#include "driverlib/cpu.h"

//*****************************************************************************
//
// Wrapper function for the CPSID instruction.  Returns the state of PRIMASK
// on entry.
//
//*****************************************************************************
#if defined(codered) || defined(gcc) || defined(sourcerygxx)
void __attribute__((naked))
CPUcpsid(void)
{
    //
    // Disable interrupts.
    //
    __asm("    cpsid   i\n"
          "    bx      lr\n");
}
#endif
#if defined(ewarm)
void
CPUcpsid(void)
{
    //
    // Disable interrupts.
    //
    __asm("    cpsid   i\n");
}
#endif
#if defined(rvmdk) || defined(__ARMCC_VERSION)
__asm void
CPUcpsid(void)
{
    //
    // Disable interrupts.
    //
    cpsid   i;
    bx      lr
}
#endif

//*****************************************************************************
//
// Wrapper function for the CPSIE instruction.  Returns the state of PRIMASK
// on entry.
//
//*****************************************************************************
#if defined(codered) || defined(gcc) || defined(sourcerygxx)
void __attribute__((naked))
CPUcpsie(void)
{
    //
    // Enable interrupts.
    //
    __asm("    cpsie   i\n"
          "    bx      lr\n");
}
#endif
#if defined(ewarm)
void
CPUcpsie(void)
{
    //
    // Enable interrupts.
    //
    __asm("    cpsie   i\n");
}
#pragma diag_default=Pe940
#endif
#if defined(rvmdk) || defined(__ARMCC_VERSION)
__asm void
CPUcpsie(void)
{
    //
    // Enable interrupts.
    //
    cpsie   i;
    bx      lr
}
#endif

//*****************************************************************************
//
// Wrapper function for the WFI instruction.
//
//*****************************************************************************
#if defined(codered) || defined(gcc) || defined(sourcerygxx)
void __attribute__((naked))
CPUwfi(void)
{
    //
    // Wait for the next interrupt.
    //
    __asm("    wfi\n"
          "    bx      lr\n");
}
#endif
#if defined(ewarm)
void
CPUwfi(void)
{
    //
    // Wait for the next interrupt.
    //
    __asm("    wfi\n");
}
#endif
#if defined(rvmdk) || defined(__ARMCC_VERSION)
__asm void
CPUwfi(void)
{
    //
    // Wait for the next interrupt.
    //
    wfi;
    bx      lr
}
#endif
