float gDepth;

float dragon(float function_arg)
{
	float function_body_var;
	function_body_var = function_arg+10;
	gDepth += 1;
	if (gDepth < 2) {
		dragon(function_arg);
	}
	return 0;
}

dragon(0);
