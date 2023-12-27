#pragma once
#ifndef IR_MODULE_H
#define IR_MODULE_H

#include <stdint.h>
#include "daisysp.h"
#include "base_effect_module.h"
#ifdef __cplusplus

/** @file ir_module.h */

using namespace daisysp;

namespace bkshepherd
{

class IRModule : public BaseEffectModule
{
  public:
    IRModule();
    ~IRModule();

    void Init(float sample_rate) override;
    void ProcessMono(float in) override;
    void ProcessStereo(float inL, float inR) override;
    //void SetTempo(uint32_t bpm) override;
    float GetBrightnessForLED(int led_id) override;

  private:

    float m_gainMin;
    float m_gainMax;

    float m_cachedEffectMagnitudeValue;
};
} // namespace bkshepherd
#endif
#endif
