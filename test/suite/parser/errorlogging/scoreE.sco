
include scoreC.sco

float scoreEError()
{
	printf("error in scoreEError, line 6\n"); ++e;
	return 0;
}

if (testlevel==7) { printf("calling scoreEError on line 10 of scoreE.sco\n"); scoreEError(); }

