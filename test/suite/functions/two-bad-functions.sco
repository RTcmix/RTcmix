float bad_function_01(float arg)
{
	return arg * x;
}

float bad_function_02(float arg)
{
	float x;
	if (arg > 0) {
		x = bad_function_01(33);
	}
	else {
		x = 999;
	}
	return x;
}

bad_function_02(34);

