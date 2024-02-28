// Test new list comparisons

l1 = { 1, 2, 3 };
l2 = { 1, 2, 3 };
cl1 = l1;
l3 = { 1, 2 };
rl3 = { 2, 1 };
l4 = { 3 };
l5 = { 1, 2, 4 };
l6 = {}
list l7;	// null list

float test(float got, float expected) { if (got != expected) { error("FAILED. compare returned %f, expected %f", got, expected); } return 0; }

b = (l1 == l1)
test(b, true);

b = (l1 == l2)
test(b, true);

b = (l1 == cl1);
test(b, true);

b = (l6 == l6);
test(b, true);

b = (l7 == l7);
test(b, true);

// Greater than

b = (l1 > l3);
test(b, true);

b = (l1 > l4);
test(b, true);

b = (l1 > l6);
test(b, true);

b = (l1 > l7);
test(b, true);

b = (rl3 > l3);
test(b, true);

// Less than

b = (l1 < l5);
test(b, true);

b = (rl3 < l3);
test(b, false);

b = (l4 < l3);
test(b, true);

b = (l6 < l4);
test(b, true);

b = (l7 < l6);
test(b, true);

printf("SUCCEEDED\n");
