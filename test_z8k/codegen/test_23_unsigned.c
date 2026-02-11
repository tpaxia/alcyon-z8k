/* Test 23: Unsigned operations — different compare and shift */
unsigned x, y;

f()
{
	if (x > y)
		return 1;
	return 0;
}

g()
{
	return x >> 2;
}
