float tablei(long nsample, double *array, float *tab)
{
	register int loc1,loc2;
	float frac = ((float)(nsample)/(*tab)) * *(tab+1);
	if(frac < 0) return(array[0]);
	if(frac >= *(tab+1)) return(array[(int)*(tab+1)]);
	loc1 = frac;
	loc2 = loc1+1;
	frac = frac - (float)loc1;
	return(*(array+loc1) + frac * (*(array+loc2) - *(array+loc1)));
}
