// testing parsing of new ternary operator

a = 100;
b = 200;
c = 1000;


// In MinC, ternary operators work like compound expressions: 2 + 4 * 8 == (2 + 4) * 8 using LH grouping.
// So, in ternary, b ? x : y + z == (b ? x : y) + z.

// Here we check the normal case where the result would be identical either way

result = (false) ? a : b + c;

if (result != b + c) { printf("result %d != b + c (%d)\n", result, b + c); exit("ternary parser incorrect"); }

// Minc requires that both expressions resolve to unary.  Make sure that works.

result = (true) ? (a + b) : c;

if (result != a + b) { printf("result %d != a + b (%d)\n", result, a + b); exit("ternary parser incorrect"); }

printf("TEST SUCCEEDED\n");

