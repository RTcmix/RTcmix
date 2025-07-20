// testing reporting of secondary-level function calls for errors

float topLevelFunction()
{
	x = midLevelFunction();
	return x;
}

float midLevelFunction() {
	x = functionCalledIncorrectly(666);
	return x;
}

float functionCalledIncorrectly(string expectedStringArg)
{
	return 0;
}

topLevelFunction();

