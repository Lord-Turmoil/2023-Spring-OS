#include <lib.h>
#include <conio.h>

int main(int argc, char* argv[])
{
	int ch;

	printf("Press 'Q' (uppercase) to exit\n");
	printf("Press 'W' (uppercase) for a separator\n");
	for (; ; )
	{
		ch = getch();
		if (ch == 'W')
		{
			printf("------------------------------\n");
			continue;
		}
		printf("%c is %d\n", ch, ch);
		if (ch == 'Q')
			break;
	}

	return 0;
}
