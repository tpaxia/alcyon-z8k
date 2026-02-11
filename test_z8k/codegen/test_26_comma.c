/* Test 26: Comma operator */
int x, y;

f()
{
	x = (1, 2, 3);
	return x;
}

g()
{
	x = 0;
	y = 0;
	x = (y = 5, y + 1);
	return x;
}
