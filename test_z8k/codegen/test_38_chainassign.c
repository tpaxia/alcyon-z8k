/* Test 38: Chained assignment */
int a, b, c;

f()
{
	a = b = c = 5;
	return a + b + c;
}

g()
{
	int x, y;
	x = y = 10;
	return x + y;
}
