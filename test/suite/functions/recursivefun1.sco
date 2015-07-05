float recursioncount;
recursioncount = 10;

float recursivefunction(float arg)
{
	printf("recursivefunction count %d\n", recursioncount);
	if (recursioncount == 0) {
		return arg;
	}
	recursioncount -= 1;
	ret = recursivefunction(arg + 1);
	return ret;
}

y = recursivefunction(100);

print(y);
