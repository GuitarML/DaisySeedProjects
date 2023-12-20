#pragma once
#ifndef NEURAL_NET_MODULE_H
#define NEURAL_NET_MODULE_H

#include <stdint.h>
#include "daisysp.h"
#include "base_effect_module.h"
#include <RTNeural/RTNeural.h>

#ifdef __cplusplus

/** @file modulated_tremolo_module.h */

using namespace daisysp;


namespace bkshepherd
{

class NeuralNetModule : public BaseEffectModule
{
  public:
    NeuralNetModule();
    ~NeuralNetModule();

    void Init(float sample_rate) override;
    void ParameterChanged(int parameter_id) override;
    void SelectModel();
    void CalculateMix();
    void CalculateTone();
    void ProcessMono(float in) override;
    void ProcessStereo(float inL, float inR) override;
    float GetBrightnessForLED(int led_id) override;

  private:


    float m_gainMin;
    float m_gainMax;

    float wetMix;
    float dryMix;

    float nnLevelAdjust;

    float m_toneFreqMin;    
    float m_toneFreqMax;

    Tone tone;       // Low Pass
    Balance bal;     // Balance for volume correction in filtering

    float m_cachedEffectMagnitudeValue;
};
} // namespace bkshepherd
#endif
#endif
