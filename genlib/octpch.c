float octpch(float pch)
{
	int oct = pch;
	return(oct + 8.333333 * (pch - oct));
}
