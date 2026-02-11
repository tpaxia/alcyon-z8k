/* Test 25: Function pointers and indirect calls */
int add(a, b)
int a, b;
{
	return a + b;
}

int call_fn(fp, x, y)
int (*fp)();
int x, y;
{
	return (*fp)(x, y);
}

int main()
{
	int (*fp)();
	fp = add;
	return call_fn(fp, 10, 20);
}
