/* Test 22: Local variables and stack frame access */
f()
{
	int a, b, c;
	a = 10;
	b = 20;
	c = a + b;
	return c;
}

g(x, y)
int x, y;
{
	int tmp;
	tmp = x;
	x = y;
	y = tmp;
	return x + y;
}
