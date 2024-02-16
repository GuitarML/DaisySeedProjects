#pragma once
#ifndef KEY_SAMPLER_MODULE_H
#define KEY_SAMPLER_MODULE_H

#include <stdint.h>
#include "daisysp.h"
#include "base_effect_module.h"
#ifdef __cplusplus

/** @file key_sampler_module.h */


using namespace daisysp;

namespace bkshepherd
{



class KeySamplerModule : public BaseEffectModule
{
  public:
   KeySamplerModule();
    ~KeySamplerModule();

    void Init(float sample_rate) override;
    void OnNoteOn(float notenumber, float velocity) override;
    void OnNoteOff(float notenumber, float velocity) override;
    void ResetBuffer();
    void RecordSample(float input);
    void ProcessNewSample();
    float SamplePlayBack();
    void ProcessMono(float in) override;
    void ProcessStereo(float inL, float inR) override;


    float GetBrightnessForLED(int led_id) override;

  private:

    bool previousRecordState;
    int  bufferCounter;
    int  currentSampleLen;
    bool keysPlaying[25] = {false};
    int  keySampleIndex[25] = { 0 };
    float  keyAmplitude[25] = { 0.0 };
    int  psDelay = 1500; // set delay time in samples of the pitchshifter

    //float m_freqMin;
    //float m_freqMax;

    PitchShifter pitchshifter;


    float m_cachedEffectMagnitudeValue;
};
} // namespace bkshepherd
#endif
#endif
