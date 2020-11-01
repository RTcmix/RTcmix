float outer(float arg) {
	n = arg;
	return n;
}

float inner(float arg) {
	n = arg;
	return n + 1;
}

x = outer(inner(1));

if (x != 2) {
	printf("FAILED\n");
}
else {
	printf("SUCCEEDED\n");
}

