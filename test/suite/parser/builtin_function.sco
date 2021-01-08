// basic simple test of builtin function

// this function will fail if its argument is zero, so a good test of whether
// args are being correctly passed to the builtin routines

reset(99999);

// this is a non-defined function and so should just warn us

not_defined_function(3, 4, 5);

printf("SUCCEEDED\n");
