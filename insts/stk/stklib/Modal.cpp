// Modal.C -- hacked version (by BGG) for RTcmix from Perry/Gary's STK
//	-- this is the superclass for ModalBar

// original head/comment:

/***************************************************/
/*! \class Modal
    \brief STK resonance model instrument.

    This class contains an excitation wavetable,
    an envelope, an oscillator, and N resonances
    (non-sweeping BiQuad filters), where N is set
    during instantiation.

    by Perry R. Cook and Gary P. Scavone, 1995 - 2002.
*/
/***************************************************/

#include "Modal.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ugens.h>

Modal :: Modal(int modes)
  : nModes(modes)
{
  if ( nModes <= 0 ) {
    char msg[256];
    sprintf(msg, "Modal: Invalid number of modes (%d) argument to constructor!", modes);
    handleError(msg, StkError::FUNCTION_ARGUMENT);
  }

  // We don't make the excitation wave here yet, because we don't know
  // what it's going to be.
// BGG -- for RTcmix, we will be passing in the samples from the excitation
// function through the tick(...) method below

  ratios = (MY_FLOAT *) new MY_FLOAT[nModes];
  radii = (MY_FLOAT *) new MY_FLOAT[nModes];
  filters = (BiQuad **) calloc( nModes, sizeof(BiQuad *) );
  for (int i=0; i<nModes; i++ ) {
    filters[i] = new BiQuad;
    filters[i]->setEqualGainZeroes();
  }

// BGG -- envelope also passed in from RTcmix (makegen)
//  envelope = new Envelope;
  onepole = new OnePole;

/* BGG -- none of this vibrato nonsense! :-)
  // Concatenate the STK RAWWAVE_PATH to the rawwave file
  char file[128];
  strcpy(file, RAWWAVE_PATH);
  vibrato = new WaveLoop( strcat(file,"sinewave.raw"), TRUE);

  // Set some default values.
  vibrato->setFrequency( 6.0 );
  vibratoGain = 0.0;
*/
  directGain = 0.0;
  masterGain = 1.0;
  baseFrequency = 440.0;

  this->clear();

  stickHardness =  0.5;
  strikePosition = 0.561;
}  

Modal :: ~Modal()
{
//  delete envelope; 
  delete onepole;
//  delete vibrato;

  delete [] ratios;
  delete [] radii;
  for (int i=0; i<nModes; i++ ) {
    delete filters[i];
  }
  free(filters);
}

void Modal :: clear()
{    
  onepole->clear();
  for (int i=0; i<nModes; i++ )
    filters[i]->clear();
}

void Modal :: setFrequency(MY_FLOAT frequency)
{
  baseFrequency = frequency;
  for (int i=0; i<nModes; i++ )
    this->setRatioAndRadius(i, ratios[i], radii[i]);
}

void Modal :: setRatioAndRadius(int modeIndex, MY_FLOAT ratio, MY_FLOAT radius)
{
  if ( modeIndex < 0 ) {
    rtcmix_advise("Modal", "setRatioAndRadius modeIndex parameter is less than zero!");
    return;
  }
  else if ( modeIndex >= nModes ) {
    rtcmix_advise("Modal", "setRatioAndRadius modeIndex parameter is greater than the number of operators!");
    return;
  }

  MY_FLOAT nyquist = Stk::sampleRate() / 2.0;
  MY_FLOAT temp;

  if (ratio * baseFrequency < nyquist) {
    ratios[modeIndex] = ratio;
  }
  else {
    temp = ratio;
    while (temp * baseFrequency > nyquist) temp *= (MY_FLOAT) 0.5;
    ratios[modeIndex] = temp;
#if defined(_STK_DEBUG_)
    // cerr << "Modal : Aliasing would occur here ... correcting." << endl;
#endif
  }
  radii[modeIndex] = radius;
  if (ratio < 0) 
    temp = -ratio;
  else
    temp = ratio*baseFrequency;

  filters[modeIndex]->setResonance(temp, radius);
}

void Modal :: setMasterGain(MY_FLOAT aGain)
{
  masterGain = aGain;
}

void Modal :: setDirectGain(MY_FLOAT aGain)
{
  directGain = aGain;
}

void Modal :: setModeGain(int modeIndex, MY_FLOAT gain)
{
  if ( modeIndex < 0 ) {
    rtcmix_advise("Modal", "setModeGain modeIndex parameter is less than zero!");
    return;
  }
  else if ( modeIndex >= nModes ) {
    rtcmix_advise("Modal", "setModeGain modeIndex parameter is greater than the number of operators!");
    return;
  }

  filters[modeIndex]->setGain(gain);
}

void Modal :: strike(MY_FLOAT amplitude)
{
  MY_FLOAT gain = amplitude;
  if ( amplitude < 0.0 ) {
    rtcmix_advise("Modal", "strike amplitude is less than zero!");
    gain = 0.0;
  }
  else if ( amplitude > 1.0 ) {
    rtcmix_advise("Modal", "strike amplitude is greater than 1.0!");
    gain = 1.0;
  }

//  envelope->setRate(1.0);
//  envelope->setTarget(gain);
  onepole->setPole(1.0 - gain);
//  envelope->tick();
// BGG -- wave no longer exists because we pass in the excitation function
//  wave->reset();

  MY_FLOAT temp;
  for (int i=0; i<nModes; i++) {
    if (ratios[i] < 0)
      temp = -ratios[i];
    else
      temp = ratios[i] * baseFrequency;
    filters[i]->setResonance(temp, radii[i]);
  }
}

void Modal :: noteOn(MY_FLOAT frequency, MY_FLOAT amplitude)
{
  this->strike(amplitude);
  this->setFrequency(frequency);

#if defined(_STK_DEBUG_)
  // cerr << "Modal: NoteOn frequency = " << frequency << ", amplitude = " << amplitude << endl;
#endif
}

void Modal :: noteOff(MY_FLOAT amplitude)
{
  // This calls damp, but inverts the meaning of amplitude (high
  // amplitude means fast damping).
  this->damp(1.0 - (amplitude * 0.03));

#if defined(_STK_DEBUG_)
  // cerr << "Modal: NoteOff amplitude = " << amplitude << endl;
#endif
}

void Modal :: damp(MY_FLOAT amplitude)
{
  MY_FLOAT temp;
  for (int i=0; i<nModes; i++) {
    if (ratios[i] < 0)
      temp = -ratios[i];
    else
      temp = ratios[i] * baseFrequency;
    filters[i]->setResonance(temp, radii[i]*amplitude);
  }
}

// BGG -- this is the RTcmix tick() method -- we pass in the amplitude
// to enable a makegen control, and the excitation function (as samples)
// via the excite variable
MY_FLOAT Modal :: tick(MY_FLOAT amp, MY_FLOAT excite)
{
//  MY_FLOAT temp = masterGain * onepole->tick(wave->tick() * envelope->tick());
// BGG, yeah, I know this could be done more efficiently...
  MY_FLOAT temp = masterGain * onepole->tick(excite * amp);

  MY_FLOAT temp2 = 0.0;
  for (int i=0; i<nModes; i++)
    temp2 += filters[i]->tick(temp);

  temp2  -= temp2 * directGain;
  temp2 += directGain * temp;

// BGG -- no vibrato, but note that it is AM
/*
  if (vibratoGain != 0.0)	{
    // Calculate AM and apply to master out
    temp = 1.0 + (vibrato->tick() * vibratoGain);
    temp2 = temp * temp2;
  }
*/
    
  lastOutput = temp2;
  return lastOutput;
}

// BGG -- dummy for RTcmix
MY_FLOAT Modal :: tick()
{
return 0.0;
}
