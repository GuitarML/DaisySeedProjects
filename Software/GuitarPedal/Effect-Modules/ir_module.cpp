#include "ir_module.h"
#include "../Util/audio_utilities.h"
#include "ImpulseResponse/ImpulseResponse.h"
#include "ImpulseResponse/ir_data.h"

using namespace bkshepherd;

static const char* s_irNames[1] = {"test_ir"};

static const int s_paramCount = 5;
static const ParameterMetaData s_metaData[s_paramCount] = {{name: "Gain", valueType: ParameterValueType::FloatMagnitude, defaultValue: 74, knobMapping: 0, midiCCMapping: 1},
                                                           {name: "IR", valueType: ParameterValueType::Binned, valueBinCount: 1, valueBinNames: s_irNames, defaultValue: 0, knobMapping: 1, midiCCMapping: 2},
                                                           {name: "Level", valueType: ParameterValueType::FloatMagnitude, defaultValue: 74, knobMapping: 2, midiCCMapping: 3}
                                                           };

// Default Constructor
IRModule::IRModule() : BaseEffectModule(),
                                                        m_gainMin(1.0f),
                                                        m_gainMax(20.0f),
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

    //const auto irData = mIR->GetData();
    mIR = std::make_unique<dsp::ImpulseResponse>(ir_data, sample_rate);  // ir_data is from ir_data.h

}

void IRModule::ProcessMono(float in)
{
    BaseEffectModule::ProcessMono(in);

    //sample** irPointers = toneStackOutPointers;
    //if (mIR != nullptr && GetParam(kIRToggle)->Value())
    //    irPointers = mIR->Process(toneStackOutPointers, numChannelsInternal, numFrames);
    int numChannelsInternal = 1;
    int numFrames = 1;

    float** irPointers = mIR->Process(m_audioLeft, numChannelsInternal, numFrames);


    m_audioLeft = m_audioLeft * GetParameterAsMagnitude(2);
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

 /*
void ModulatedTremoloModule::SetTempo(uint32_t bpm)
{
    float freq = tempo_to_freq(bpm);

    if (freq <= m_tremoloFreqMin)
    {
        SetParameterRaw(2, 0);
    }
    else if (freq >= m_tremoloFreqMax)
    {
        SetParameterRaw(2, 127);
    }
    else 
    {
        // Get the parameter as close as we can to target tempo
        SetParameterRaw(2, ((freq - m_tremoloFreqMin) / (m_tremoloFreqMax - m_tremoloFreqMin)) * 128);
    }
}
*/
float IRModule::GetBrightnessForLED(int led_id)
{    
    float value = BaseEffectModule::GetBrightnessForLED(led_id);

    if (led_id == 1)
    {
        return value * m_cachedEffectMagnitudeValue;
    }

    return value;
}
