/* Test 33: Static local variables */
f()
{
	static int count;
	count = count + 1;
	return count;
}

g()
{
	static int val = 10;
	val = val + 5;
	return val;
}
