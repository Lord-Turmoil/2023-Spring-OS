#include <pash.h>

char buffer[128];

int main(int argc, char* argv[])
{
	input_opt_t opt;

	init_input_opt(&opt);
	opt.minLen = 1;
	opt.maxLen = 127;
	opt.interruptible = 1;

	syscall_putchar('I');
	syscall_putchar('c');
	syscall_putchar('a');
	syscall_putchar('n');
	syscall_putchar('\b');
	syscall_putchar('\b');
	syscall_putchar('y');
	syscall_putchar('\n');
	while (get_string(buffer, &opt) > 0)
	{
		printf("\nI got '%s'\n", buffer);
	}

	return 0;
}
