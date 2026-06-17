// ============================================================================
// switch fall-through scope / FunctionBalance verification (PASS)
//   A grouped-case shared body gets local scope (like while/for) and must
//   keep the for/while block depth balanced across iterations.
// Exit 0 = pass; any failed check calls exit("...") -> non-zero.
// ============================================================================

float outer;
outer = 100;

sum = 0;
for (i = 0; i < 4; i = i + 1) {
   switch (i) {
   case 0:
   case 1: { float loc; loc = i * 10; sum = sum + loc; }   // local 'loc' re-declared each pass
   case 2: { sum = sum + 1; }
   default: { sum = sum + 1000; }                            // i==3
   }
}
// i=0 -> 0, i=1 -> 10, i=2 -> 1, i=3 -> 1000  => 1011
if (sum != 1011) { printf("sum=%d expected 1011\n", sum); exit("grouped switch-in-for sum failed"); }

// Depth balance: 'outer' (global) must be intact after the loop.
if (outer != 100) { printf("outer=%f expected 100\n", outer); exit("scope depth unbalanced after grouped switch"); }

// if/else uses global scope; a leak of the case body's depth would misbehave here.
afterVar = -1;
if (sum == 1011) { afterVar = 7; }
if (afterVar != 7) { exit("if-block after grouped switch misbehaved (scope depth unbalanced)"); }

printf("TEST SUCCEEDED\n");