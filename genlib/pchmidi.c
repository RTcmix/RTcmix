float pchmidi(unsigned char midinote)
{
	float pchoct();
	return (pchoct(((float)midinote/12.) + 3.));
}
