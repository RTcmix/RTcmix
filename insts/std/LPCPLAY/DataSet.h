#include <RefCounted.h>

// DataSet.h

class DataSet : public RefCounted
{
public:
	DataSet();
	int	open(const char *fileName, int npoleGuess, float sampRate);
	inline int getNPoles() { return _nPoles; }
	inline int getFrameCount() { return _frameCount; }
	int	getFrame(float frameno, float *pCoeffs);
protected:
	~DataSet();
	void	allocArray(int nPoles);
private:
	int	_nPoles;
	int _frameCount;
	int	_fdesc;
	int	_lpHeaderSize;
	float	*_array;
	int	_oldframe, _endframe;
	int	_framsize;
	int	_fprec;
	int	_recsize;
	int	_bprec;
	int	_bpframe;
	int	_npolem1;
};

