#include "cloudyreverb_module.h"

using namespace bkshepherd;

static const int s_paramCount = 6;
static const ParameterMetaData s_metaData[s_paramCount] = {{name: "In Gain", valueType: ParameterValueType::FloatMagnitude, valueBinCount: 0, defaultValue: 64, knobMapping: 0, midiCCMapping: 21},
                                                           {name: "Mix", valueType: ParameterValueType::FloatMagnitude, valueBinCount: 0, defaultValue: 64, knobMapping: 1, midiCCMapping: 22},
                                                           {name: "Level", valueType: ParameterValueType::FloatMagnitude, valueBinCount: 0, defaultValue: 64, knobMapping: 2, midiCCMapping: 23},
                                                           {name: "Time", valueType: ParameterValueType::FloatMagnitude, valueBinCount: 0, defaultValue: 64, knobMapping: 3, midiCCMapping: 24},
                                                           {name: "Diffusion", valueType: ParameterValueType::FloatMagnitude, valueBinCount: 0, defaultValue: 64, knobMapping: 4, midiCCMapping: 25},
                                                           {name: "LP Filter", valueType: ParameterValueType::FloatMagnitude, valueBinCount: 0, defaultValue: 64, knobMapping: 5, midiCCMapping: 26}};

// Default Constructor
CloudyReverbModule::CloudyReverbModule() : BaseEffectModule(),
                                        m_verbMin(0.5f),
                                        m_verbMax(1.0f)

{
    // Set the name of the effect
    m_name = "CloudyVerb";

    // Setup the meta data reference for this Effect
    m_paramMetaData = s_metaData;

    // Initialize Parameters for this Effect
    this->InitParams(s_paramCount);
}

// Destructor
CloudyReverbModule::~CloudyReverbModule()
{
    // No Code Needed
}

void CloudyReverbModule::Init(float sample_rate)
{
    BaseEffectModule::Init(sample_rate);

    cloudy_reverb.Init(reverb_buffer);
}

void CloudyReverbModule::ParameterChanged(int parameter_id)
{
    if (parameter_id == 0) {  
        cloudy_reverb.set_input_gain(GetParameterAsMagnitude(0));

    } else if (parameter_id == 3) { 
        cloudy_reverb.set_amount(m_verbMin + (m_verbMax - m_verbMin) * GetParameterAsMagnitude(3));

    } else if (parameter_id == 4) { 
        cloudy_reverb.set_diffusion(GetParameterAsMagnitude(4));

    } else if (parameter_id == 5) { 
        cloudy_reverb.set_lp(GetParameterAsMagnitude(5));
    }
}

void CloudyReverbModule::ProcessMono(float in)
{
    BaseEffectModule::ProcessMono(in);

    // Process Reverb
    float sendl, sendr;  // Reverb Inputs/Outputs
    sendl = m_audioLeft;
    sendr = m_audioLeft;
    cloudy_reverb.Process(&sendl, &sendr, 1);

    // Apply mix and level controls
    m_audioLeft = (sendl * GetParameterAsMagnitude(1) + m_audioLeft * (1.0 - GetParameterAsMagnitude(1))) * GetParameterAsMagnitude(2) * 1.5;
    m_audioRight = (sendr * GetParameterAsMagnitude(1) + m_audioLeft * (1.0 - GetParameterAsMagnitude(1))) * GetParameterAsMagnitude(2) * 1.5;

}

void CloudyReverbModule::ProcessStereo(float inL, float inR)
{    
    // Do the base stereo calculation (which resets the right signal to be the inputR instead of combined mono)
    BaseEffectModule::ProcessStereo(inL, inR);

    float sendl, sendr;  // Reverb Inputs/Outputs
    sendl = m_audioLeft;
    sendr = m_audioRight;
    cloudy_reverb.Process(&sendl, &sendr, 1);

    // Apply mix and level controls
    m_audioLeft = (sendl * GetParameterAsMagnitude(1) + m_audioLeft * (1.0 - GetParameterAsMagnitude(1))) * GetParameterAsMagnitude(2) * 1.5;
    m_audioRight = (sendr * GetParameterAsMagnitude(1) + m_audioRight * (1.0 - GetParameterAsMagnitude(1))) * GetParameterAsMagnitude(2) * 1.5;

}

float CloudyReverbModule::GetBrightnessForLED(int led_id)
{    
    float value = BaseEffectModule::GetBrightnessForLED(led_id);

    if (led_id == 1)
    {
        return value; // * GetParameterAsMagnitude(0);
    }

    return value;
}