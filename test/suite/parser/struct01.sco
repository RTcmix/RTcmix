struct Foo {
	float f_element,
	string s_element
};

struct Foo myFoo;

myFoo.f_element = 11.0;

x = myFoo.f_element + 9;

if (x != 20) {
	printf("FAILED\n");
}
else {
	printf("SUCCEEDED\n");
}


