float hpluck(float xin, float *q)
{
	float temp;
	int ip;
	q[0]++;
	if(*q <= q[1]) ip = *q;
	else *q = ip = 20;
	q[7] = (1.-q[8]) * q[ip] + q[8] * q[7];
	temp = q[9] * (q[2] * q[ip] + q[3] * q[6]);
	q[6] = q[ip];
	q[ip] = q[10] * temp + q[4] - q[10] * q[5];
	q[5] = q[ip] = xin + q[ip];
	q[4] = temp;
	return(q[7]);
}
