// Test struct .copy() : new top-level instance, members shallow-copied (reference members shared)

struct Bar { float f, list l };

l = { 10, 20, 30 };
struct Bar a = { 1.0, l };

b = a.copy();

// 1. Top-level independence: mutating copy's float must NOT affect original
b.f = 99;
if (a.f != 1.0) {
	printf("FAILED: copy() float member is aliased (a.f=%f)\n", a.f);
	exit(1);
}
if (b.f != 99) {
	printf("FAILED: copy() did not retain its own float (b.f=%f)\n", b.f);
	exit(1);
}

// 2. Reference-type member (list) must be SHARED between original and copy (shallow)
b.l[0] = 777;
if (a.l[0] != 777) {
	printf("FAILED: list member not shared after copy() (a.l[0]=%f)\n", a.l[0]);
	exit(1);
}

printf("SUCCEEDED\n");