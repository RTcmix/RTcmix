struct Argument {
	float _member,
	method float get() { return this._member; }
};

struct Caller {
	float _cMember,
	method float methodWithArgument(float arg0) { this._cMember = arg0; return this._cMember; }
};

struct Argument arg = { 11 };

struct Caller caller;

// Testing call into method as part of another method's argument list

caller.methodWithArgument(arg.get());

