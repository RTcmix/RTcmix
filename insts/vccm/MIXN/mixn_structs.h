#define MAXRATES 32
#define MAXLOCS 32
#define MAXSPEAKS 16

typedef struct pt {
  double x;
  double y;
} pt;

typedef struct loc {
  pt point;
  double time;
  double xvel;
  double yvel;
  double startsamp;
} loc;

typedef struct rfact {
  double factor;
  double time;
} rfact;
