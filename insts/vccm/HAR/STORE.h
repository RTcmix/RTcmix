#define MAX_AUD_IDX 16

class STORE : public Instrument {
  float *buf_loc, *in;
  float dur;
  int t_samp;
  int aud_idx, inchan;
  int branch, skip;

public:
	STORE();
	virtual ~STORE();
	int init(double*, int);
	int configure();
	int run();
	};
