/* Test 09: Post-increment and pre-increment */
int x, y;

f()
{
	x = 0;
	y = x++;
	y = ++x;
	y = x--;
	return y;
}
