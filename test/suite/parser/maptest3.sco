// Attempting to store all datatypes using simple numeric key

l = { 777 };		// list
s = "store me";		// string
handle h;			// handle type

map m;
m[1] = l;

ll = m[1];
if (ll[0] != 777) {
	printf("map failed to store list\n");
}

m[2] = s;

if (m[2] != s) {
	printf("map failed to store string\n");
}

m[3] = h;

if (type(m[3]) != "handle") {
	printf("map failed to store handle\n");
}
