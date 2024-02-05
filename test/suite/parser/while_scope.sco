// While scopes are local

count = 1;

while (count > 0) {
	inWhileScopeVar = 9;
	--count;
}

print(inWhileScopeVar);		// this should fail
