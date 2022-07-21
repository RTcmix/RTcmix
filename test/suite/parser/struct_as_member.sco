// testing struct member of another struct

struct Inner {
	float c
};

struct Inner b;

struct Outer {
	struct Inner b
};

struct Outer a = { b };

a.b.c = 777;

if (a.b.c != 777) {
	printf("FAILED\n");
}
else {
	printf("SUCCEEDED\n");
}



