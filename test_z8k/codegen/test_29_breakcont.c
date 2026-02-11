/* Test 29: Break and continue in loops */
f()
{
	int i;
	i = 0;
	while (1) {
		if (i == 5)
			break;
		i = i + 1;
	}
	return i;
}

g()
{
	int i;
	int sum;
	sum = 0;
	i = 0;
	while (i < 10) {
		i = i + 1;
		if (i > 5)
			continue;
		sum = sum + i;
	}
	return sum;
}
