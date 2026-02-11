/* Test 37: Mixed-type arithmetic */
long lx;
int ix;
char cx;

f()
{
	ix = 100;
	lx = ix + 1;
	return lx;
}

g()
{
	cx = 10;
	ix = cx + 5;
	return ix;
}

h()
{
	lx = 1000;
	ix = 50;
	lx = lx + ix;
	ix = lx;
	return ix;
}
