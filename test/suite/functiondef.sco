print_on(5);

// We define a function here

float userFunction(float arg)
{
	return (arg * arg) / 2;
}

// We use it locally for sanity check

printf("local use\n");
x = userFunction(7);
printf("x = %f\n", x);


global_var_from_other_file = 99;	// Sanity check for second file
