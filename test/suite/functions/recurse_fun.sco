float fun3(float x) { return x; }
float fun2(float x) { return fun3(x*10); }
float fun1(float x) { return fun2(x*10); }

fun1(10);


