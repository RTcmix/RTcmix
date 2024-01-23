struct TestMethod
{
	method float callUndefinedFunction() {
		x = undefinedFunction();
		return 0;
	}
}

struct TestMethod tm;

tm.callUndefinedFunction();

