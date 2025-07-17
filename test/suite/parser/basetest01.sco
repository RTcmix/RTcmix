// Simplest possible, with no members

struct Base {
	method float base() { return 0; }
};

struct Derived structbase Base {
	method float derived() { return 0; }
};

struct Derived d;

dd = Derived();

printf("SUCCEEDED\n");

