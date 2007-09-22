class Ooscili;

class MULTIWAVE : public Instrument {
   int     nargs, branch, numpartials;
   double  overall_amp;
   double  *amp, *pan;
   Ooscili **oscil;

   int usage();
   void doupdate();

public:
   MULTIWAVE();
   virtual ~MULTIWAVE();
   virtual int init(double p[], int n_args);
   virtual int run();
};

