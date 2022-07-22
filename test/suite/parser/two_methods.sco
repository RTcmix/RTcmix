struct A {
	float _fMember,
	method float firstMethod() { return this._fMember; },
	method string secondMethod(float methodArg) { return "Foobar"; }
};

struct A a = { 11 };

x = a.firstMethod();

if (x != 11) { printf("FAILED\n"); }

y = a.secondMethod();

if (y != "Foobar") { printf("FAILED\n"); }

printf("SUCCEEDED\n");
