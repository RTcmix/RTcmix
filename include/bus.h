#ifndef _BUS_H_ 
#define _BUS_H_ 1

#include <globals.h>
#include <Lockable.h>

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
   BUS_AUX_OUT
};

enum IBusClass {
  TO_AUX,
  AUX_TO_AUX,
  TO_OUT,
  TO_AUX_AND_OUT,
  UNKNOWN
};

class BusSlot : public Lockable {
public:
	BusSlot();
	inline IBusClass Class() const;
	BusSlot    *next;
	BusSlot    *prev;
	short      in_count;
	short      in[MAXBUS];
	short      out_count;
	short      out[MAXBUS];
	short      auxin_count;
	short      auxin[MAXBUS];
	short      auxout_count;
	short      auxout[MAXBUS];
	int        refcount;
};

inline IBusClass 
BusSlot::Class() const
{
  if (this == 0)
	return UNKNOWN;
  if (auxin_count > 0 && auxout_count > 0)
	return AUX_TO_AUX;
  if (auxout_count > 0 && out_count > 0)
	return TO_AUX_AND_OUT;
  if (auxout_count > 0)
	return TO_AUX;
  if (out_count > 0)
	return TO_OUT;
  return UNKNOWN;
}

struct BusQueue {
	BusQueue(char *name, BusSlot *theQueue);
	~BusQueue();
	char *instName() { return inst_name; }
	char *inst_name;
	BusSlot *queue;
	BusQueue *next;
};

struct CheckNode {
   CheckNode() : bus_list(new short[MAXBUS]), bus_count(0) {}
   CheckNode(short *list, short count) : bus_list(list), bus_count(count) {}
   short *bus_list;
   short bus_count;
};

struct CheckQueue {
   CheckQueue(CheckNode *theNode) : node(theNode), next(NULL) {}
   CheckNode *node;
   CheckQueue *next;
};

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* exported functions */
BusSlot *get_bus_config(const char *inst_name);
ErrCode parse_bus_name(char*, BusType*, int*, int*);


#ifdef __cplusplus
} /* extern "C" */
#endif
#endif /* _BUS_H_ */
