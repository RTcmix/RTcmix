// testing uninitalized function

mfunction mFunc;

mFunc("hello ", "world");

// we shouldn't get here
printf("FAILED - null mfunction should have produced error\n");

