struct Foo { float _f };

if (1) {

	struct Foo fooVarInsideBlock;
	fooVarInsideBlock._f = 9;

}

x = fooVarInsideBlock._f;

printf("FAILED - should have generated error when accessing struct var outside of if() block\n");
