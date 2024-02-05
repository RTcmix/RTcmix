// For scopes are local

for (count = 0; count < 1; ++count) {
	inForScopeVar = 9;
}

print(inForScopeVar);		// this should fail
