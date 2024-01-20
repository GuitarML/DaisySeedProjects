#pragma once
#ifndef NAM_MODULE_H
#define NAM_MODULE_H

#include <stdint.h>
#include "daisysp.h"
#include "base_effect_module.h"
#include "../NeuralAmpModelerCore/NAM/wavenet.h"

#ifdef __cplusplus

/** @file nam_module.h */

using namespace daisysp;

namespace bkshepherd
{

class NamModule : public BaseEffectModule
{
  public:
    NamModule();
    ~NamModule();

    void Init(float sample_rate) override;
    void ParameterChanged(int parameter_id) override;
    void SelectModel();

    void ProcessMono(float in) override;
    void ProcessStereo(float inL, float inR) override;
    float GetBrightnessForLED(int led_id) override;


  private:

    //bool halfSampleRateSwitch = true;
    float m_gainMin;
    float m_gainMax;

    //float wetMix;
    //float dryMix;

    //float nnLevelAdjust;
    int   m_currentModelindex;

    float m_toneFreqMin;    
    float m_toneFreqMax;

    //Tone tone;       // Low Pass
    //Balance bal;     // Balance for volume correction in filtering

    float m_cachedEffectMagnitudeValue;

    //ImpulseResponse mIR;
    //int   m_currentIRindex;
};
} // namespace bkshepherd
#endif
#endif
