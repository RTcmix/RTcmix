// --- Testing compound comparisons
x = 5;
y = 3;
a = 1;
b = 2;
if (!(x > y && a < b)) { error("Compound comparison with && failed"); }
if (!(x > y || a > b)) { error("Compound comparison with || failed"); }
if (!(x > y && (a < b || x == 5))) { error("Nested compound comparison failed"); }

// --- Testing map declarations and access
map m;
m["foo"] = 42;
if (m["foo"] != 42) { error("map assignment/verification failed"); }

m["bar"] = 100;
m["baz"] = m["bar"] + 23;
if (m["baz"] != 123) { error("map assignment with expression failed"); }

// --- Testing lists of lists
mylist = { {1, 2}, {3, 4}, {5, 6} };
mylist[1][1] = 10;
if (mylist[1][1] != 10) { error("list of lists modification failed"); }

// --- Testing lists of other non-float types (strings, booleans)
strlist = { "a", "b", "c" };
if (strlist[2] != "c") { error("string list indexing failed"); }

boollist = { true, false, true };
if (boollist[1] != false) { error("boolean list indexing failed"); }

// --- Ternary expression testing
x = 10;
y = 20;
min = (x < y ? x : y);
if (min != 10) { printf("min (%d) != 10\n", min); error("ternary with < failed"); }

msg = (x > y ? "larger" : "smaller");
if (msg != "smaller") { error("ternary with string result failed"); }

result = (x == 10 && y == 20 ? true : false);
if (result != true) { error("ternary with compound boolean condition failed"); }

ternlist = (x > 0 ? {1, 2, 3} : {4, 5, 6});
if (ternlist[0] != 1) { error("ternary returning a list failed"); }
