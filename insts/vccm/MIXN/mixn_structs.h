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
  double atime;
  double xvel;
  double yvel;
} loc;

typedef struct rfact {
  double factor;
  double accel;
  double time;
} rfact;
