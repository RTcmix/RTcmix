// Here we use a function we defined in another score which was fed to the
// system

y = global_var_from_other_file;		// If cross-score globals fail, this would too

printf("use in second score\n");
x = userFunction(7);
printf("x = %f\n", x);
