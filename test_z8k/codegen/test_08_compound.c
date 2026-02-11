/* Test 08: Compound assignment */
int x;

f()
{
	x = 10;
	x += 5;
	x -= 3;
	x &= 7;
	x |= 16;
	return x;
}
