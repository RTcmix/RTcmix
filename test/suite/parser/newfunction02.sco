// testing function object in list

// first declare a function

string concat(string s1, string s2)
{
	return s1 + s2;
}

// list holding that function

fun_list = { concat };

// call it via list

s = fun_list[0]("hello ", "world");

print(s);
