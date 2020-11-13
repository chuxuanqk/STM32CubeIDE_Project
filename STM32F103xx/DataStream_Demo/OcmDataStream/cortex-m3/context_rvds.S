

;/*
; * ocm_base_t rt_hw_interrupt_disable();
; */
ocm_hw_interrupt_disable    PROC
    EXPORT  ocm_hw_interrupt_disable
    MRS     r0, PRIMASK
    CPSID   I
    BX      LR
    ENDP

;/*
; * void ocm_hw_interrupt_enable(ocm_base_t level);
; */
ocm_hw_interrupt_enable    PROC
    EXPORT  ocm_hw_interrupt_enable
    MSR     PRIMASK, r0
    BX      LR
    ENDP