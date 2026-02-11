/* Test 30: Named constants (enum-like via #define) */
#define RED   0
#define GREEN 1
#define BLUE  2

f()
{
	int c;
	c = GREEN;
	if (c == BLUE)
		return 1;
	return c;
}

g()
{
	int c;
	c = RED;
	c = c + 1;
	return c;
}
