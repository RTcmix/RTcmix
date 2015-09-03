// Global declared in advance of use in function

float gFloatVar;
gFloatVar = 10;

float fun()
{
	gFloatVar += 1;
	return 0;
}

fun();

if (gFloatVar != 11) exit("function 'fun' did not increment pre-declared global", -1);

// Global declared *after* its use in function

float fun2()
{
	gFloatVar2 += 1;
	return 0;
}

float gFloatVar2;

gFloatVar2 = 20;

fun2();

if (gFloatVar2 != 21) exit("function 'fun2' did not increment post-declared global", -1);

// Bad idea: declaring a var inside a function with same name as global.
// But it should create a unique-scoped variable.

float fun3()
{
	float gFloatVar;
	gFloatVar = 33;
	return 0;
}

fun3();

if (gFloatVar == 33) exit("function 'fun3' did not unique-scope a variable", -1);

