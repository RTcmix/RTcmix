float depth;

list beatNote(float beats)
{
	list lll;
	return lll;	// returns NULL list
}

list eighths(float beats)
{
	printf("<<<eighths depth %f>>\n", depth);
	if (depth > 1) {
		printf("hit depth - returning empty list to caller\n");
		printf("    <<<exiting eighths depth %f>>>\n", depth);
		return {};
	}
	++depth;
	list _out;
	if (chance(1, 2)) {
		printf("writing results of recursive call into _out[0]\n");
		_out[0] = eighths(1);
	}
	else {
		printf("writing results of beatNode call into _out[1]\n");
		_out[1] = beatNote(1);
	}
	--depth;
	printf("    <<<exiting eighths depth %f>>>\n", depth);
	return _out;
}

for (n = 0; n < 10000; ++n) {
	l = eighths(pickrand(2, 3, 4));
}

printf("SUCCEEDED\n");
