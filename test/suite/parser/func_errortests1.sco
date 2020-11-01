// testing various types of errors within functions

float errorfunc()
{ 
	return rtsetparams(-3, 4);
}

float outer(mfunction fun)
{
	fun();
	return 1;
}

outer(errorfunc);

