// only one 'default' allowed -> syntax error
switch (1) {
   case 1:  { x = 1; }
   default: { x = 0; }
   default: { x = 2; }
}
