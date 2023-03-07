#include <asm/asm.h>
#include <env.h>
#include <kclock.h>
#include <pmap.h>
#include <printk.h>
#include <trap.h>

// When build with 'make test lab=?_?', we will replace your 'mips_init' with a generated one from
// 'tests/lab?_?'.
#ifdef MOS_INIT_OVERRIDDEN
#include <generated/init_override.h>
#else

#define _INTERNAL

void mips_init() {
	printk("init.c:\tmips_init() is called\n");

#ifdef _INTERNAL
	printk("\n");
	printk("%%[flags][width][length]<specifier>: [%-10ld]\n", 75159);
	printk("%%[flags][width][length]<specifier>: [%010ld]\n", -75159);
	printk("%%[flags][width][------]<specifier>: [%-10x]\n", 66);
	printk("%%[flags][-----][length]<specifier>: [%-lX]\n", 66);
	printk("%%[-----][width][length]<specifier>: [%10lo]\n", 66);
	printk("%%[flags][-----][------]<specifier>: [%10O]\n", 66);
	printk("%%[-----][width][------]<specifier>: [%10u]\n", 66);
	printk("%%[-----][-----][length]<specifier>: [%lU]\n", 66);
	printk("%%[-----][-----][------]<specifier>: [%d]\n\n", -1957);
	
	printk("%%[flags][width][------]<--------->: [%-10]\n\n", 456);	// error

	printk("%%[-----][width][------]<specifier>: [%20s]\n", "Test String");
	printk("%%[-----][-----][------]<specifier>: [%s]\n\n", "Test String");

	printk("%%[-----][-----][------]<specifier>: [%d]%s[%X]\n\n", 2015, " VS ", 1986);
#endif

	// lab2:
	// mips_detect_memory();
	// mips_vm_init();
	// page_init();

	// lab3:
	// env_init();

	// lab3:
	// ENV_CREATE_PRIORITY(user_bare_loop, 1);
	// ENV_CREATE_PRIORITY(user_bare_loop, 2);

	// lab4:
	// ENV_CREATE(user_tltest);
	// ENV_CREATE(user_fktest);
	// ENV_CREATE(user_pingpong);

	// lab6:
	// ENV_CREATE(user_icode);  // This must be the first env!

	// lab5:
	// ENV_CREATE(user_fstest);
	// ENV_CREATE(fs_serv);  // This must be the second env!
	// ENV_CREATE(user_devtst);

	// lab3:
	// kclock_init();
	// enable_irq();

	while (1) {
	}
}

#endif
