list fun3()
{
	printf("<<<fun3>>>\n");
	printf("returning empty list to caller\n");
	printf("    <<<exiting fun3>>>\n");
	return {};
}

list fun2()
{
	printf("<<<fun2>>>\n");
	list _out2;
	printf("writing results of recursive call into _out2[0]\n");
	_out2[0] = fun3();
	printf("    <<<exiting fun2>>\n");
	return _out2;
}

list fun1()
{
	printf("<<<fun1>>>\n");
	list _out;
	printf("writing results of recursive call into _out[0]\n");
	_out[0] = fun2();
	printf("    <<<exiting fun1>>\n");
	return _out;
}

for (n = 0; n < 10000; ++n) {
	l = fun1();
}

printf("SUCCEEDED\n");
