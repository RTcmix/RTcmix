// Verify that expressions on RHS return their value

float verifyFloat(float val, float expected)
{
	if (val != expected) {
		printf("Previous operation failed: expected %f but got %f\n", expected, val);
		exit();
	}
	return 0;
}

l = { 0, 1, 2 };

verifyFloat(l[0] += 5, 5);
verifyFloat(l[1] -= 2, -1);
verifyFloat(l[2] *= 10, 20);
verifyFloat(l[2] /= 10, 2);

struct Foo { float f };

foo = Foo(100);

verifyFloat(foo.f += 5, 105);
verifyFloat(foo.f -= 20, 85);
verifyFloat(foo.f *= 10, 850);
verifyFloat(foo.f /= 10, 85);

map m;

m[0] = 100;

verifyFloat(m[0] += 5, 105);
verifyFloat(m[0] -= 20, 85);
verifyFloat(m[0] *= 10, 850);
verifyFloat(m[0] /= 10, 85);

printf("SUCCEEDED\n");
