struct Foo {
	float f,
	method float fmethod() { return this.f; }
};

struct Foo foo;

foo.fmethod();

foo = 0;		// Cant do this

foo.fmethod();
