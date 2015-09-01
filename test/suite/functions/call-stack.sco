depth = 0;

float fun() {
	if (depth < 5) {
		depth += 1;
		fun();
	}
	return depth;
}

fun();

