
// If aGlobalVariable has not yet been defined globally,
// this call will auto-declare a function-scope variable only.
// But, if the variable already exists globally, this will
// modify its value!

float aFunction()
{
	aGlobalVariable = 0;
	return 0;
}

aFunction();

aGlobalVariable = 9;		// this will always be legal

aFunction();

if (aGlobalVariable != 0) {
	exit("function did not overwrite global variable as expected");
}

printf("SUCCEEDED\n");
