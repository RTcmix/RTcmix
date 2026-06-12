// 'default' must be the last clause -> syntax error
switch (1) {
   default: { x = 0; }
   case 1:  { x = 1; }
}
