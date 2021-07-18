// Testing map with multiple types for its keys

map m;

m["foo"] = 7;
m["foobar"] = 17;
m[2938472] = 11;
m[2938473] = 13;

x = m["foo"];
if (x != 7) {
	printf("map failed to store value with key 'foo'\n");
}
x = m["foobar"];
if (x != 17) {
	printf("map failed to store unique value with key 'foobar'\n");
}

y = m[2938472];
if (y != 11) {
	printf("map failed to store value with key 2938472\n");
}
y = m[2938473];
if (y != 13) {
	printf("map failed to store unique value with key 2938473\n");
}
