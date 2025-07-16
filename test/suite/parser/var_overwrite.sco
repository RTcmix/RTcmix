float fun() { return 0; }
struct S { float f };

floatval = 1;
stringval = "a string";
handleval = maketable("window", 1000, "hanning");
listval = {}
map mapval;
struct S structval = { 11 };
functionval = fun;

floatval = "overwritten string";	// legal
stringval = 777;					// legal for compatibility
handleval = 0;				// now illegal
listval = 888;				// now illegal
mapval = 999;				// now illegal
structval = 101010;			// now illegal
functionval = 222;			// now illegal

