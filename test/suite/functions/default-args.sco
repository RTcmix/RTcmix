float function(float arg1, float arg2, float arg3)
{
	printf("arg1: %f, arg2: %f, arg3: %f\n", arg1, arg2, arg3);
	return 0;
}

ret = function(33);		// should set arg2 and arg3 to 0.0

