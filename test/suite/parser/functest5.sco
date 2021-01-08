// testing passing function objects as arguments


string sfunc() { return "returned_string"; }

list outerfunc(mfunction funArg)
{
	list l;
	l[0] = funArg();
	return l;
}

xlist = outerfunc(sfunc);

print(xlist);
