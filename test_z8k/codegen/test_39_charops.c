/* Test 39: Character arithmetic and comparison */
char cx, cy;

f()
{
	cx = 'A';
	cy = cx + 1;
	return cy;
}

g()
{
	cx = 'z';
	if (cx >= 'a' && cx <= 'z')
		return 1;
	return 0;
}

h()
{
	cx = 'A';
	cx = cx + 32;
	return cx;
}
