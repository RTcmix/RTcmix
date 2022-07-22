struct A {
	float _fMember,
	method float firstMethod() { return this._fMember; },
	method float firstMethod(float x) { return x + 77; }
};

struct A a = { 11 };

printf("FAILED - should have noticed error\n");
