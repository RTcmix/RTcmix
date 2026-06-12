// ============================================================================
// switch statement - functional + scope/FunctionBalance verification (PASS)
// Exit 0 = pass; any failed check calls exit("...") -> non-zero.
// ============================================================================

// 1. Basic match selects the right case
result = -1;
x = 2;
switch (x) {
   case 1: { result = 1; }
   case 2: { result = 2; }
   case 3: { result = 3; }
}
if (result != 2) { printf("1 basic: result=%d expected 2\n", result); exit("switch basic match failed"); }

// 2. No fall-through: exactly one case body runs
count = 0;
switch (1) {
   case 1: { count = count + 1; }
   case 2: { count = count + 1; }       // must NOT run
}
if (count != 1) { printf("2 fallthrough: count=%d expected 1\n", count); exit("switch fall-through failed"); }

// 3. default runs when nothing matches
result = -1;
switch (99) {
   case 1: { result = 1; }
   default: { result = 0; }
}
if (result != 0) { printf("3 default: result=%d expected 0\n", result); exit("switch default failed"); }

// 4. default does NOT run when a case matches (default is last but skipped)
result = -1;
switch (1) {
   case 1: { result = 1; }
   default: { result = 0; }
}
if (result != 1) { printf("4 default-skip: result=%d expected 1\n", result); exit("switch default-skip failed"); }

// 5. No match and no default: nothing runs
result = -1;
switch (42) {
   case 1: { result = 1; }
}
if (result != -1) { printf("5 nomatch: result=%d expected -1\n", result); exit("switch no-match failed"); }

// 6. Runtime (non-constant) case expression
a = 5; b = 3;
result = -1;
switch (8) {
   case a + b: { result = 100; }        // 5+3==8 -> match
   case a - b: { result = 200; }
}
if (result != 100) { printf("6 runtime-expr: result=%d expected 100\n", result); exit("switch runtime case expr failed"); }

// 7. String switch (== on strings)
result = -1;
s = "hello";
switch (s) {
   case "world": { result = 1; }
   case "hello": { result = 2; }
   default: { result = 0; }
}
if (result != 2) { printf("7 string: result=%d expected 2\n", result); exit("switch string match failed"); }

// 8. Type mismatch is a non-match, not an error (float switch vs string case)
result = -1;
switch (7) {
   case "seven": { result = 1; }        // different type -> non-match, must not crash
   case 7: { result = 2; }
   default: { result = 0; }
}
if (result != 2) { printf("8 typemismatch: result=%d expected 2\n", result); exit("switch type-mismatch failed"); }

// 9. FunctionBalance / scope depth: switch nested in a for loop, repeated
sum = 0;
for (i = 0; i < 3; ++i) {
   switch (i) {
      case 0: { sum = sum + 10; }
      case 1: { sum = sum + 20; }
      default: { sum = sum + 30; }      // i==2
   }
}
if (sum != 60) { printf("9 switch-in-for: sum=%d expected 60\n", sum); exit("switch inside for failed"); }

// 10. Depth balance check: after the switches, an if-block must still scope as global
//     (switch uses the for/while depth; if it leaked into if/else depth this would misbehave)
afterVar = -1;
if (sum == 60) { afterVar = 7; }        // if/else uses global scope -> visible here
if (afterVar != 7) { exit("if-block after switch misbehaved (scope depth unbalanced)"); }

printf("TEST SUCCEEDED\n");
