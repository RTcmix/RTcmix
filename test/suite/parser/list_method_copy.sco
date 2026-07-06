// Test list .copy() : new top-level list, elements shallow-copied (reference elements shared)

a = { 1, 2, 3 };
b = a.copy();

// 1. Top-level independence: changing copy's element must NOT affect original
b[0] = 99;
if (a[0] != 1) {
	printf("FAILED: list.copy() element is aliased (a[0]=%f)\n", a[0]);
	exit(1);
}

// 2. Reference-type element (nested list) must be SHARED after copy()
inner = { 10, 20 };
c = { inner };
d = c.copy();
inner[0] = 777;
if (d[0][0] != 777) {
	printf("FAILED: nested list element not shared after copy() (d[0][0]=%f)\n", d[0][0]);
	exit(1);
}

printf("SUCCEEDED\n");