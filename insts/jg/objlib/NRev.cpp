/* NRev Reverb Subclass
   by Tim Stilson, 1998
     based on CLM NRev
   Integrated into STK by Gary Scavone
   (changed a bit in accordance with clm nrev.ins  -JGG)

   This is based on some of the famous Stanford CCRMA reverbs (NRev,
   KipRev) all based on the the Chowning/Moorer/Schroeder reverberators,
   which use networks of simple allpass and comb delay filters.  This
   particular arrangement consists of 6 comb filters in parallel,
   followed by 3 allpass filters, a lowpass filter, and another allpass
   in series, followed by two allpass filters in parallel with
   corresponding right and left outputs.
*/

#include "NRev.h"


NRev :: NRev(double srate, double T60) : Reverb(srate)
{
   double srscale = _sr / 25641.0;
   int lens[15] = { 1433, 1601, 1867, 2053, 2251, 2399,
                    347, 113, 37, 59, 53, 43, 37, 29, 19 };

   for (int i = 0; i < 15; i++) {
      int val = (int)floor(srscale * lens[i]);
      if ((val & 1) == 0)
         val++;
      while (!this->isprime(val))
         val += 2;
      lens[i] = val;
   }
   for (int i = 0; i < 6; i++) {
      CdelayLine[i] = new DLineN((long)(lens[i]) + 2);
      CdelayLine[i]->setDelay((long)(lens[i]));
      combCoef[i] = pow(10, (-3 * lens[i] / (T60 * _sr)));
   }
   for (int i = 0; i < 6; i++) {
      APdelayLine[i] = new DLineN((long)(lens[i + 6]) + 2);
      APdelayLine[i]->setDelay((long)(lens[i + 6]));
   }
#ifdef NOMORE // don't want to import NCHANS anymore  -JGG
   // but 4-chan unimplemented in tick (see reverb() in snd-dac.c in snd)
   if (NCHANS == 4) {
      APdelayLine[3]->setDelay((long)(lens[10]));      // as per clm nrev.ins
      APdelayLine[6] = new DLineN((long)(lens[13]) + 2);
      APdelayLine[6]->setDelay((long)(lens[13]));
      APdelayLine[7] = new DLineN((long)(lens[14]) + 2);
      APdelayLine[7]->setDelay((long)(lens[14]));
   }
   else
      APdelayLine[6] = APdelayLine[7] = NULL;
#else
   APdelayLine[6] = APdelayLine[7] = NULL;
#endif

   allPassCoeff = 0.7;
   effectMix = 0.3;
   this->clear();
}


NRev :: ~NRev()
{
   for (int i = 0; i < 6; i++)
      delete CdelayLine[i];
   for (int i = 0; i < 8; i++)
      delete APdelayLine[i];
}


void NRev :: clear()
{
   for (int i = 0; i < 6; i++)
      CdelayLine[i]->clear();
   for (int i = 0; i < 6; i++)
      APdelayLine[i]->clear();
#ifdef NOMORE // don't want to import NCHANS anymore  -JGG
   if (NCHANS == 4) {
      APdelayLine[6]->clear();
      APdelayLine[7]->clear();
   }
#endif
   lastOutL = 0.0;
   lastOutR = 0.0;
   lpLastout = 0.0;
}


void NRev :: setEffectMix(double mix)
{
   effectMix = mix;
}


double NRev :: lastOutput()
{
   return (lastOutL + lastOutR) * 0.5;
}


double NRev :: lastOutputL()
{
   return lastOutL;
}


double NRev :: lastOutputR()
{
   return lastOutR;
}


double NRev :: tick(double input)
{
   double temp, temp0, temp1, temp2, temp3;

   temp0 = 0.0;
   for (int i = 0; i < 6; i++) {
      temp = input + (combCoef[i] * CdelayLine[i]->lastOut());
      temp0 += CdelayLine[i]->tick(temp);
   }
   for (int i = 0; i < 3; i++) {
      temp = APdelayLine[i]->lastOut();
      temp1 = allPassCoeff * temp;
      temp1 += temp0;
      APdelayLine[i]->tick(temp1);
      temp0 = -(allPassCoeff * temp1) + temp;
   }
   lpLastout = 0.7 * lpLastout + 0.3 * temp0;       // onepole LP filter
   temp = APdelayLine[3]->lastOut();
   temp1 = allPassCoeff * temp;
   temp1 += lpLastout;
   APdelayLine[3]->tick(temp1);
   temp1 = -(allPassCoeff * temp1) + temp;
    
   temp = APdelayLine[4]->lastOut();
   temp2 = allPassCoeff * temp;
   temp2 += temp1;
   APdelayLine[4]->tick(temp2);
   lastOutL = effectMix * (-(allPassCoeff * temp2) + temp);
    
   temp = APdelayLine[5]->lastOut();
   temp3 = allPassCoeff * temp;
   temp3 += temp1;
   APdelayLine[5]->tick(temp3);
   lastOutR = effectMix * (-(allPassCoeff * temp3) + temp);

   temp = (1.0 - effectMix) * input;
   lastOutL += temp;
   lastOutR += temp;
     
   return (lastOutL + lastOutR) * 0.5;
}


