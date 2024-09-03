// Same method declared on base and derived - only derived gets called

struct Base {
	method float aMethod() { return 0; }
};

struct Derived structbase Base {
	method float aMethod() { return 0; }
};

struct Derived d;

d.aMethod();

printf("SUCCEEDED\n");
