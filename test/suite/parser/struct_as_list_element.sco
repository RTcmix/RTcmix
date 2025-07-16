struct Foo { float f, method float fmethod() { return 0; } };

struct Foo foo;

l = { foo };

l[0].fmethod();

