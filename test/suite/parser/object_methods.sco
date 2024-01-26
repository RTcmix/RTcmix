struct Test {
	list object,
	float testLen,
	string testType
};

handle h;

map m;
m[0] = 0;
m[1] = 1;

struct TestStruct { float f, string s };
testStruct = TestStruct(11, "hello");

objectTests = {
	Test({7.0}, 1, "float"),
	Test({"a string"}, 8, "string"),
	Test({h}, 1, "handle"),
	Test({{1, 2, 3}}, 3, "list"),
	Test({m}, 2, "map"),
	Test({testStruct}, 0, "struct")		// struct does not suport len() so returns 0
};

printf("List of tests: %l\n", objectTests);

objectList = {};	// this keeps objects from overwriting others (for now)

for (i = 0; i < len(objectTests); ++i) {
	test = objectTests[i];
	printf("Testing %s methods\n", test.testType);
	obj = test.object[0];
	if (obj.len() != test.testLen) { error("len does not match"); }
	if (obj.type() != test.testType) { error("type does not match"); }
}
