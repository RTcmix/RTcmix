#define MAX_AUD_IDX 16

class HOLD : public Instrument {
  float *buf_loc, *in;
  float dur;
  int t_samp;
  int aud_idx, inchan;

public:
	HOLD();
	virtual ~HOLD();
	int init(float*, int);
	int run();
	};
