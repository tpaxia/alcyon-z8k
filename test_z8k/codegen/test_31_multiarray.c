/* Test 31: Multi-dimensional arrays */
int grid[3][4];

f()
{
	grid[0][0] = 1;
	grid[1][2] = 5;
	grid[2][3] = 9;
	return grid[1][2];
}

g(i, j)
int i, j;
{
	grid[i][j] = 42;
	return grid[i][j];
}
