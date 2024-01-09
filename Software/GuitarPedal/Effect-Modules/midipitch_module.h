#pragma once
#ifndef MIDIPITCH_MODULE_H
#define MIDIPITCH_MODULE_H

#include <stdint.h>
#include "daisysp.h"
#include "base_effect_module.h"
#ifdef __cplusplus

/** @file midipitch_module.h */


using namespace daisysp;

namespace bkshepherd
{


class MidiPitchModule : public BaseEffectModule
{
  public:
    MidiPitchModule();
    ~MidiPitchModule();

    void Init(float sample_rate) override;
    void ParameterChanged(int parameter_id) override;
    void OnNoteOn(float notenumber, float velocity) override;
    void OnNoteOff(float notenumber, float velocity) override;
    void ProcessMono(float in) override;
    void ProcessStereo(float inL, float inR) override;
    float GetBrightnessForLED(int led_id) override;

  private:

    PitchShifter pitch;
    //ReverbSc     verb;

    float m_freqMin;
    float m_freqMax;
    float m_verbMin;
    float m_verbMax;


    float m_cachedEffectMagnitudeValue;
};
} // namespace bkshepherd
#endif
#endif
