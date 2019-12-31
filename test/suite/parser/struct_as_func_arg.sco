struct Foo { float f_member };

struct Foo foo;

foo.f_member = 11;

float passFooAsArgument(struct Foo fooArg) {
	if (fooArg.f_member != 11) {
		printf("ERROR: struct passed as function argument did not preserve member value")
		exit(1)
	}
	return 0;
}

passFooAsArgument(foo);
