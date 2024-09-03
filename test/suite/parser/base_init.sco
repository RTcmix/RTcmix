struct Base {
	float _f,
	method float _init() { printf("Calling Base.init()\n"); return 0; }
}

struct Derived structbase Base {
	string _s,
	method float _init() { printf("Calling Derived.init()\n"); return 0; }
}

d = Derived(100, "100");

print(d);

printf("SUCCEEDED\n");

