// DataSet.h

class DataSet
{
public:
	DataSet();
	void ref();
	void unref();
	int	open(const char *fileName, int npoleGuess, float sampRate);
	inline int getNPoles() { return _nPoles; }
	int	getFrame(float frameno, float *pCoeffs);
protected:
	~DataSet();
	void	allocArray(int nPoles);
private:
	int _refCount;
	int	_nPoles;
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

