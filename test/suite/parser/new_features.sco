/* testing new parser features */

/*
l = { 100, 101 };

z = l[1];

// nested list

ll = { { 0, 1 }, { 2, 3 } };

x = ll[1][1];

*/

// direct read access to struct via list

struct S { float f };
struct S myS;
myS.f = 999;

slist = { myS };

x = slist[0].f;	

if (x != 999) {
	exit("x should equal 999");
}

// direct write access to struct via list

slist[0].f = 777;

xx = slist[0].f;	

if (xx != 777) {
	exit("x should equal 777");
}


// direct read access to struct via function call

/* NOT YET SUPPORTED

struct S returnAnS(float value)
{
	struct S _s;
	_s.f = value;
	return _s;
}


xxx = returnAnS(555).f;

if (xxx != 555) {
	exit("x should equal 555");
}

*/