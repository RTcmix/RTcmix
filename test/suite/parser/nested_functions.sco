float outer(float arg) {
	n = arg;
	return n;
}

float inner(float arg1, float arg2) {
	n = arg1;
	return n + arg2;
}

x = outer(inner(1, 2));

if (x != 3) {
	printf("FAILED\n");
}
else {
	printf("SUCCEEDED\n");
}

