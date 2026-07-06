// Test map .copy() : new top-level map, values shallow-copied (reference values shared)

map m;
m["k"] = 5;
n = m.copy();

// 1. Top-level independence: changing copy's entry must NOT affect original
n["k"] = 8;
if (m["k"] != 5) {
	printf("FAILED: map.copy() entry is aliased (m[k]=%f)\n", m["k"]);
	exit(1);
}

// 2. Reference-type value (list) must be SHARED after copy()
inner = { 10, 20 };
map mm;
mm["list"] = inner;
nn = mm.copy();
inner[0] = 777;
if (nn["list"][0] != 777) {
	printf("FAILED: map value list not shared after copy() (nn[list][0]=%f)\n", nn["list"][0]);
	exit(1);
}

printf("SUCCEEDED\n");