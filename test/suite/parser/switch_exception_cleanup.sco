// An exception thrown DURING execution of a case body must unwind cleanly through the
// case-body scope, the switch's for/while-depth balance, and the enclosing block/for
// scopes -- a clean parse error (exit 1), never a crash/assert/segfault.
for (i = 0; i < 1; ++i) {
   localInFor = 5;
   {
      switch (1) {
         case 1: {
            inCaseLocal = 7;
            boom = undeclaredVariableXYZ;   // undeclared read -> throws while exct'ing this case body
         }
         default: { skip = 0; }
      }
   }
}
print("should not reach here");
