/* ADSR Subclass of the Envelope Class, by Perry R. Cook, 1995-96
   This is the traditional ADSR (Attack, Decay, Sustain, Release) envelope.
   It responds to simple KeyOn and KeyOff messages, keeping track of it's
   state. There are two tick (update value) methods, one returns the value,
   and the other returns the state.
*/

#include "ADSR.h"    


ADSR :: ADSR(double srate) : Envelope(srate)
{
   target = 0.0;
   value = 0.0;
   attackRate = 0.001;
   decayRate = 0.001;
   sustainLevel = 0.5;
   releaseRate = 0.01;
   state = ADSR_ATTACK;
}


ADSR :: ~ADSR()
{
}


void ADSR :: keyOn()
{
   target = 1.0;
   rate = attackRate;
   state = ADSR_ATTACK;
}


void ADSR :: keyOff()
{
   target = 0.0;
   rate = releaseRate;
   state = ADSR_RELEASE;
}


void ADSR :: setAttackRate(double aRate)
{
   if (aRate < 0.0) {
      fprintf(stderr, "Negative rates not allowed! Correcting...\n");
      attackRate = -aRate;
   }
   else
      attackRate = aRate;
}


void ADSR :: setDecayRate(double aRate)
{
   if (aRate < 0.0) {
      fprintf(stderr, "Negative rates not allowed! Correcting...\n");
      decayRate = -aRate;
   }
   else
      decayRate = aRate;
}


void ADSR :: setSustainLevel(double aLevel)
{
   if (aLevel < 0.0 ) {
      fprintf(stderr, "Sustain level out of range! Correcting...\n");
      sustainLevel = 0.0;
   }
   else
      sustainLevel = aLevel;
}


void ADSR :: setReleaseRate(double aRate)
{
   if (aRate < 0.0) {
      fprintf(stderr, "Negative rates not allowed! Correcting...\n");
      releaseRate = -aRate;
   }
   else
      releaseRate = aRate;
}


void ADSR :: setAttackTime(double aTime)
{
   if (aTime < 0.0) {
      fprintf(stderr, "Negative times not allowed! Correcting...\n");
      attackRate = (1.0 / _sr) / -aTime;
   }
   else
      attackRate = (1.0 / _sr) / aTime;
}


void ADSR :: setDecayTime(double aTime)
{
   if (aTime < 0.0) {
      fprintf(stderr, "Negative times not allowed! Correcting...\n");
      decayRate = (1.0 / _sr) / -aTime;
   }
   else
      decayRate = (1.0 / _sr) / aTime;
}


void ADSR :: setReleaseTime(double aTime)
{
   if (aTime < 0.0) {
      fprintf(stderr, "Negative times not allowed! Correcting...\n");
      releaseRate = (1.0 / _sr) / -aTime;
   }
   else
      releaseRate = (1.0 / _sr) / aTime;
}


void ADSR :: setAllTimes(
   double attTime, double decTime, double susLevel, double relTime)
{
   this->setAttackTime(attTime);
   this->setDecayTime(decTime);
   this->setSustainLevel(susLevel);
   this->setReleaseTime(relTime);
}


void ADSR :: setTarget(double aTarget)
{
   target = aTarget;
   if (value < target) {
      state = ADSR_ATTACK;
      this->setSustainLevel(target);
      rate = attackRate;
   }
   if (value > target) {
      this->setSustainLevel(target);
      state = ADSR_DECAY;
      rate = decayRate;
   }
}


void ADSR :: setValue(double aValue)
{
   state = ADSR_SUSTAIN;
   target = aValue;
   value = aValue;
   this->setSustainLevel(aValue);
   rate = 0.0;
}


double ADSR :: tick()
{
   if (state == ADSR_ATTACK) {
      value += rate;
      if (value >= target) {
         value = target;
         rate = decayRate;
         target = sustainLevel;
         state = ADSR_DECAY;
      }
   }
   else if (state == ADSR_DECAY) {
      value -= decayRate;
      if (value <= sustainLevel) {
         value = sustainLevel;
         rate = 0.0;
         state = ADSR_SUSTAIN;
      }
   }
   else if (state == ADSR_RELEASE) {
      value -= releaseRate;
      if (value <= 0.0) {
         value = 0.0;
         state = ADSR_END;
      }
   }
   return value;
}


int ADSR :: informTick()
{
   this->tick();
   return state;
}


double ADSR :: lastOut()
{
   return value;
}


