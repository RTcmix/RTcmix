struct StructWithMethod {
	float _floatMember,
	method float theMethod() {
		return this._floatMember;
	}
};

struct StructWithMethod myStruct = { 11.0 };

x = myStruct.theMethod();

struct StructWithMethodTakingArgument {
	string _stringMember,
	method float theMethod(string stringArgument) {
		this._stringMember = stringArgument;
		return this._stringMember;
	}
};

struct StructWithMethodTakingArgument myStruct2 = { "This is a string" };

y = myStruct2.theMethod("another string");

print(y);
