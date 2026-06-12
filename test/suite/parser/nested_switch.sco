// Nested switch statements are not allowed -> should fail to parse.
switch (1) {
   case 1: {
      switch (2) {
         case 2: { x = 1; }
      }
   }
}
