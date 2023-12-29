#pragma once
#ifndef IR_MODULE_H
#define IR_MODULE_H

#include <stdint.h>
#include "daisysp.h"
#include "base_effect_module.h"
#include "ImpulseResponse/ImpulseResponse.h"
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
    void ParameterChanged(int parameter_id) override;
    void SelectIR();
    void ProcessMono(float in) override;
    void ProcessStereo(float inL, float inR) override;
    float GetBrightnessForLED(int led_id) override;

  private:

    ImpulseResponse mIR;

    float m_gainMin;
    float m_gainMax;
    int   m_currentIRindex;

    float m_cachedEffectMagnitudeValue;
};
} // namespace bkshepherd
#endif
#endif
