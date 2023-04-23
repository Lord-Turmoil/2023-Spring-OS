#include <env.h>
#include <lib.h>
#include <mmu.h>

void exit(void) {
	// After fs is ready (lab5), all our open files should be closed before dying.
#if !defined(LAB) || LAB >= 5
	close_all();
#endif

	syscall_env_destroy(0);
	user_panic("unreachable code");
}

volatile struct Env *env;
extern int main(int, char **);

/*
 * This function is the wrapper of main. You know, when we compile a 
 * C program, the compiler will add our main into invoke_main, and here
 * it is so-called libmain. So this file is used separately when compile
 * user's source file.
 */
void libmain(int argc, char **argv) {
	// set env to point at our env structure in envs[].
	env = &envs[ENVX(syscall_getenvid())];

	// call user main routine
	main(argc, argv);

	// exit gracefully
	exit();
}
