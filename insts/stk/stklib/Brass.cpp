// Brass.C -- hacked version (by BGG) for RTcmix from Perry/Gary's STK

// original head/comment:

/***************************************************/
/*! \class Brass
    \brief STK simple brass instrument class.

    This class implements a simple brass instrument
    waveguide model, a la Cook (TBone, HosePlayer).

    This is a digital waveguide model, making its
    use possibly subject to patents held by
    Stanford University, Yamaha, and others.

    Control Change Numbers: 
       - Lip Tension = 2
       - Slide Length = 4
       - Vibrato Frequency = 11
       - Vibrato Gain = 1
       - Volume = 128

    by Perry R. Cook and Gary P. Scavone, 1995 - 2002.
*/
/***************************************************/

#include "Brass.h"
#include <string.h>
#include <math.h>
#include <ugens.h>

// BGG -- eliminated the ADSR (breath amp now comes in through
// 	the tick() method) and the vibrato
Brass :: Brass(MY_FLOAT lowestFrequency)
{
  length = (long) (Stk::sampleRate() / lowestFrequency + 1);
  delayLine = new DelayA( 0.5 * length, length );

  lipFilter = new BiQuad();
  lipFilter->setGain( 0.03 );
  dcBlock = new PoleZero();
  dcBlock->setBlockZero();

  this->clear();
	maxPressure = (MY_FLOAT) 0.0;
  lipTarget = 0.0;

  // Necessary to initialize variables.
  setFrequency( 220.0 );
}

Brass :: ~Brass()
{
  delete delayLine;
  delete lipFilter;
  delete dcBlock;
}

void Brass :: clear()
{
  delayLine->clear();
  lipFilter->clear();
  dcBlock->clear();
}

void Brass :: setFrequency(MY_FLOAT frequency)
{
  MY_FLOAT freakency = frequency;
  if ( frequency <= 0.0 ) {
    rtcmix_advise("Brass", "setFrequency parameter is less than or equal to zero!");
    freakency = 220.0;
  }

  // Fudge correction for filter delays.
  slideTarget = (Stk::sampleRate() / freakency * 2.0) + 3.0;
  delayLine->setDelay(slideTarget); // play a harmonic

  lipTarget = freakency;
// BGG commented this out for RTcmix PField dyn-control
// not sure why it's here (see setLip() below)
//  lipFilter->setResonance( freakency, 0.997 );
}

void Brass :: setLip(MY_FLOAT frequency)
{
  MY_FLOAT freakency = frequency;
  if ( frequency <= 0.0 ) {
    rtcmix_advise("Brass", "setLip parameter is less than or equal to zero!");
    freakency = 220.0;
  }

  lipFilter->setResonance( freakency, 0.997 );
}

// BGG -- added this method for access to the slide length from RTcmix
void Brass :: setSlide(int slength)
{
	delayLine->setDelay(slength);
}

// BGG -- startBlowing is just used to set the maxPressure in RTcmix
void Brass :: startBlowing(MY_FLOAT amplitude, MY_FLOAT rate)
{
//  adsr->setAttackRate(rate);
  maxPressure = amplitude;
//  adsr->keyOn();
}

// BGG -- stopBlowing and noteOn/Off methods aren't used in RTcmix
void Brass :: stopBlowing(MY_FLOAT rate)
{
//  adsr->setReleaseRate(rate);
//  adsr->keyOff();
}

void Brass :: noteOn(MY_FLOAT frequency, MY_FLOAT amplitude)
{
  setFrequency(frequency);
  this->startBlowing(amplitude, amplitude * 0.001);

#if defined(_STK_DEBUG_)
  // cerr << "Brass: NoteOn frequency = " << frequency << ", amplitude = " << amplitude << endl;
#endif
}

void Brass :: noteOff(MY_FLOAT amplitude)
{
  this->stopBlowing(amplitude * 0.005);

#if defined(_STK_DEBUG_)
  // cerr << "Brass: NoteOff amplitude = " << amplitude << endl;
#endif
}

// the time-varying breath pressure envelope now comes in through a var
// in this tick() method
MY_FLOAT Brass :: tick(float ampPressure)
{
  MY_FLOAT breathPressure = maxPressure * ampPressure;

  MY_FLOAT mouthPressure = 0.3 * breathPressure;
  MY_FLOAT borePressure = 0.85 * delayLine->lastOut();
  MY_FLOAT deltaPressure = mouthPressure - borePressure; // Differential pressure.
  deltaPressure = lipFilter->tick( deltaPressure );      // Force - > position.
  deltaPressure *= deltaPressure;                        // Basic position to area mapping.
  if ( deltaPressure > 1.0 ) deltaPressure = 1.0;         // Non-linear saturation.
  // The following input scattering assumes the mouthPressure = area.
  lastOutput = deltaPressure * mouthPressure + ( 1.0 - deltaPressure) * borePressure;
  lastOutput = delayLine->tick( dcBlock->tick( lastOutput ) );

  return lastOutput;
}

// the dummy tick() method
MY_FLOAT Brass :: tick()
{
return 0.0;
}


// BGG -- RTcmix doesn't use these...
void Brass :: controlChange(int number, MY_FLOAT value)
{
  MY_FLOAT norm = value * ONE_OVER_128;
  if ( norm < 0 ) {
    norm = 0.0;
    rtcmix_advise("Brass", "Control value less than zero!");
  }
  else if ( norm > 1.0 ) {
    norm = 1.0;
    rtcmix_advise("Brass", "Control value greater than 128.0!");
  }

/*  BGG -- commented this stuff out because I didn't compile-in SKINI
	(sorry perry!)
  if (number == __SK_LipTension_)	{ // 2
    MY_FLOAT temp = lipTarget * pow( 4.0, (2.0 * norm) - 1.0 );
    this->setLip(temp);
  }
  else if (number == __SK_SlideLength_) // 4
    delayLine->setDelay( slideTarget * (0.5 + norm) );
  else if (number == __SK_ModFrequency_) // 11
    vibrato->setFrequency( norm * 12.0 );
  else if (number == __SK_ModWheel_ ) // 1
    vibratoGain = norm * 0.4;
  else if (number == __SK_AfterTouch_Cont_) // 128
    adsr->setTarget( norm );
  else
    // cerr << "Brass: Undefined Control Number (" << number << ")!!" << endl;
*/

#if defined(_STK_DEBUG_)
  // cerr << "Brass: controlChange number = " << number << ", value = " << value << endl;
#endif
}
