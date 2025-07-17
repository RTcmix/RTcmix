// This code is legal

struct Base {
	float _f
};

struct Derived structbase Base {
	method float fun() { return 0; }
};

struct Derived d;	// no initializer provided

printf("d.fun() = %d\n", d.fun());

printf("SUCCEEDED\n");

