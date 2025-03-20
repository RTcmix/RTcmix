set_option("suppressed_fun_names=foo,MyClassFunction");

struct MyClass {
	method float MyClassFunction() {
		printf("Called MyClassFunction - banner should have been suppressed\n"); return 0;
    }
};

struct MyClass mc;

mc.MyClassFunction();

