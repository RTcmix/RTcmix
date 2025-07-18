// testing parsing of new ternary operator

a = 100;
b = 200;
c = 1000;


// In MinC, ternary operators work like compound expressions: 2 + 4 * 8 == (2 + 4) * 8 using LH grouping.
// So, in ternary, b ? x : y + z == (b ? x : y) + z.

// Here we check the normal case where the result would be identical either way

result = (false) ? a : b + c;

if (result != b + c) { printf("result %d != b + c (%d)\n", result, b + c); exit("ternary parser incorrect"); }

// Here we check to be sure the LH binding happens -- the result should include the addition of c even though the boolean is true.

result = (true) ? a : b + c;

if (result != a + c) { printf("result %d != a + c (%d)\n", result, a + c); exit("ternary parser not reducing left"); }

// Minc requires that both expressions be unary.  Make sure that works.

result = (true) ? (a + b) : c;

if (result != a + b) { printf("result %d != a + b (%d)\n", result, a + b); exit("ternary parser incorrect"); }

printf("TEST SUCCEEDED\n");

