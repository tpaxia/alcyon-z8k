/* Test 07: Shift and bitwise operations */
int x, y;

f()
{
	x = y << 2;
	x = x >> 1;
	x = x ^ y;
	x = ~x;
	return x;
}
