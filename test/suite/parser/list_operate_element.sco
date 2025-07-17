testList = { 1, 2, 3, 4 };

x = (testList[1] += 10);
y = (testList[2] -= 10);
z = (testList[0] *= 10);
a = (testList[3] /= 2);

if (x != 12) { print("SCORE FAILED"); }
else if (y != -7) { print("SCORE FAILED"); }
else if (z != 10)  { print("SCORE FAILED"); }
else if (a != 2) { print("SCORE FAILED"); }
else print("SCORE SUCCEEDED");

