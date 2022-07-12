struct StructWithMethod {
	float _floatMember,
	method float theMethod(string stringArgument) {
		x = stringArgument;		// this lets us see a normal arg sym lookup
		return this._floatMember;
	}
};

struct StructWithMethod myStruct = { 11.0 };

x = myStruct.theMethod("a string");


