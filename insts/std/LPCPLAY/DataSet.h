#include <RefCounted.h>

// DataSet.h

class DataSet : public RefCounted
{
public:
	DataSet();
	off_t	open(const char *fileName, int npoleGuess, float sampRate);
	inline int getNPoles() { return _nPoles; }
	inline off_t getFrameCount() { return _frameCount; }
	int	getFrame(float frameno, float *pCoeffs);
protected:
	~DataSet();
	void	allocArray(int nPoles);
private:
	int	_nPoles;
	off_t _frameCount;
	int	_fdesc;
	int	_lpHeaderSize;
	float	*_array;
	off_t	_oldframe, _endframe;
	int	_framsize;
	int	_fprec;
	int	_recsize;
	int	_bprec;
	int	_bpframe;
	int	_npolem1;
};

