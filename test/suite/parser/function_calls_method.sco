struct A {
	float _fMember,
	method float firstMethod() { return this._fMember; }
};

struct A a = { 11 };

float x;

float aFunction(struct A aArg) {
	x = aArg.firstMethod();
	return x;
}

aFunction(a);

if (x != 11) { printf("FAILED\n"); }

printf("SUCCEEDED\n");
