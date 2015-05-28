float floatreturn() { return 1024; }

float x;

x = floatreturn();

print(x);

if (x != 1024) {
	exit("floatreturn() did not return 1024.0", -1);
}

string stringreturn() { return "hello, world!"; }

string s;

s = stringreturn();

if (s != "hello, world!") {
	exit("stringreturn did not return correct string", -1);
}
