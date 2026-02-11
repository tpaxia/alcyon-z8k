/* Test 14: Structs — member access */
struct point {
	int x;
	int y;
};

struct point pt;

f()
{
	pt.x = 10;
	pt.y = 20;
	return pt.x + pt.y;
}

g(pp)
struct point *pp;
{
	pp->x = 5;
	return pp->y;
}
