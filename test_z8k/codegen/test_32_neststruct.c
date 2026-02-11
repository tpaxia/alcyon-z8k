/* Test 32: Nested structs */
struct point {
	int x;
	int y;
};

struct rect {
	struct point tl;
	struct point br;
};

f()
{
	struct rect r;
	r.tl.x = 1;
	r.tl.y = 2;
	r.br.x = 10;
	r.br.y = 20;
	return r.br.x - r.tl.x;
}

g(rp)
struct rect *rp;
{
	return rp->br.y - rp->tl.y;
}
