#define MAX_AUD_IDX 16

class RECORD : public Instrument {
  float *buf_loc, *in;
  float dur;
  int aud_idx, inchan;

public:
	RECORD();
	virtual ~RECORD();
	int init(double*, int);
	int configure();
	int run();
	};
