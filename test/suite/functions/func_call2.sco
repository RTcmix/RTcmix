float myfunction(string arg1, handle h) { 
	print(arg1);
	print(h);
	return 0;
}

handle mh;
mh = { 3, 4, 5 };
myfunction("hello", mh);

