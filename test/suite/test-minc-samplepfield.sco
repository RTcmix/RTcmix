table = maketable("line", 1000, 0,0, 1,1);

x = samptable(table, 500);
printf("halfway through 0-to-1 line: %f\n", x);

pfield = table * 10;	// convert table into pfield

y = samptable(pfield, 500);
printf("halfway through 0-to-10 pfield: %f\n", y);
