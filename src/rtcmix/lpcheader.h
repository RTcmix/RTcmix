/*										LPCHEADER.H	*/

typedef	struct {
	int	headersize, lpmagic, npoles, nvals;
	float	framrate, srate, duration;
	char	text[4];
} LPHEADER;

#ifdef __cplusplus
extern "C" {
#endif
	int checkForHeader(int afd, int *nPoles, float sr, Bool *pSwapped);
#ifdef __cplusplus
}
#endif
