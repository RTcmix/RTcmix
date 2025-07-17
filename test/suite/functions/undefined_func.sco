set_option("bail_on_undefined_function=false");

struct TestMethod
{
	method float callUndefinedFunction() {
		x = undefinedFunction();
		return 0;
	}
}

struct TestMethod tm;

tm.callUndefinedFunction();

