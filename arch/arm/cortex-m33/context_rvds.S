;/**
 ;***********************************************************************************************************************
 ;* Copyright (c) 2020, China Mobile Communications Group Co.,Ltd.
 ;*
 ;* Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with 
 ;* the License. You may obtain a copy of the License at
 ;*
 ;*     http://www.apache.org/licenses/LICENSE-2.0
 ;*
 ;* Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on 
 ;* an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the 
 ;* specific language governing permissions and limitations under the License.
 ;*
 ;* @file        context_rvds.S
 ;*
 ;* @brief       This file provides context switch functions related to the ARM M4 architecture.
 ;*
 ;* @revision
 ;* Date         Author          Notes
 ;* 2020-02-23   OneOS Team      First version.
 ;***********************************************************************************************************************
 ;*/

SCB_VTOR        EQU     0xE000ED08               ; Vector table offset register
NVIC_INT_CTRL   EQU     0xE000ED04               ; Interrupt control state register
NVIC_SYSPRI2    EQU     0xE000ED20               ; System priority register (2)
NVIC_PENDSV_PRI EQU     0x00FF0000               ; PendSV priority value (lowest)
NVIC_PENDSVSET  EQU     0x10000000               ; Value to trigger PendSV exception

    AREA |.text|, CODE, READONLY, ALIGN=2
    THUMB
    REQUIRE8
    PRESERVE8

    IMPORT os_task_switch_interrupt_flag
    IMPORT os_interrupt_from_task
    IMPORT os_interrupt_to_task

    IMPORT os_hw_hard_fault_exception

;/**
 ;***********************************************************************************************************************
 ;* @brief           Disable interrupt.
 ;*
 ;* @param           None.
 ;*
 ;* @return          The state before disable interrupt.
 ;***********************************************************************************************************************
 ;*/
os_hw_interrupt_disable    PROC
    EXPORT  os_hw_interrupt_disable
    MRS     r0, PRIMASK
    CPSID   I
    BX      LR
    ENDP

;/**
 ;***********************************************************************************************************************
 ;* @brief           Enable interrupt.
 ;*
 ;* @param[in]       State to restore.
 ;*
 ;* @return          None.
 ;***********************************************************************************************************************
 ;*/
os_hw_interrupt_enable    PROC
    EXPORT  os_hw_interrupt_enable
    MSR     PRIMASK, r0
    BX      LR
    ENDP

;/**
 ;***********************************************************************************************************************
 ;* @brief           Start the task swtich process by software trigger PendSV interrupt.
 ;*
 ;* @param[in]       from            SP of 'from' task. from-->r0.
 ;* @param[in]       to              SP of 'to' task.   to  -->r1.
 ;*
 ;* @return          None.
 ;***********************************************************************************************************************
 ;*/
os_hw_context_switch_interrupt
    EXPORT os_hw_context_switch_interrupt
os_hw_context_switch    PROC
    EXPORT os_hw_context_switch

    ; Set os_task_switch_interrupt_flag to 1
    LDR     r2, =os_task_switch_interrupt_flag
    LDR     r3, [r2]
    CMP     r3, #1
    BEQ     _reswitch
    MOV     r3, #1
    STR     r3, [r2]

    LDR     r2, =os_interrupt_from_task   ; Set os_interrupt_from_task
    STR     r0, [r2]

_reswitch
    LDR     r2, =os_interrupt_to_task     ; Set os_interrupt_to_task
    STR     r1, [r2]

    LDR     r0, =NVIC_INT_CTRL            ; Trigger the PendSV exception (causes context switch)
    LDR     r1, =NVIC_PENDSVSET
    STR     r1, [r0]
    BX      LR
    ENDP

;/**
 ;***********************************************************************************************************************
 ;* @brief           PendSV interrupt handler,switch the context of the task.
 ;*
 ;* @param           None.
 ;*
 ;* @return          None.
 ;***********************************************************************************************************************
 ;*/
PendSV_Handler   PROC
    EXPORT PendSV_Handler

    ; Disable interrupt to protect context switch
    MRS     r2, PRIMASK
    CPSID   I

    ; Get os_task_switch_interrupt_flag
    LDR     r0, =os_task_switch_interrupt_flag
    LDR     r1, [r0]
    CBZ     r1, pendsv_exit         ; Pendsv already handled

    ; Clear os_task_switch_interrupt_flag to 0
    MOV     r1, #0x00
    STR     r1, [r0]

    LDR     r0, =os_interrupt_from_task
    LDR     r1, [r0]
    CBZ     r1, switch_to_task    ; Skip register save at the first time

    MRS     r1, psp                 ; Get from task stack pointer

    IF      {FPU} != "SoftVFP"
    TST     lr, #0x10               ; if(!EXC_RETURN[4])
    VSTMFDEQ  r1!, {d8 - d15}       ; Push FPU register s16~s31
    ENDIF

    STMFD   r1!, {r4 - r11}         ; Push r4 - r11 register

    IF      {FPU} != "SoftVFP"
    MOV     r4, #0x00               ; flag = 0

    TST     lr, #0x10               ; if(!EXC_RETURN[4])
    MOVEQ   r4, #0x01               ; flag = 1

    STMFD   r1!, {r4}               ; Push flag
    ENDIF

    LDR     r0, [r0]
    STR     r1, [r0]                ; Update from task stack pointer

switch_to_task
    LDR     r1, =os_interrupt_to_task
    LDR     r1, [r1]
    LDR     r1, [r1]                ; Load task stack pointer

    IF      {FPU} != "SoftVFP"
    LDMFD   r1!, {r3}               ; Pop flag
    ENDIF

    LDMFD   r1!, {r4 - r11}         ; Pop r4 - r11 register

    IF      {FPU} != "SoftVFP"
    CMP     r3,  #0                 ; if(flag_r3 != 0)
    VLDMFDNE  r1!, {d8 - d15}       ; Pop FPU register s16~s31
    ENDIF

    MSR     psp, r1                 ; Update stack pointer

    IF      {FPU} != "SoftVFP"
    ORR     lr, lr, #0x10           ; lr |=  (1 << 4), clean FPCA.
    CMP     r3,  #0                 ; if(flag_r3 != 0)
    BICNE   lr, lr, #0x10           ; lr &= ~(1 << 4), set FPCA.
    ENDIF

pendsv_exit
    ; Restore interrupt
    MSR     PRIMASK, r2

    ORR     lr, lr, #0x04
    BX      lr
    ENDP

;/**
 ;***********************************************************************************************************************
 ;* @brief           This function is called when the scheduler starts the first task, Only used once.
 ;*
 ;* @param[in]       to              SP of 'to' task.
 ;*
 ;* @return          None.
 ;***********************************************************************************************************************
 ;*/
os_hw_context_switch_to    PROC
    EXPORT os_hw_context_switch_to
    ; Set to task
    LDR     r1, =os_interrupt_to_task
    STR     r0, [r1]

    IF      {FPU} != "SoftVFP"
    ; CLEAR CONTROL.FPCA
    MRS     r2, CONTROL             ; Read
    BIC     r2, #0x04               ; Modify
    MSR     CONTROL, r2             ; Write-back
    ENDIF

    ; Set from task to 0
    LDR     r1, =os_interrupt_from_task
    MOV     r0, #0x0
    STR     r0, [r1]

    ; Set interrupt flag to 1
    LDR     r1, =os_task_switch_interrupt_flag
    MOV     r0, #1
    STR     r0, [r1]

    ; set the PendSV exception priority
    LDR     r0, =NVIC_SYSPRI2
    LDR     r1, =NVIC_PENDSV_PRI
    LDR.W   r2, [r0,#0x00]       ; Read
    ORR     r1,r1,r2             ; Modify
    STR     r1, [r0]             ; Write-back

    ; Trigger the PendSV exception (causes context switch)
    LDR     r0, =NVIC_INT_CTRL
    LDR     r1, =NVIC_PENDSVSET
    STR     r1, [r0]

    ; Restore MSP
    LDR     r0, =SCB_VTOR
    LDR     r0, [r0]
    LDR     r0, [r0]
    MSR     msp, r0

    ; Enable interrupts at processor level
    CPSIE   F
    CPSIE   I

    ; Never reach here!
    ENDP

;/**
 ;***********************************************************************************************************************
 ;* @brief           HardFault interrupt handler.
 ;*
 ;* @param           None.
 ;*
 ;* @return          None.
 ;***********************************************************************************************************************
 ;*/
    EXPORT HardFault_Handler
HardFault_Handler    PROC

    ; get current context
    TST     lr, #0x04               ; if(!EXC_RETURN[2])
    ITE     EQ
    MRSEQ   r0, msp                 ; [2]=0 ==> Z=1, get fault context from handler.
    MRSNE   r0, psp                 ; [2]=1 ==> Z=0, get fault context from task.

    STMFD   r0!, {r4 - r11}         ; Push r4 - r11 register
    IF      {FPU} != "SoftVFP"
    STMFD   r0!, {lr}               ; Push dummy for flag
    ENDIF
    STMFD   r0!, {lr}               ; Push exec_return register

    TST     lr, #0x04               ; if(!EXC_RETURN[2])
    ITE     EQ
    MSREQ   msp, r0                 ; [2]=0 ==> Z=1, update stack pointer to MSP.
    MSRNE   psp, r0                 ; [2]=1 ==> Z=0, update stack pointer to PSP.

    PUSH    {lr}
    BL      os_hw_hard_fault_exception
    POP     {lr}

    ORR     lr, lr, #0x04
    BX      lr
    ENDP

    ALIGN   4

    END

