map m;

// Key is string

m["foo"] = 7;

x = m["foo"];

if (x != 7) {
	printf("FAILED: map failed to store value with key 'foo'\n");
}
else {
	printf("SUCCEEDED\n");
}
