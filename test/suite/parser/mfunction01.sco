float function(string sArg) { return 0; }

fptr = function;

if (fptr.type() != "function") {
	error("type() method failed on mfunction obj");
}

struct FunTest {
	mfunction myFunction
};

myFunTest = FunTest(fptr);

printf("Testing a.b.c()\n");

if (myFunTest.myFunction.type() != "function") {
	error("type() method failed on mfunction-type struct member");
}

myFunList = { fptr };

printf("Testing a[b].c()\n");

if (myFunList[0].type() != "function") {
	error("type() method failed on mfunction-type list element");
}

printf("Success\n");
