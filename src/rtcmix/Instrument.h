/* RTcmix  - Copyright (C) 2000  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
 */
#ifndef _INSTRUMENT_H_
#define _INSTRUMENT_H_ 1

#include <bus.h>
#include <buffers.h>
#include <sys/types.h>


class Instrument {
public:
   float          start;
   float          dur;
   int            cursamp;
   int            chunksamps;
   int            i_chunkstart;   // we need this for rtperf
   int            endsamp;
   int            nsamps;
   int            output_offset;

   int            sfile_on;        // a soundfile is open (for closing later)
   int            fdIndex;         // index into unix input file desc. table
   off_t          fileOffset;      // current offset in file for this inst

   double         inputsr;
   int            inputchans;
   int            outputchans;

   int            mytag;           // for note tagging/rtupdate() 

   BUFTYPE        *outbuf;         // private interleaved buffer

   BusSlot        *bus_config;

private:
   BUFTYPE        *obufptr;
   short          bufstatus[MAXBUS];
   short          needs_to_run;

public:
   Instrument();
   virtual ~Instrument();
   void set_bus_config(const char *);
   virtual int init(float *, short);
   virtual int run();

   void exec(BusType bus_type, int bus);
   int rtaddout(BUFTYPE samps[]);  // replacement for old rtaddout
   void addout(BusType bus_type, int bus);

   float getstart();
   float getdur();
   int getendsamp();
   void setendsamp(int);
   void setchunk(int);
   void set_ichunkstart(int);
   void set_output_offset(int);
private:
   void gone();                    // decrements reference to input soundfile

};


// prototypes for functions called by instruments
// probably should move these somewhere else
int rtsetoutput(float, float, Instrument *);
int rtsetinput(float, Instrument *);
int rtinrepos(Instrument *, int, int);
int rtgetin(float *, Instrument *, int);
float rtupdate(int, int);  // tag, p-field for return value


#endif /* _INSTRUMENT_H_  */
