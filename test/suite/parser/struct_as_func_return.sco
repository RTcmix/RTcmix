struct Foo { float f_member };

struct Foo gFoo;	// global: This test is not testing structs declared inside a function

struct Foo returnAFoo() {
	gFoo.f_member = 11;
	return gFoo;
}

x = returnAFoo();

if (x.f_member != 11) {
	printf("FAILED: struct returned via function call did not preserve member value\n")
	exit(1)
}

printf("SUCCEEDED\n");
