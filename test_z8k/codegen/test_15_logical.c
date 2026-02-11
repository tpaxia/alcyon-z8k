/* Test 15: Logical operators — &&, ||, ! */
int a, b;

f()
{
	if (a && b)
		return 1;
	return 0;
}

g()
{
	if (a || b)
		return 1;
	return 0;
}

h()
{
	return !a;
}
