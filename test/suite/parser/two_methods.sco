struct AStruct {
	float _fMember,
	method float firstMethod() { return this._fMember; },
	method string secondMethod(float methodArg) { return "Foobar"; }
};

struct AStruct aVariable = { 11 };

x = aVariable.firstMethod();

if (x != 11) { exit("FAILED"); }

y = aVariable.secondMethod(0);

if (y != "Foobar") { exit("FAILED"); }

printf("SUCCEEDED\n");
