max_depth = 3;

list fun1(float depth)
{
	list funLocalList;
	if (depth < max_depth) {
		funLocalList = fun2(depth+1);
		return funLocalList;
	}
	return { depth };
}

list fun2(float depth)
{
	list funLocalList;
	if (depth < max_depth) {
		return fun1(depth+1);
	}
	return { depth };
}

mylist = fun1(0);

