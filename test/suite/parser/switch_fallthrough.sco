// ============================================================================
// switch fall-through / label grouping (PASS)
//   case x: case y: { ... }  -> the shared block runs if x OR y matches
// Exit 0 = pass; any failed check calls exit("...") -> non-zero.
// ============================================================================

float test(float v)
{
   float result;
   result = -1;
   switch (v) {
   case 1:
   case 2: { result = 12; }       // 1 or 2 share this block
   case 3: { result = 3; }        // its own block
   default: { result = 99; }
   }
   return result;
}

// 1. First label of a group matches
if (test(1) != 12) { exit("group first label (v=1) failed"); }
// 2. Second label of a group matches
if (test(2) != 12) { exit("group second label (v=2) failed"); }
// 3. A standalone label keeps its own block (not the group's)
if (test(3) != 3) { exit("standalone label (v=3) failed"); }
// 4. No match falls to default
if (test(7) != 99) { exit("no-match -> default (v=7) failed"); }

// 5. Three-label group plus a trailing standalone label, no default
float test2(float v)
{
   float r;
   r = -1;
   switch (v) {
   case 10:
   case 20:
   case 30: { r = 1; }
   case 40: { r = 2; }
   }
   return r;
}
if (test2(10) != 1) { exit("3-label group (v=10) failed"); }
if (test2(20) != 1) { exit("3-label group (v=20) failed"); }
if (test2(30) != 1) { exit("3-label group (v=30) failed"); }
if (test2(40) != 2) { exit("trailing standalone label (v=40) failed"); }
if (test2(99) != -1) { exit("no match, no default (v=99) should leave -1"); }

// 6. String labels grouped
float test3(string s)
{
   float r;
   r = 0;
   switch (s) {
   case "a":
   case "b": { r = 1; }
   default: { r = 2; }
   }
   return r;
}
if (test3("a") != 1) { exit("string group (a) failed"); }
if (test3("b") != 1) { exit("string group (b) failed"); }
if (test3("z") != 2) { exit("string group default (z) failed"); }

// 7. A group must run its shared body exactly once (no double execution)
count = 0;
switch (2) {
   case 1:
   case 2: { count = count + 1; }
   case 3: { count = count + 1; }       // must NOT run
}
if (count != 1) { printf("7 once: count=%d expected 1\n", count); exit("grouped body ran wrong number of times"); }

printf("TEST SUCCEEDED\n");