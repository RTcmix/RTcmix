float aFunction(float arg) { return arg % 2; }

struct A {
	float _fMember,
	method float firstMethod() { return aFunction(this._fMember); }
};

struct A a = { 11 };

x = a.firstMethod();

if (x != 11) { printf("FAILED\n"); }

printf("SUCCEEDED\n");
