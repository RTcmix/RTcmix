#define MAX_AUD_IDX 16

class STORE : public Instrument {
  float *buf_loc, *in;
  float dur;
  int t_samp;
  int aud_idx, inchan;

public:
	STORE();
	virtual ~STORE();
	int init(float*, int);
	int run();
	};
