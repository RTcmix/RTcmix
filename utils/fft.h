#define FLOAT_PRECISION float
#define D16BITINT short
#define LENFLOAT (sizeof(FLOAT_PRECISION))
#define LENCOMPLEX 2*LENFLOAT
#define LEN16BITINT (sizeof(D16BITINT))
#define SRDEFAULT 1000.0
      
#define TRUE 1
#define FALSE 0

/* types of input or output
	a real ascii
	A complex ascii
	f real float
	F complex float
	w real word (short int - 16 bits on vax)
	L real longword (long int - 32 bits on vax)
*/
#define DEFAULT_INTYPE "A"
#define DEFAULT_OUTTYPE "A"
#define DEFAULT_SR 1000.0
#define DEFAULT_FFTSIZE 32
#define DEFAULT_NFFTS 1
#define DEFAULT_MAXFFTS 32767
