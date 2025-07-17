// This code is legal with warning message

struct Base {
	float _f
};

struct Derived structbase Base {
	method float fun() { return 0; }
};

d = Derived();	// no initializer provided!

printf("d.fun() = %d\n", d.fun());

printf("SUCCEEDED\n");

