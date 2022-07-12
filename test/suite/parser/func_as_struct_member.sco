float a_func() { printf("a_func called\n"); return 7; }

struct Foo {
	mfunction	memberFunc
};

struct Foo foo = { a_func };

foo.memberFunc();

	
