struct Foo { float f, method float fmethod() { return 0; } };

struct Foo foo;

map m;

m["fmethod"] = foo;

m["fmethod"].fmethod();

