float standaloneFunction() { return 8; }

struct StructWithMethod {
	float _floatMember,
	method float theMethod() {
		return this._floatMember;
	}
};

standaloneFunction();

struct StructWithMethod myStruct;

myStruct.theMethod();
