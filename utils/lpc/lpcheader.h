/*										LP.H	*/

#define	LP_MAGIC    999
#define	MAXPOLES    34
#define	MAXFRAME    (MAXPOLES + 4)
#define	LPBUFSIZ    4096
#define	LPBUFVALS   1024
#define	BUFNOSHIFT  10
#define	BUFPOSMASK  1023

typedef	struct {
	int	headersize, lpmagic, npoles, nvals;
	float	framrate, srate, duration;
	char	text[4];
} LPHEADER;
