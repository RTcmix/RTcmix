// This will succeed due to backwards-compatible if/else scopes

struct Foo { float _f };

if (1) {

	struct Foo fooVarInsideBlock;
	fooVarInsideBlock._f = 9;
}

x = fooVarInsideBlock._f;

