// testing function object

// first declare a function

string concat(string s1, string s2)
{
	return s1 + s2;
}

// variable holding that function

fun_object = concat;

// call it correctly

s = fun_object("hello ", "world");

print(s);
