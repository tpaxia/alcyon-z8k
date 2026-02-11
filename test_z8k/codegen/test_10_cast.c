/* Test 10: Type conversions */
long lx;
int ix;
char cx;

f()
{
	lx = ix;
	ix = lx;
	cx = ix;
	return cx;
}
