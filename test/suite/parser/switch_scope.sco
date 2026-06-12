// Switch case bodies use local scope (like while/for): a var declared in a case
// body must NOT be visible afterward.  print() of it should fail.
switch (1) {
   case 1: { inCaseVar = 9; }
}
print(inCaseVar);     // this should FAIL (inCaseVar is local to the case body)
