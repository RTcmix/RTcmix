#include <math.h>    // for M_PI

#define MAX_SPEAKERS 16

class Speaker {
public:
   Speaker(const int channel, const double angle, const double distance);
   ~Speaker();

   int channel() const { return _channel; }
   void setChannel(const int channel) { _channel = channel; }
   double gain() const { return _gain; }
   void setGain(const double gain) { _gain = gain; }
   double angle() const { return _angle; }
   double angleDegrees() const { return _angle / (M_PI * 2.0) * 360.0; }
   void setAngle(const double angle) { _angle = angle; }
   double prevAngle() const { return _prevAngle; }
   void setPrevAngle(const double angle) { _prevAngle = angle; }
   double nextAngle() const { return _nextAngle; }
   void setNextAngle(const double angle) { _nextAngle = angle; }
   double distance() const { return _distance; }
   void setDistance(const double distance) { _distance = distance; }

private:
   int _channel;
   double _angle;
   double _prevAngle;
   double _nextAngle;
   double _distance;
   double _gain;
};

int NPAN_get_speakers(int *, Speaker *[], double *);

