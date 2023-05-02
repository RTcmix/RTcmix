#include <RefCounted.h>
#include <Lockable.h>

// DataSet.h

class DataSet : public RefCounted, public Lockable
{
public:
	DataSet();
	off_t	open(const char *fileName, int npoleGuess, float sampRate);
	int getNPoles() const { return _nPoles; }
	off_t getFrameCount() const { return _frameCount; }
	int	getFrame(double frameno, float *pCoeffs);
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
	bool _swapped;
};

