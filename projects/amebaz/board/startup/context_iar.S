
SCB_VTOR        EQU     0xE000ED08               ; Vector Table Offset Register
NVIC_INT_CTRL   EQU     0xE000ED04               ; interrupt control state register
NVIC_SYSPRI2    EQU     0xE000ED20               ; system priority register (2)
NVIC_PENDSV_PRI EQU     0x00FF0000               ; PendSV priority value (lowest)
NVIC_PENDSVSET  EQU     0x10000000               ; value to trigger PendSV exception

    SECTION    .text:CODE(2)
    THUMB
    REQUIRE8
    PRESERVE8

    IMPORT os_task_switch_interrupt_flag
    IMPORT os_interrupt_from_task
    IMPORT os_interrupt_to_task

;/*
; * os_base_t os_hw_interrupt_disable();
; */
    EXPORT os_hw_interrupt_disable
os_hw_interrupt_disable:
    MRS     r0, PRIMASK
    CPSID   I
    BX      LR

;/*
; * void os_hw_interrupt_enable(os_base_t level);
; */
    EXPORT  os_hw_interrupt_enable
os_hw_interrupt_enable:
    MSR     PRIMASK, r0
    BX      LR

;/*
; * void os_hw_context_switch(os_uint32_t from, os_uint32_t to);
; * r0 --> from
; * r1 --> to
; */
    EXPORT os_hw_context_switch_interrupt
    EXPORT os_hw_context_switch
os_hw_context_switch_interrupt:
os_hw_context_switch:
    ; set os_task_switch_interrupt_flag to 1
    LDR     r2, =os_task_switch_interrupt_flag
    LDR     r3, [r2]
    CMP     r3, #1
    BEQ     _reswitch
    MOV     r3, #1
    STR     r3, [r2]

    LDR     r2, =os_interrupt_from_task   ; set os_interrupt_from_task
    STR     r0, [r2]

_reswitch
    LDR     r2, =os_interrupt_to_task     ; set os_interrupt_to_task
    STR     r1, [r2]

    LDR     r0, =NVIC_INT_CTRL              ; trigger the PendSV exception (causes context switch)
    LDR     r1, =NVIC_PENDSVSET
    STR     r1, [r0]
    BX      LR

; r0 --> switch from task stack
; r1 --> switch to task stack
; psr, pc, lr, r12, r3, r2, r1, r0 are pushed into [from] stack
    EXPORT PendSV_Handler
PendSV_Handler:

    ; disable interrupt to protect context switch
    MRS     r2, PRIMASK
    CPSID   I

    ; get os_task_switch_interrupt_flag
    LDR     r0, =os_task_switch_interrupt_flag
    LDR     r1, [r0]
    CBZ     r1, pendsv_exit         ; pendsv already handled

    ; clear os_task_switch_interrupt_flag to 0
    MOV     r1, #0x00
    STR     r1, [r0]

    LDR     r0, =os_interrupt_from_task
    LDR     r1, [r0]
    CBZ     r1, switch_to_task    ; skip register save at the first time

    MRS     r1, psp                 ; get from thread stack pointer

#if defined ( __ARMVFP__ )
    TST     lr, #0x10               ; if(!EXC_RETURN[4])
    BNE     skip_push_fpu
    VSTMDB  r1!, {d8 - d15}         ; push FPU register s16~s31
skip_push_fpu
#endif

    STMFD   r1!, {r4 - r11}         ; push r4 - r11 register

#if defined ( __ARMVFP__ )
    MOV     r4, #0x00               ; flag = 0
    TST     lr, #0x10               ; if(!EXC_RETURN[4])
    BNE     push_flag
    MOV     r4, #0x01               ; flag = 1
push_flag
    ;STMFD   r1!, {r4}              ; push flag
    SUB     r1, r1, #0x04
    STR     r4, [r1]
#endif

    LDR     r0, [r0]
    STR     r1, [r0]                ; update from task stack pointer

switch_to_task
    LDR     r1, =os_interrupt_to_task
    LDR     r1, [r1]
    LDR     r1, [r1]                ; load task stack pointer

#if defined ( __ARMVFP__ )
    LDMFD   r1!, {r3}               ; pop flag
#endif

    LDMFD   r1!, {r4 - r11}         ; pop r4 - r11 register

#if defined ( __ARMVFP__ )
    CBZ     r3, skip_pop_fpu
    VLDMIA  r1!, {d8 - d15}         ; pop FPU register s16~s31
skip_pop_fpu
#endif

    MSR     psp, r1                 ; update stack pointer

#if defined ( __ARMVFP__ )
    ORR     lr, lr, #0x10           ; lr |=  (1 << 4), clean FPCA.
    CBZ     r3, return_without_fpu  ; if(flag_r3 != 0)
    BIC     lr, lr, #0x10           ; lr &= ~(1 << 4), set FPCA.
return_without_fpu
#endif

pendsv_exit
    ; restore interrupt
    MSR     PRIMASK, r2

    ORR     lr, lr, #0x04
    BX      lr

;/*
; * void os_hw_context_switch_to(os_uint32_t to);
; * r0 --> to
; */
    EXPORT os_hw_context_switch_to
os_hw_context_switch_to:
    LDR     r1, =os_interrupt_to_task
    STR     r0, [r1]

#if defined ( __ARMVFP__ )
    ; CLEAR CONTROL.FPCA
    MRS     r2, CONTROL             ; read
    BIC     r2, r2, #0x04           ; modify
    MSR     CONTROL, r2             ; write-back
#endif

    ; set from task to 0
    LDR     r1, =os_interrupt_from_task
    MOV     r0, #0x0
    STR     r0, [r1]

    ; set interrupt flag to 1
    LDR     r1, =os_task_switch_interrupt_flag
    MOV     r0, #1
    STR     r0, [r1]

    ; set the PendSV exception priority
    LDR     r0, =NVIC_SYSPRI2
    LDR     r1, =NVIC_PENDSV_PRI
    LDR.W   r2, [r0,#0x00]       ; read
    ORR     r1,r1,r2             ; modify
    STR     r1, [r0]             ; write-back

    LDR     r0, =NVIC_INT_CTRL      ; trigger the PendSV exception (causes context switch)
    LDR     r1, =NVIC_PENDSVSET
    STR     r1, [r0]

    ; restore MSP
    LDR     r0, =SCB_VTOR
    LDR     r0, [r0]
    LDR     r0, [r0]
    NOP
    MSR     msp, r0

    ; enable interrupts at processor level
    CPSIE   F
    CPSIE   I

    ; never reach here!

; compatible with old version
    EXPORT os_hw_interrupt_task_switch
os_hw_interrupt_task_switch:
    BX      lr


    END
