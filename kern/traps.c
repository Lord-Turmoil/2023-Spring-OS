#include <env.h>
#include <pmap.h>
#include <printk.h>
#include <trap.h>

extern void handle_int(void);
extern void handle_tlb(void);
extern void handle_sys(void);
extern void handle_mod(void);
extern void handle_reserved(void);
extern void handle_ov(void);

void (*exception_handlers[32])(void) = {
    [0 ... 31] = handle_reserved,
    [0] = handle_int,
    [2 ... 3] = handle_tlb,
#if !defined(LAB) || LAB >= 4
    [1] = handle_mod,
    [8] = handle_sys,
#endif
	[12] = handle_ov
};

/* Overview:
 *   The fallback handler when an unknown exception code is encountered.
 *   'genex.S' wraps this function in 'handle_reserved'.
 */
void do_reserved(struct Trapframe *tf) {
	print_tf(tf);
	panic("Unknown ExcCode %2d", (tf->cp0_cause >> 2) & 0x1f);
}

void do_ov(struct Trapframe* tf)
{
	curenv->env_ov_cnt++;

	unsigned long instr = *((unsigned long*)tf->cp0_epc);
	unsigned long type = (instr >> 26) & 0x3f;
	unsigned long opt = (instr) & 0x7ff;

	if (type == 0x0)	// r
	{
		if (opt == 0x20)	// add
		{
			instr = (instr & (~0x7ff)) | 0x21;
			printk("add ov handled\n");
		}
		else if (opt == 0x22)	// sub
		{
			instr = (instr & (~0x7ff)) | 0x23;
			printk("sub ov handled\n");
		}

		u_long va = tf->cp0_epc;
		struct Page* pp = page_lookup(curenv->env_pgdir, va, NULL);
		panic_on(pp == NULL);
		*(u_long*)(page2kva(pp) + PPO(va)) = instr;
	}
	else if (type == 0x8)	// i
	{
		unsigned long imm = instr & 0xffff;
		unsigned long reg_s = (instr >> 21) & 0x1f;
		unsigned long reg_t = (instr >> 16) & 0x1f;
		tf->regs[reg_t] = tf->regs[reg_s] / 2ul + imm / 2ul;
		tf->cp0_epc += 4;
		printk("addi ov handled\n");
	}
}

