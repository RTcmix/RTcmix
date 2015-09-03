float gDepth;

float dragon(float function_arg, float function_arg2)
{
	float function_body_var;
	function_body_var = 10;
	gDepth += 1;
	if (gDepth < 2) {
		dragon(function_arg, 777);
	}
	return 0;
}

dragon(0, 111);
