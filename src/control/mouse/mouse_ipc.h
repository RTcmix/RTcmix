#ifndef _MOUSE_IPC_H_
#define _MOUSE_IPCH_

#include <labels.h>

#define SOCK_PORT		9797

enum {
	kPacketConfigureXLabelPrefix = 0,
	kPacketConfigureYLabelPrefix,
	kPacketConfigureXLabelUnits,
	kPacketConfigureYLabelUnits,
	kPacketConfigureXLabelPrecision,
	kPacketConfigureYLabelPrecision,
	kPacketUpdateXLabel,
	kPacketUpdateYLabel,
	kPacketMouseCoords,
	kPacketQuit
};

typedef struct {
	short type;								// from enum above
	short id;								// assigned by RTcmixMouse object
	union {
		int precision;						// kPacketConfigure?LabelPrecision
		double value;						// kPacketUpdate?Label
		struct s_point {					// kPacketMouseCoords
			double x;
			double y;
		} point;
		char str[PART_LABEL_LENGTH];	// kPacketConfigure?LabelPrefix and *Units
	} data;
} MouseSockPacket;						// should be 20 bytes

#endif // _MOUSE_IPCH_