#ifndef _BUS_H_ 
#define _BUS_H_ 1

#define MAXBUS 16

typedef enum {
   NO_ERR = 0,
   INVAL_BUS_ERR,
   INVAL_BUS_CHAN_ERR,
   LOOP_ERR,
   UNKNOWN_ERR
} ErrCode;

typedef enum {
   BUS_IN,
   BUS_OUT,
   BUS_AUX_IN,
   BUS_AUX_OUT
} BusType;


typedef struct _BusSlot BusSlot;

struct _BusSlot {
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

typedef struct _BusQueue BusQueue;

struct _BusQueue {
   char *inst_name;
   BusSlot *queue;
   BusQueue *next;
};

typedef struct _CheckNode CheckNode;

struct _CheckNode {
   short *bus_list;
   short bus_count;
};

typedef struct _CheckQueue CheckQueue;

struct _CheckQueue {
   CheckNode *node;
   CheckQueue *next;
};


/* exported functions */
ErrCode parse_bus_name(char*, BusType*, int*, int*);

#endif /* _BUS_H_ */
