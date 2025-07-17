struct Test {
	float f,
	method float fun() { ++this.f; return this.f; }
};

t = Test(100);
ret = t.fun();

printf("t.f is now %f\n", t.f);

if (ret != 101 || t.f != 101) { print("SCORE FAILED"); }
else { print("SCORE PASSED"); }

