/*										LPCHEADER.H	*/

typedef	struct {
	int	headersize, lpmagic, npoles, nvals;
	float	framrate, srate, duration;
	char	text[4];
} LPHEADER;
