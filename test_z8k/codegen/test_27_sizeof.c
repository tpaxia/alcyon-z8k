/* Test 27: Sizeof operator */
struct point {
	int x;
	int y;
};

f()
{
	int a;
	a = sizeof(int);
	a = a + sizeof(char);
	a = a + sizeof(long);
	return a;
}

g()
{
	return sizeof(struct point);
}
