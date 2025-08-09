// Testing return value of assignments, expressions, etc

float test(float got, float expected) { if (got != expected) { error("FAILED.  expression returned %f, expected %f", got, expected); } return 0; }

float x, y, z;
l1 = {};
l2 = {};

printf("testing simple float expressions\n");
test(x = 10, 10);
test(x = (y = 10), 10);

printf("testing list access expressions\n");
test(l1[0] = 20, 20);
test(x = l1[0], 20);

struct S {
	float f,
	method float set(float inF) { this.f = inF; return this.f; },
	method float get() { return this.f; }
};

struct S s;

printf("testing struct member expressions\n");
test(s.f = 30, 30);
test(z = (s.f = 40), 40);
test(s.set(50), 50);


printf("testing list/struct combinations\n");

l2[0] = s;	// store struct

test(l2[0].f = 60, 60);

struct SS {
	list l
};

ss = SS({});	// initialize with list

test(ss.l[0] = 70, 70);

printf("testing list/method combinations\n");

struct SSS {
	list l
};

sss = SSS({});

test(sss.l[0] = 80, 80);

l3 = { sss };

test(l3[0].l[0] = 90, 90);

printf("testing recursive function pointers\n");

mfunction retfunction1() { return retfunction2; }

mfunction retfunction2() { return retfunction1; }

retfunction1()()()();


