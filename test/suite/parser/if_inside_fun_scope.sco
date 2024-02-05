// If() scopes are ignored -- but not inside a Minc function call!

float aMincFunction()
{
	if (1) {
		ifInsideFunctionVar = 9;
	}
	print(ifInsideFunctionVar);		// should fail
	return 0;
}

aMincFunction();

