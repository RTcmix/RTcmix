/* put value in delay line. See delset. x is float */
void
delput(float x, float *a, int *l)
{
	int index = l[0];
	a[index] = x;
	l[0]++;
	if (l[0] >= l[2])
		l[0] -= l[2];
}
