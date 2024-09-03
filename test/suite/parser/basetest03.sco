struct Foo {
	float f, 
	method float printMe() { printf("f = %f\n", this.f); return this.f; }
};

struct Bar structbase Foo {
	string s,
	method float describe() { return this.printMe(); }
};

f = Foo(99);

b = Bar(999, "hello");	// needs initializer for base as well as us

b.describe();

printf("SUCCEEDED\n");

