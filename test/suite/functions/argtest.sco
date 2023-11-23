float testfunction(float f, string s, list l)
{
	printf("in testfunction: %d args\n", _n_args);
	print(f);
	print(s);
	print(l);
	return 0;
}

string str;
float flt;
list lst;

str = "hello";
flt = 77;
lst = { "hi", "there", 99 };

testfunction(flt, str, lst);

testfunction(0, "goodbye", { 7, 6, 5, 4 });

