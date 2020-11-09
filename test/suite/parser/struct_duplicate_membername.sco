struct Foo { float f, handle f };

struct Foo foo;

foo.f = 0;

printf("FAILED - should not have made it to the end\n");
