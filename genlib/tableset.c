
void
tableset(float SR, float dur, int size, float *tab)
{
	tab[0] = (long)(dur * SR  -.9999);
	tab[1] = size - 1;
}
