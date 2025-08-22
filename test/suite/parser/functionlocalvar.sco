// Check to be sure the symbol for a function-local var is unique
// across recursive calls

recursionLevel = 0;

float functionWithLocalVar(float setIt)
{
	float localVar, scopeNum;
	localVar = setIt;
	++recursionLevel;
	scopeNum = recursionLevel;
	printf("TEST: At scope level %d\n", scopeNum);
	if (recursionLevel < 2) {
		recurseVar = functionWithLocalVar(setIt * 998);
	}
	printf("TEST: Back at scope level %d\n", scopeNum);
	return localVar;
}

printf("TEST: At scope level 0\n");

topLevel = functionWithLocalVar(77);

if (topLevel != 77) {
	error("Problem with recursive function variable scopes!");
}

printf("Done.\n");