float fun()
{
	localFunVar = 9;
	return 0;
}

fun();

print(localFunVar);		// should fail because auto-decls in functions are local
