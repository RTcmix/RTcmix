float outer(float arg) {
	n = arg;
	return n;
}

float inner(float arg) {
	n = arg;
	return n + 1;
}

x = outer(inner(1));

