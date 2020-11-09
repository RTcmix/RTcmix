// testing function object in struct

// first declare a function

string concat(string s1, string s2)
{
	return s1 + s2;
}

// struct holding that function

struct HoldFun { mfunction mFunc };

// declare struct

struct HoldFun funHolder;

// assign

funHolder.mFunc = concat;

// call it via struct

s = funHolder.mFunc("hello ", "world");

print(s);
