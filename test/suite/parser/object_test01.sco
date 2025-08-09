// testing parser's 'obj' type

struct StructType {
	float	_member
};

struct StructType2 {
	struct StructType			_structMember
};

st = StructType(11);
st2 = StructType2(st);

// test dot operator on RHS

x = st2._structMember._member;

// test dot operator on LHS

st2._structMember = st;

st2._structMember._member = 9;

if (st2._structMember._member != 9) { error("expression 'a.b.c = e' failed to evaluate"); }


// test method calls

struct StructType3 {
	float			_member,
	mfunction		_structFunction,
	method float 	_get() { return _this.member; }
};

struct StructType4 {
	struct StructType3			_structMember,
	method struct StructType3	_get() { return this._structMember; }
};

string foo() { return "foo"; }

st4 = StructType4(StructType3(11, foo));

// sanity check

r = st4._get();
st4._get();		// method call as stmt

// dot-on-dot for member function pointer (requires no 'this')

rr = st4._get()._structFunction();

// dot-on-dot for method call (requires 'this')

//rrr = st4._get()._get();

