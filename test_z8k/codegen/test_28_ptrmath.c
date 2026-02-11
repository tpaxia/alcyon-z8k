/* Test 28: Pointer arithmetic */
int arr[5];

f()
{
	int *p;
	p = &arr[0];
	*p = 10;
	p = p + 1;
	*p = 20;
	return *(p - 1) + *p;
}

g()
{
	int *p;
	int *q;
	p = &arr[0];
	q = &arr[3];
	return q - p;
}
