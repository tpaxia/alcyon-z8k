/* Test 35: Address-of on locals, arrays, functions */
int arr[5];

int add(a, b)
int a, b;
{
	return a + b;
}

f()
{
	int x;
	int *p;
	x = 42;
	p = &x;
	return *p;
}

g()
{
	int *p;
	p = &arr[2];
	*p = 77;
	return arr[2];
}

h()
{
	int (*fp)();
	fp = add;
	return (*fp)(3, 4);
}
