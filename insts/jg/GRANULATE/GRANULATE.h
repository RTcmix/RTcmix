
class GrainStream;

class GRANULATE : public Instrument {

public:
   GRANULATE();
   virtual ~GRANULATE();
   virtual int init(double p[], int n_args);
   virtual int run();

private:
   int _nargs, _skip, _branch;
   bool _stereoOut;
   double _amp, _curwinstart, _curwinend;
   GrainStream *_stream;

   void doupdate();
};

