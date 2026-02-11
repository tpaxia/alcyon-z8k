int fact(n)
int n;
{
	int m;
	if (n <= 1)
		return 1;
	m = n - 1;
	return n * fact(m);
}

int main()
{
	return fact(5);
}
