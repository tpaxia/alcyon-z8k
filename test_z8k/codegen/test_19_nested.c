/* Test 19: Nested expressions and complex control flow */
int x, y, z;

f()
{
	x = (y + z) * (y - z);
}

g(a, b, c)
int a, b, c;
{
	if (a > b) {
		if (b > c)
			return a;
		return b;
	}
	return c;
}
