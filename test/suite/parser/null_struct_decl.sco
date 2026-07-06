// Test: a struct declared with "= null" is a typed NULL struct.

struct Foo {
	float f
};

// Declare a NULL struct (no members instantiated).
struct Foo foo = null;

// A NULL struct tests false and compares equal to 0.
if (foo) { error("null struct should test false"); }
if (foo != 0) { error("null struct should compare == 0"); }

// Assigning a real struct makes it non-null and usable.
struct Foo realFoo;
realFoo.f = 42;
foo = realFoo;

if (!foo) { error("assigned struct should test true"); }
if (foo == 0) { error("assigned struct should not compare == 0"); }
if (foo.f != 42) { error("member access after assignment failed"); }

// A second variable may be declared null in the same way.
struct Foo bar = null;
if (bar) { error("second null struct should test false"); }

printf("null_struct_decl SUCCEEDED\n");