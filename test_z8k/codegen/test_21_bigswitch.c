/* Test 21: Large switch — tests jump table path (ncases > 4) */
f(x)
int x;
{
	switch (x) {
	case 0: return 10;
	case 1: return 11;
	case 2: return 12;
	case 3: return 13;
	case 4: return 14;
	default: return 0;
	}
}
