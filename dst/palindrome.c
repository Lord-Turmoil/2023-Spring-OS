#include <stdio.h>

int IsPalindrome(int number);
int main()
{
	int n;
	scanf("%d", &n);

	if (IsPalindrome(n))
	{
		printf("Y\n");
	}
	else
	{
		printf("N\n");
	}

	return 0;
}

int IsPalindrome(int number)
{
	char buffer[16];
	char* front;
	char* back;

	sprintf(buffer, "%d", number);
	front = back = buffer;
	while (*back)
		++back;
	--back;
	while (front < back)
	{
		if (*front != *back)
			return 0;
		++front;
		--back;
	}

	return 1;
}