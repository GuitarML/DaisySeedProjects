#include "string_keys_module.h"
#include "../Util/audio_utilities.h"

using namespace bkshepherd;


static const int s_paramCount = 6;
static const ParameterMetaData s_metaData[s_paramCount] = {{name: "Structure", valueType: ParameterValueType::FloatMagnitude, defaultValue: 74, knobMapping: 0, midiCCMapping: 22},
                                                           {name: "Brightness", valueType: ParameterValueType::FloatMagnitude, defaultValue: 74, knobMapping: 1, midiCCMapping: 24},
                                                           {name: "Level", valueType: ParameterValueType::FloatMagnitude, defaultValue: 74, knobMapping: 2, midiCCMapping: 23},
                                                           {name: "Damping", valueType: ParameterValueType::FloatMagnitude, defaultValue: 74, knobMapping: 3, midiCCMapping: 25},
                                                           {name: "VerbTime", valueType: ParameterValueType::FloatMagnitude, defaultValue: 74, knobMapping: 4, midiCCMapping: 26},
                                                           {name: "VerbDamp", valueType: ParameterValueType::FloatMagnitude, defaultValue: 74, knobMapping: 5, midiCCMapping: 27}
                                                           };

// Default Constructor
StringKeysModule::StringKeysModule() : BaseEffectModule(),
                                                        m_freqMin(300.0f),
                                                        m_freqMax(20000.0f),
                                                        m_verbMin(0.6f),
                                                        m_verbMax(1.0f),
                                                        m_cachedEffectMagnitudeValue(1.0f)
{
    // Set the name of the effect
    m_name = "StringKeys";

    // Setup the meta data reference for this Effect
    m_paramMetaData = s_metaData;
    
    // Initialize Parameters for this Effect
    this->InitParams(s_paramCount);
}

// Destructor
StringKeysModule::~StringKeysModule()
{
    // No Code Needed
}

void StringKeysModule::Init(float sample_rate)
{
    BaseEffectModule::Init(sample_rate);

    modalvoice.Init(sample_rate);
    verb.Init(sample_rate);
}

void StringKeysModule::ParameterChanged(int parameter_id)
{
    if (parameter_id == 0) {  
        modalvoice.SetStructure(GetParameterAsMagnitude(0));

    } else if (parameter_id == 1) { 
        modalvoice.SetBrightness(GetParameterAsMagnitude(1));

    } else if (parameter_id == 3) { 
        modalvoice.SetDamping(GetParameterAsMagnitude(3));

    } else if (parameter_id == 4) { 
        verb.SetFeedback(m_verbMin + (m_verbMax - m_verbMin) * (GetParameterAsMagnitude(4)));

    } else if (parameter_id == 5) { 
        verb.SetLpFreq(m_freqMin + (m_freqMax - m_freqMin) * (1.0 - GetParameterAsMagnitude(5) * GetParameterAsMagnitude(5)));
    }
}

void StringKeysModule::OnNoteOn(float notenumber, float velocity)
{
    // Note Off can come in as Note On w/ 0 Velocity
    if(velocity == 0.f)
    {
        
    }
    else
    {
        // Using velocity for accent setting (striking the resonator harder)
        modalvoice.SetAccent(velocity/128.0);
        modalvoice.SetFreq(mtof(notenumber));
        modalvoice.Trig();
    }
}

void StringKeysModule::OnNoteOff(float notenumber, float velocity)
{
    // Currently note off does nothing
}

void StringKeysModule::ProcessMono(float in)
{
    BaseEffectModule::ProcessMono(in); // This doesn't matter since not using input audio

    float voice_out = modalvoice.Process();
    float wetl, wetr;
    verb.Process(voice_out, voice_out, &wetl, &wetr);

    m_audioLeft     = (voice_out + wetl) * GetParameterAsMagnitude(2);  // Doing 50/50 mix of dry/reverb
    m_audioRight    = (voice_out + wetr) * GetParameterAsMagnitude(2);

}

void StringKeysModule::ProcessStereo(float inL, float inR)
{    
    // Calculate the mono effect
    ProcessMono(inL);

    // Do the base stereo calculation (which resets the right signal to be the inputR instead of combined mono)
    //BaseEffectModule::ProcessStereo(m_audioLeft, inR);

}



float StringKeysModule::GetBrightnessForLED(int led_id)
{    
    float value = BaseEffectModule::GetBrightnessForLED(led_id);

    if (led_id == 1)
    {
        return value * m_cachedEffectMagnitudeValue;
    }

    return value;
}
