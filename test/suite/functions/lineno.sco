float ten_line_func(float arg) {
	float f, g, h;
	if (arg % 3) {
		f = g = h = arg * 8 * z;
	}
	else {
		f = g = h = arg / 8;
	}
	return f;
}
print(ten_line_func(10));
