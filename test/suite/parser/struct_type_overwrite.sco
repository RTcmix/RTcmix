struct Foo {
	float f,
	method float fmethod() { return this.f; }
};

struct Bar { string s };

struct Foo foo;

foo.fmethod();

struct Bar bar;

foo = bar;	// overwrite!

foo.fmethod();
