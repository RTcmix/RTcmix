float aFunction(float arg) { return arg % 2; }

struct A {
	float _fMember,
	method float firstMethod() { return aFunction(this._fMember); }
};

struct A a = { 11 };

print(a);

x = a.firstMethod();

print(x);

if (x != 1) { exit("FAILED"); }

printf("SUCCEEDED\n");
