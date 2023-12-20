testList = { 1, 2, 3, 4 };

y = ++testList[1];
z = --testList[3];

printf("y = %d, z = %d\n", y, z);

printf("testList[1] now %f\n", testList[1]);
printf("testList[3] now %f\n", testList[3]);

if (y != 3) { print("SCORE FAILED"); }
else if (y != testList[1])  { print("SCORE FAILED"); }
else if (y != z) { print("SCORE FAILED"); }
else if (z != testList[3]) { print("SCORE FAILED"); }
else print("SCORE SUCCEEDED");

