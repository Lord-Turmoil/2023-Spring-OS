#include <lib.h>

// ampersand 1000 a
int main(int argc, char* argv[])
{
	if (argc != 3)
	{
		printf("Usage: ampersand n c\n");
		return 1;
	}

	int delay = atoi(argv[1]);
	if (delay == 0)
		delay = 1000;

	sleep(delay);

	printf("%s\n", argv[2]);

	return 0;
}
