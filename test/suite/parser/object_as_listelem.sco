// testing parser's 'obj' type

struct StructType {
	float			_member,
	method string	_method() { return 0; },
	method float 	_get() { return this._member; }
};

// Create object
s = StructType(11);

s._method();
x = s._get();

// store object into list and read out

l = {};

l[0] = s;

ss = l[0];

// re-test

ss._method();
x = ss._get();

// now access object member and method directly from array

y = l[0]._member;

z = l[0]._method();

q = l[0]._get();	// tests access to 'this'

