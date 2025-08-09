float foo() { printf("foo called\n"); return 0; }

mfunction return_a_fun() { return foo; }

return_a_fun()();

