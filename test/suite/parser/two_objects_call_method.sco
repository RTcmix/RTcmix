struct StructWithMethod {
	float _floatMember,
	method float theMethod() {
		return this._floatMember;
	}
};

struct StructWithMethod myStruct1, myStruct2;

myStruct1.theMethod();

myStruct2.theMethod();
