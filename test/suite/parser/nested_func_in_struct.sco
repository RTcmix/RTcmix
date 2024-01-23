struct A {
	struct B b
};

struct B {
	mfunction c
}

float fun() { return 7; }

struct B b = { fun };
struct A a = { b };

val = a.b.c();

if (val != 7)
{
	error("nested mfunction call failed to return correct value");
}

