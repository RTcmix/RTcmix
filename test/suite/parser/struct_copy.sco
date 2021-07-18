struct Foo { float f_member };

struct Foo foo;

// Make x share foo
x = foo;

x.f_member = 7;

if (x.f_member != foo.f_member) {
	printf("FAILED: copied struct object is not copying member values!")
	exit(1);
}

printf("SUCCEEDED\n");
