/* Test 11: Loops — while, for, do-while */
int sum;

f()
{
	int i;
	sum = 0;
	i = 0;
	while (i < 10) {
		sum += i;
		i++;
	}
}

g()
{
	int i;
	i = 5;
	do {
		i--;
	} while (i > 0);
}
