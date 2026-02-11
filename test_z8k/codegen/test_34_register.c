/* Test 34: Register variables */
f()
{
	register int x;
	register int y;
	x = 10;
	y = 20;
	return x + y;
}

g(n)
int n;
{
	register int sum;
	register int i;
	sum = 0;
	i = 1;
	while (i <= n) {
		sum = sum + i;
		i = i + 1;
	}
	return sum;
}
