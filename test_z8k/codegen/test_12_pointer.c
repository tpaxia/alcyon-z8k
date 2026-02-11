/* Test 12: Pointers — address-of, dereference, pointer arithmetic */
int x;
int *p;

f()
{
	p = &x;
	*p = 42;
	x = *p + 1;
}
