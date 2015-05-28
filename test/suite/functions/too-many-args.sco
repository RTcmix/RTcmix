float function(float arg1, float arg2)
{
	printf("arg1: %f, arg2: %f\n", arg1, arg2);
	return 0;
}

ret = function(33, 44, 55);		// should fail due to too many args
