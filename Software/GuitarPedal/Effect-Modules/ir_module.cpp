#include "ir_module.h"
#include "../Util/audio_utilities.h"
#include "ImpulseResponse/ir_data.h"

using namespace bkshepherd;

static const char* s_irNames[3] = {"RhythmIR", "LeadIR", "CleanIR"};

static const int s_paramCount = 2;
static const ParameterMetaData s_metaData[s_paramCount] = {{name: "IR", valueType: ParameterValueType::Binned, valueBinCount: 3, valueBinNames: s_irNames, defaultValue: 0, knobMapping: 0, midiCCMapping: 2},
                                                           {name: "Level", valueType: ParameterValueType::FloatMagnitude, defaultValue: 74, knobMapping: 1, midiCCMapping: 3}
                                                           };

// Default Constructor
IRModule::IRModule() : BaseEffectModule(),
                                                        m_gainMin(0.0f),
                                                        m_gainMax(0.75f),  // Lowered gain for high ir output
                                                        m_cachedEffectMagnitudeValue(1.0f)
{
    // Set the name of the effect
    m_name = "IR";

    // Setup the meta data reference for this Effect
    m_paramMetaData = s_metaData;
    
    // Initialize Parameters for this Effect
    this->InitParams(s_paramCount);
}

// Destructor
IRModule::~IRModule()
{
    // No Code Needed
}

void IRModule::Init(float sample_rate)
{
    BaseEffectModule::Init(sample_rate);

    SelectIR();

}

void IRModule::ParameterChanged(int parameter_id)
{
    if (parameter_id == 0) {  // Change Model
        SelectIR();
    } 
}
void IRModule::SelectIR()
{
    int irIndex = GetParameterAsBinnedValue(0) - 1;
    if (irIndex != m_currentIRindex) {
        mIR.Init(ir_collection[irIndex]);  // ir_data is from ir_data.h
    }
    m_currentIRindex = irIndex;
}

void IRModule::ProcessMono(float in)
{
    BaseEffectModule::ProcessMono(in);

    m_audioLeft = mIR.Process(m_audioLeft);

    m_audioLeft = m_audioLeft * (m_gainMin + (m_gainMax - m_gainMin) * GetParameterAsMagnitude(1));
    m_audioRight = m_audioLeft;
}

void IRModule::ProcessStereo(float inL, float inR)
{    
    // Calculate the mono effect
    ProcessMono(inL);

    // Do the base stereo calculation (which resets the right signal to be the inputR instead of combined mono)
    //BaseEffectModule::ProcessStereo(m_audioLeft, inR);

    // Use the same magnitude as already calculated for the Left Audio
    //m_audioRight = m_audioRight * m_cachedEffectMagnitudeValue;
}


float IRModule::GetBrightnessForLED(int led_id)
{    
    float value = BaseEffectModule::GetBrightnessForLED(led_id);

    if (led_id == 1)
    {
        return value * m_cachedEffectMagnitudeValue;
    }

    return value;
}
