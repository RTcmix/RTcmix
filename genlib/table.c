float table(long nsample, float *array, float *tab)
{
	int loc = (int)((float)(nsample)/(*tab)) * *(tab+1);
	if(loc < 0) return(array[0]);
	if(loc >= *(tab+1)) loc = *(tab+1);
	return(*(array + loc));
}

