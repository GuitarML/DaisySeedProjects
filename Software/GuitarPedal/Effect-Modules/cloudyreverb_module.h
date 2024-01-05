#pragma once
#ifndef CLOUDYREVERB_MODULE_H
#define CLOUDYREVERB_MODULE_H

#include <stdint.h>
#include "daisysp.h"
#include "base_effect_module.h"
#include "clouds/dsp/frame.h"
#include "rings/dsp/fx/reverb.h"
using namespace rings;

#ifdef __cplusplus

/** @file cloudyreverb_module.h */
// Implentation of reverb from eurorack

using namespace daisysp;

namespace bkshepherd
{

class CloudyReverbModule : public BaseEffectModule
{
  public:
    CloudyReverbModule();
    ~CloudyReverbModule();

    void Init(float sample_rate) override;
    void ParameterChanged(int parameter_id) override;
    void ProcessMono(float in) override;
    void ProcessStereo(float inL, float inR) override;
    float GetBrightnessForLED(int led_id) override;

  private:
    Reverb cloudy_reverb;
    uint16_t reverb_buffer[65536];

    float m_verbMin;
    float m_verbMax;

};
} // namespace bkshepherd
#endif
#endif
