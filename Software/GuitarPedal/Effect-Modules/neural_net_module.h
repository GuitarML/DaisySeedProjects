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

//RTNeural::ModelT<float, 1, 1,
//    RTNeural::GRULayerT<float, 1, 9>,
//    RTNeural::DenseT<float, 9, 1>> model;

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
    void ProcessMono(float in) override;
    void ProcessStereo(float inL, float inR) override;
    float GetBrightnessForLED(int led_id) override;

  private:


    float m_gainMin;
    float m_gainMax;

    float m_cachedEffectMagnitudeValue;
};
} // namespace bkshepherd
#endif
#endif
