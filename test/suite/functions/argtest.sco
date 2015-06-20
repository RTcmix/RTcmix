float testfunction(float f, string s, handle h)
{
	print("in testfunction:");
	print(f);
	print(s);
	print(h);
	return 0;
}

string str;
float flt;
handle hnd;

str = "hello";
flt = 77;
hnd = { "hi", "there", 99 };

testfunction(flt, str, hnd);

testfunction(0, "goodbye", { 7, 6, 5, 4 });

