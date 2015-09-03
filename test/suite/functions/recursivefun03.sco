float gDepth;

float dragon(float function_arg)
{
	float function_body_var, function_body_var2;
	function_body_var = 10;
	function_body_var2 = 11;
	gDepth += 1;
	if (gDepth < 2) {
		dragon(1);
	}
	return 0;
}

dragon(0);
