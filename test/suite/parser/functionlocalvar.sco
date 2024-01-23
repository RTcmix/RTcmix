// Check to be sure the symbol for a function-local var is unique
// across recursive calls

recursionLevel = 0;

float functionWithLocalVar(float setIt)
{
	localVar = setIt;
	++recursionLevel;
	if (recursionLevel < 2) {
		recurseVar = functionWithLocalVar(setIt * 998);
	}
	return localVar;
}

topLevel = functionWithLocalVar(77);

if (topLevel != 77) {
	error("Problem with recursive function variable scopes!");
}

printf("Done.\n");