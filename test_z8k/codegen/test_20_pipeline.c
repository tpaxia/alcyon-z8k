/* Test 20: Full pipeline — assemble and link a complete program */
int result;

int add(a, b)
int a, b;
{
	return a + b;
}

int main()
{
	result = add(3, 4);
	return result;
}
