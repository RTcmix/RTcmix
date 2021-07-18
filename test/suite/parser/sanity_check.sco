/* sanity check for basic operations */

float verify(float state) {
	if (state == false) {
		exit("verify failed!");
	}
	return 0;
}

// auto decls

f = 20;
s = "string";
l = {};
l2 = { 1, 2 };

// basic operations

x = f + 1;
verify((x == 21));

y = ++x;
verify((y == 22));

z = x + y;
verify((z == 44));

q = y - x;
verify((q == 0));

ss = s + "s";
verify((ss == "strings"));

// operations on lists

l[0] = z;
l[1] = s;
l[2] = l2;

ll = l2 + 1;
ll2 = l2 * 2;


// function decls

float fun() { return 0; }
float fun2(float a) { return a; }
string fun3() { return "foobar"; }
string fun4(string s) { return "foobar" + s; }
list fun5() { return { 0, 1, 2 }; }

// function calls

g = fun();
h = fun2(4);
si = fun3();
ssi = fun4("hi");
lhl = fun5();

// struct decls

struct S { float f, string s, list l, handle h, map m };

// operations on structs

struct S the_s;

the_s.f = x;
the_s.s = ss;
the_s.l = ll2;




