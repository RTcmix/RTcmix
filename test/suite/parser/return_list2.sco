float depth;

list eighths()
{
	printf("<<<eighths depth %f>>\n", depth);
	if (depth > 1) {
		printf("hit depth - returning empty list to caller\n");
		printf("    <<<exiting eighths depth %f>>>\n", depth);
		return {};
	}
	++depth;
	list _out;
	printf("writing results of recursive call into _out[0]\n");
	_out[0] = eighths();
	--depth;
	printf("    <<<exiting eighths depth %f>>>\n", depth);
	return _out;
}

for (n = 0; n < 1000; ++n) {
	printf("---- ITERATION %f ----\n", n);
	l = eighths();
}

printf("SUCCEEDED\n");
