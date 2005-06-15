class Ooscil;

class WAVY : public Instrument {
	enum {
		kNoCombineMode,
		kAddMode,
		kSubtractMode,
		kMultiplyMode
	};

	typedef float (*CombineFunction)(float, float);
	static inline float nocombine(float, float);
	static inline float add(float, float);
	static inline float subtract(float, float);
	static inline float multiply(float, float);
	void setCombineFunc(CombineFunction func) { _combiner = func; }

	int _nargs, _branch, _mode;
	float _amp, _pan;
	double _freqraw, _phaseOffset;
	Ooscil *_oscil1, *_oscil2;
	CombineFunction _combiner;

	int usage() const;
	int getCombinationMode() const;
	void doupdate();

public:
	WAVY();
	virtual ~WAVY();
	virtual int init(double p[], int n_args);
	virtual int run();
};

inline float WAVY::nocombine(float a, float) { return a; }		// ignore <b>
inline float WAVY::add(float a, float b) { return a + b; }
inline float WAVY::subtract(float a, float b) { return a - b; }
inline float WAVY::multiply(float a, float b) { return a * b; }

