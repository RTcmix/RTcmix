float pluck(float sig, float *q)
{
	float temp;
	int ip,len;
	q[0]++;
	if(q[0] < q[1]) {
		ip=q[0];
		temp=q[ip];
		q[ip]=sig+q[2]*q[ip]+q[3]*q[ip-1];
		return(temp);
	}
	ip=4;
	q[0]=4.;
	temp=q[ip];
	len=q[1]-1.;
	q[ip]=sig+q[2]*q[ip]+q[3]*q[len];
	return(temp);
}
