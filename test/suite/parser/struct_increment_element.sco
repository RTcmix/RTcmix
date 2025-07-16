struct Test { float f }

struct Test t = { 100 };

y = ++t.f;

printf("t.f is now %f\n", t.f);

if (y != 101) { print("SCORE FAILED"); }
else { print("SCORE PASSED"); }

