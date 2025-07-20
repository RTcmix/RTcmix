// --- Testing Assignment
x = 10;
if (x != 10) { error("x = 10 failed"); }

// --- Testing PLUSEQ
x += 5;
if (x != 15) { error("x += 5 failed"); }

// --- Testing MINUSEQ
x -= 3;
if (x != 12) { error("x -= 3 failed"); }

// --- Testing MULEQ
x *= 2;
if (x != 24) { error("x *= 2 failed"); }

// --- Testing DIVEQ
x /= 4;
if (x != 6) { error("x /= 4 failed"); }

// --- Testing Pre-increment
++x;
if (x != 7) { error("++x failed"); }

// --- Testing Pre-decrement
--x;
if (x != 6) { error("--x failed"); }

// --- Testing Subscript Assignment
arr = { 1, 2, 3 }; arr[1] = 42;
if (arr[1] != 42) { error("arr[1] = 42 failed"); }

// --- Testing Subscript Compound Assignment
arr[0] += 7;
if (arr[0] != 8) { error("arr[0] += 7 failed"); }

// --- Testing Subscript Pre-Increment
++arr[2];
if (arr[2] != 4) { error("++arr[2] failed"); }

// --- Testing Subscript Pre-Decrement
--arr[2];
if (arr[2] != 3) { error("--arr[2] failed"); }

// --- Testing Function Call
powval = pow(2, 3);
if (powval != 8) { error("pow(2, 3) failed"); }

// --- Testing Boolean AND
result = 1 && 0;
if (result != 0) { error("1 && 0 failed"); }

// --- Testing Boolean OR
result = 0 || 1;
if (result != 1) { error("0 || 1 failed"); }

// --- Testing Equality
x = 10; result = (x == 10);
if (result != 1) { error("x == 10 failed"); }

// --- Testing Inequality
result = (x != 5);
if (result != 1) { error("x != 5 failed"); }

// --- Testing Less Than
result = (x < 20);
if (result != 1) { error("x < 20 failed"); }

// --- Testing Greater Than
result = (x > 5);
if (result != 1) { error("x > 5 failed"); }

// --- Testing Less Equal
result = (x <= 15);
if (result != 1) { error("x <= 15 failed"); }

// --- Testing Greater Equal
result = (x >= 6);
if (result != 1) { error("x >= 6 failed"); }

// --- Testing Logical NOT
result = !0;
if (result != 1) { error("!0 failed"); }

// --- Testing Modulo
modval = 10 % 3;
if (modval != 1) { error("10 % 3 failed"); }

// --- Testing Addition
if (3 + 4 != 7) { error("3 + 4 failed"); }

// --- Testing Subtraction
if (10 - 2 != 8) { error("10 - 2 failed"); }

// --- Testing Multiplication
if (4 * 2 != 8) { error("4 * 2 failed"); }

// --- Testing Division
if (8 / 2 != 4) { error("8 / 2 failed"); }

// --- Testing Power
if (2 ^ 3 != 8) { error("2 ^ 3 failed"); }

// --- Testing Unary Minus
if (-5 != 0 - 5) { error("-5 failed"); }

// --- Testing TRUE Literal
if (TRUE != 1) { error("TRUE failed"); }

// --- Testing FALSE Literal
if (FALSE != 0) { error("FALSE failed"); }

// --- Testing Ternary
result = (x == 10) ? 1 : 0;
if (result != 1) { error("ternary expression failed"); }

// --- Testing Expression List
mylist = { 1, 2, 3 };
if (mylist[1] != 2) { error("mylist[1] failed"); }

// --- Testing Empty List
empty = { };
if (len(empty) != 0) { error("empty list failed"); }

// --- Testing Struct Field Assignment
struct Point { float x, float y };
struct Point pt;
pt.x = 3;
pt.y = 4;
if (pt.x != 3) { error("pt.x assignment failed"); } if (pt.y != 4) { error("pt.y assignment failed"); }

// --- Testing Struct Assignment
struct Point pt2; pt2 = pt;
if (pt2.x != 3) { error("pt2.x copy failed"); } if (pt2.y != 4) { error("pt2.y copy failed"); }

// --- Testing Nested Struct Field Access
struct Nested { struct Point inner };
struct Nested nest;
nest.inner = Point();
nest.inner.x = 5;
nest.inner.y = 6;
if (nest.inner.x != 5) { error("nest.inner.x failed"); } if (nest.inner.y != 6) { error("nest.inner.y failed"); }

// --- Testing Parentheses Grouping
if (((3 + 2) * 4) != 20) { error("parentheses grouping failed"); }

// --- Testing If/Else Logic
x = 10; if (x > 5 && x < 15) { y = 1; } else { y = 2; }
if (y != 1) { error("if/else block failed"); }

// --- Testing If/Else True Branch
x = 5; if (x < 10) { y = 1; } else { y = 2; }
if (y != 1) { error("if/else true branch failed"); }

// --- Testing If/Else False Branch
x = 15; if (x < 10) { y = 1; } else { y = 2; }
if (y != 2) { error("if/else false branch failed"); }

// --- Testing While Loop
i = 0; sum = 0; while (i < 5) { sum += i; ++i; }
if (sum != 10) { error("while loop failed"); }

// --- Testing For Loop
sum = 0; for (i = 0; i < 5; ++i) { sum += i; }
if (sum != 10) { error("for loop failed"); }

// --- Testing Function Return
float f() { return 42; }; result = f();
if (result != 42) { error("function return failed"); }

// --- Testing Function With Args
float add(float a, float b) { return a + b; } result = add(3, 4);
if (result != 7) { error("add(3, 4) failed"); }
