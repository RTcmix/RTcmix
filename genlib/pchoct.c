float pchoct(float oct)
{
	float x, result;
	x = (int)oct;
	result = (.12 * (oct - x) + x); 
	return result;
}
