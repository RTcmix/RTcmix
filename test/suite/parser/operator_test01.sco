// a list containing one of each type

printf("THIS TEST SHOULD GENERATE WARNINGS BUT NO FAILURES OR PARSER ERRORS\n");

map m;
m[0] = 100;

struct Test { float f };

float function() { return 100; }

objectList = {
	100,								// float
	"100",								// string
	{ 100 },							// list
	maketable("wave3", 1000, 1, 1, 0),	// handle
	m,									// map
	Test(100),							// struct
	function							// mfunction
};

printf("Operators +, -, *, and /\n");

for (i = 0; i < objectList.len(); ++i)
{
	obj2  = objectList[i];
	printf("    type <op> %s, %s <op> type\n", obj2.type(), obj2.type());
	for (j = 0; j < objectList.len(); ++j)
	{
		obj = objectList[j];
	
		x1 = obj + obj2;
		x2 = obj2 + obj;
		x3 = obj - obj2;
		x4 = obj2 - obj;
		x5 = obj * obj2;
		x6 = obj2 * obj;
		x7 = obj / obj2;
		x8 = obj2 / obj;
	}

	printf("Operators +=, -=, *=, /=, ++, and --\n");

	for (n = 0; n < objectList.len(); ++n)
	{
		obj = objectList[n];
	
		x1 = obj += obj2;
		x3 = obj -= obj2;
		x5 = obj *= obj2;
		x7 = obj /= obj2;
	}
}

printf("Operators ++, and --\n");
for (n = 0; n < objectList.len(); ++n)
{
	obj = objectList[n];

	++obj;
	--obj;
}

printf("SUCCESS\n");

	
