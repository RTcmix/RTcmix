/* RTcmix  - Copyright (C) 2000  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
 */
#ifndef _INSTRUMENT_H_
#define _INSTRUMENT_H_ 1

#include <RefCounted.h>
#include <bus.h>
#include <buffers.h>
#include <sys/types.h>
#include <rtupdate.h>
#include <maxdispargs.h>

#define MAXNUMPARAMS 100

class heap;
class PFieldSet;

class Instrument : public RefCounted {
protected:

   float          _start;
   float          _dur;
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

   BusSlot        *_busSlot;
#ifdef PFIELD_CLASS
	PFieldSet	  *_pfields;
	static float  s_fArray[MAXDISPARGS];
	static double s_dArray[MAXDISPARGS];
#endif
#ifdef RTUPDATE
   // new RSD variables
   EnvType rsd_env;
   int rise_samps, sustain_samps, decay_samps;
   float *rise_table,r_tabs[2];
   float *sustain_table,s_tabs[2];
   float *decay_table,d_tabs[2];
   float rise_time, sustain_time, decay_time;
   int _startOffset;
   int rsd_samp;
#endif /* RTUPDATE */

private:
   char 		  *_name;	// the name of this instrument
   BUFTYPE        *obufptr;
   short          bufstatus[MAXBUS];
   short          needs_to_run;

#ifdef RTUPDATE
// stores the current index into the pfpath array
   int			  cumulative_size[MAXNUMPARAMS];

// stores the current index for each 'j' value
   int 			  pfpathcounter[MAXNUMPARAMS];

// used in linear interpolation to store the updated value
   double 		  newpvalue[MAXNUMPARAMS];

// used in linear interpolation to store the last updated value
   double 		  oldpvalue[MAXNUMPARAMS][2];

// These are the tables used by the gen interpolation
   float		  *ptables[MAXNUMPARAMS];
   float 		  ptabs[MAXNUMPARAMS][2];

// stores the sample number when updating begins to allow valid indexing into
// the gen array
   int 			  oldsamp[MAXNUMPARAMS];

// stores the current index into the pipath array
   int			  cumulative_isize[MAXNUMPARAMS];

// The instrument number and instrument slot number
   int 			  instnum;
   int			  slot;
	
// used to keep track of which set of data in the pfpath array we are looking 
// at.  This allows multiple calls to note_pfield_path and inst_pfield_path
// with different envelopes specified
   int			  j[MAXNUMPARAMS];
   int 			  k[MAXNUMPARAMS];
#endif /* RTUPDATE */

public:
	// Instruments should use these to access variables.
	inline int		currentFrame() const { return cursamp; }
	inline int		framesToRun() const { return chunksamps; }
	inline int		nSamps() const { return nsamps; }
	inline int		inputChannels() const { return inputchans; }
	inline int		outputChannels() const { return outputchans; }
	// Use this to increment cursamp inside single-frame run loops.
	inline void		increment() { ++cursamp; }
	// Use this to increment cursamp inside block-based run loops.
	inline void	    increment(int amount) { cursamp += amount; }
	inline float	getstart();
	inline float	getdur();
	inline int		getendsamp();
	void			setendsamp(int);
	inline void		setchunk(int);
	inline void		set_ichunkstart(int);
	inline void		set_output_offset(int);

	void 			schedule(heap *rtHeap);
	void			set_bus_config(const char *);
	inline const BusSlot *	getBusSlot() const;
	virtual int		setup(PFieldSet *);				// Called by checkInsts()
	virtual int		init(float *, int, double *);	// Called by setup()
	virtual int		configure();					// Called by inTraverse
	virtual int		run();
	virtual int		update(float *, int *, double *);	// Called by run()

	int				exec(BusType bus_type, int bus);
	void			addout(BusType bus_type, int bus);
	bool			isDone() const { return cursamp >= nsamps; }
	const char *	name() const { return _name; }

#ifdef RTUPDATE
   float			rtupdate(int, int);  // tag, p-field for return value
   void				pf_path_update(int tag, int pval);
   void				pi_path_update(int pval);
   void				set_instnum(char* name);

   void				RSD_setup(int RISE_SLOT, int SUSTAIN_SLOT,
              				  int DECAY_SLOT, float duration);
   void				RSD_check();
   float			RSD_get();
#endif /* RTUPDATE */

// BGG -- added this for Ortgetin object support (see lib/Ortgetin.C)
   friend			class Ortgetin;

protected:
   // Methods which are called from within other methods
	Instrument();
	virtual		~Instrument();	// never called directly -- use unref()
   
	// This is called by set_bus_config() ONLY.
    void			setName(const char *name);

   // This is called by the public init() when it is not redefined

	virtual int		init(float *, int);

	static int		rtsetoutput(float, float, Instrument *);
	static int		rtsetinput(float, Instrument *);
	static int		rtinrepos(Instrument *, int, int);
	static int		rtgetin(float *, Instrument *, int);
	int rtaddout(BUFTYPE samps[]);  				// replacement for old rtaddout
	int rtbaddout(BUFTYPE samps[], int length);	// block version of same

private:
   void				gone(); // decrements reference to input soundfile
};

/* ------------------------------------------------------------- getstart --- */
inline float Instrument::getstart()
{
   return _start;
}

/* --------------------------------------------------------------- getdur --- */
inline float Instrument::getdur()
{
   return _dur;
}

/* ----------------------------------------------------------- getendsamp --- */
inline int Instrument::getendsamp()
{
   return endsamp;
}


/* ------------------------------------------------------------- setchunk --- */
inline void Instrument::setchunk(int csamps)
{
   chunksamps = csamps;
}

/* ------------------------------------------------------ set_ichunkstart --- */
inline void Instrument::set_ichunkstart(int csamps)
{
   i_chunkstart = csamps;
}

/* ---------------------------------------------------- set_output_offset --- */
inline void Instrument::set_output_offset(int offset)
{
   output_offset = offset;
}

inline const BusSlot *	Instrument::getBusSlot() const
{
	return _busSlot;
}

#endif /* _INSTRUMENT_H_  */

