struct A {
	struct B b
};

struct B {
	mfunction c
}

float fun() { return 0; }

struct B b = { fun };
struct A a = { b };

a.b.c();

