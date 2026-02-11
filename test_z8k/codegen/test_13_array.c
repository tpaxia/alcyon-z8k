/* Test 13: Arrays — indexing and address computation */
int arr[10];

f()
{
	arr[0] = 1;
	arr[5] = 2;
	arr[0] = arr[5];
}

g(i)
int i;
{
	return arr[i];
}
