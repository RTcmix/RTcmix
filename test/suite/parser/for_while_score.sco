global_var = 7;

for (i = 0; i < 10; ++i) {
	y = global_var + i;		// Both RHS vars should be visible to this line
}

// Nested for loop!

for (n = 0; n < 2; ++n) {
	outer_loop_var = 7;
	for (i = 0; i < 10; ++i) {
		y = global_var + outer_loop_var + i + n;		// ALL RHS vars should be visible to this line
	}
}

i = 0;

while (i < 10) {
	y = global_var + i;		// Both RHS vars should be visible to this line
	++i;
}

printf("SUCCEEDED\n");
