float standalone_function() { return 0; }

standalone_function();

struct ContainsMincfunction {
	string 		_dummyMember,
	mfunction	_functionPointer
};


struct ContainsMincfunction s;

s._functionPointer = standalone_function;

s._functionPointer();


struct ContainsMethod {
	string _dummyMember,
	method float struct_method() { return 0; }
};

struct ContainsMethod c;

c.struct_method();

