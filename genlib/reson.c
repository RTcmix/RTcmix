float reson(float x, float *a)
{
	float temp;
	temp = *a * x + *(a+1) * *(a+3) - *(a+2) * *(a+4);
	*(a+4) = *(a+3);
	*(a+3) = temp;
	return(temp);
}
