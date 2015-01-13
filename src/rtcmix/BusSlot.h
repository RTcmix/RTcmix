/* RTcmix  - Copyright (C) 2000  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/

#ifndef _BUSSLOT_H_
#define _BUSSLOT_H_

#include <RefCounted.h>
#include <Lockable.h>
#include <bus.h>

class BusSlot : public RefCounted, public Lockable {
public:
	BusSlot(int inBusCount);
	inline		IBusClass Class() const;
	inline 	   short * getBusList(BusType type, int *busCount);
	BusSlot    *next;
	BusSlot    *prev;
	short      *in;
	short      *out;
	short      *auxin;
	short      *auxout;
	short      in_count;
	short      out_count;
	short      auxin_count;
	short      auxout_count;
protected:
	virtual ~BusSlot();
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

inline short * 
BusSlot::getBusList(BusType type, int *busCount)
{
	if (type == BUS_AUX_OUT) {
		*busCount = auxout_count;
		return auxout;
	}
	else {	// type == BUS_OUT
		*busCount = out_count;
		return out;
	}
}

#endif	// _BUSSLOT_H_
