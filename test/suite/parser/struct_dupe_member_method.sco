struct Foo
{
	member float fMember(float arg1, float arg2) { return 0; },
	float fMember
}

struct Foo foo;

foo.fMember(1, 2);

