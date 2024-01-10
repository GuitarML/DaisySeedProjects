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

    int numVoices = 3;
    int maxVoices = 3;
    //PitchShifter voices[3];
    //PitchShifter pitch2;
    //PitchShifter pitch3;

    float keyNums[3] = {0.0};
    int numKeys = 0;
    //ReverbSc     verb;  // Freezes if I include both Pitchshifter and ReverbSc..

    float m_freqMin;
    float m_freqMax;
    float m_verbMin;
    float m_verbMax;

    bool voicesActive[3] = {false};
    float voiceKeys[3] = {0.0};

    Adsr       env_[3];
    bool env_gate_[3] = {false};

    float m_cachedEffectMagnitudeValue;
};
} // namespace bkshepherd
#endif
#endif
