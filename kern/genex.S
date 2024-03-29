#include <asm/asm.h>
#include <drivers/dev_rtc.h>
#include <stackframe.h>

/*
 * See include/asm.h for its definition.
 * NESTED(symbol, framesize, rpc)
 */

.macro BUILD_HANDLER exception handler
NESTED(handle_\exception, TF_SIZE + 8, zero)
	/* See entry.S, $sp is (KSTACKTOP - TF_SIZE), which is our Trapframe. */
	move    a0, sp
	addiu   sp, sp, -8	/* 8 bytes aligned */
	jal     \handler
	addiu   sp, sp, 8  /* Here, sp is restored to (KSTACKTOP - TF_SIZE) */
	j       ret_from_exception
END(handle_\exception)
.endm

.text

FEXPORT(ret_from_exception)
	RESTORE_SOME  /* sp remains unchanged as (KSTACKTOP - TF_SIZE) */
	lw      k0, TF_EPC(sp)
	lw      sp, TF_REG29(sp) /* Deallocate stack */
.set noreorder
	jr      k0
	rfe
.set reorder

NESTED(handle_int, TF_SIZE, zero)
	mfc0    t0, CP0_CAUSE
	mfc0    t2, CP0_STATUS
	and     t0, t2
	andi    t1, t0, STATUS_IM4
	bnez    t1, timer_irq
	// TODO: handle other irqs
timer_irq:
	sw      zero, (KSEG1 | DEV_RTC_ADDRESS | DEV_RTC_INTERRUPT_ACK)
	li      a0, 0
	j       schedule
END(handle_int)

BUILD_HANDLER tlb do_tlb_refill

#if !defined(LAB) || LAB >= 4
BUILD_HANDLER mod do_tlb_mod
BUILD_HANDLER sys do_syscall
#endif

BUILD_HANDLER reserved do_reserved
