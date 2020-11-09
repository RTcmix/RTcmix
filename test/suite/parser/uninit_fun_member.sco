// testing uninitalized function object in struct

// struct holding function

struct HoldFun { mfunction mFunc };

// declare struct

struct HoldFun funHolder;

// call it via struct

s = funHolder.mFunc("hello ", "world");


// we shouldn't get here
printf("FAILED - null function should have produced error\n");

