// testing reporting of top-level function calls for errors

float functionWithError()
{
	undefinedList[0] = 1;
	return 0;
}

functionWithError();

