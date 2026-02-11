/* Test 36: Pre/post-increment on complex lvalues */
int arr[5];

struct counter {
	int val;
};

f()
{
	arr[0] = 10;
	arr[0]++;
	return arr[0];
}

g()
{
	arr[1] = 5;
	++arr[1];
	return arr[1];
}

h(p)
struct counter *p;
{
	p->val++;
	return p->val;
}
