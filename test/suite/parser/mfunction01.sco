float function(string sArg) { return 0; }

fptr = function;

ret = fptr("hello");

if (fptr.type() != "function") {
	error("type() method failed on mfunction obj");
}

struct FunTest {
	mfunction myFunction
};

myFunTest = FunTest(fptr);

printf("Testing a.b() for mfunction\n");

if (myFunTest.myFunction.type() != "function") {
	error("type() method failed on mfunction-type struct member");
}

ret = myFunTest.myFunction("hello");

myFunList = { fptr };

printf("Testing a[b]() for mfunction\n");

if (myFunList[0].type() != "function") {
	error("type() method failed on mfunction-type list element");
}

ret = myFunList[0]("hello");

myStructList = { myFunTest };

printf("Testing a[b].c() for mfunction\n");

ret = myStructList[0].myFunction("hello");

printf("Success\n");
