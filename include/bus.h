#ifndef _BUS_H_ 
#define _BUS_H_ 1

/* MAXBUS is the maximum number of buses you can ever request dynamically. Some arrays are still statically set using this */
#define MINBUS 8
#define MAXBUS 129

#if defined(EMBEDDED)
#define DEFAULT_MAXBUS 17
#else
#define DEFAULT_MAXBUS 65
#endif

#ifdef __cplusplus

enum ErrCode {
   NO_ERR = 0,
   INVAL_BUS_ERR,
   INVAL_BUS_CHAN_ERR,
   LOOP_ERR,
   UNKNOWN_ERR
};

enum BusType {
   BUS_IN,
   BUS_OUT,
   BUS_AUX_IN,
   BUS_AUX_OUT,
   BUS_NONE_IN,
   BUS_NONE_OUT
};

enum IBusClass {
  TO_AUX,
  AUX_TO_AUX,
  TO_OUT,
  TO_AUX_AND_OUT,
  UNKNOWN
};

struct CheckNode;

struct BusConfig
{
	BusConfig() : In_Config(0), HasChild(false), HasParent(false), AuxInUse(false), AuxOutInUse(false),
				  OutInUse(false), RevPlay(0) {}
	/* Bus graph, parsed by check_bus_inst_config */
	/* Allows loop checking ... and buffer playback order? */
	CheckNode *	In_Config;
	/* State for bus graph */
	bool		HasChild;
	bool		HasParent;
	bool		AuxInUse;
	bool		AuxOutInUse;
	bool		OutInUse;
	short		RevPlay;
};

class BusSlot;

extern "C" {
/* exported functions */
BusSlot *get_bus_config(const char *inst_name);
ErrCode parse_bus_name(char*, BusType*, int*, int*);
} /* extern "C" */
#endif	/* __cplusplus */

#endif /* _BUS_H_ */
