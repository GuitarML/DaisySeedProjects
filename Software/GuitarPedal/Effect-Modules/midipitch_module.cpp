#include "midipitch_module.h"
#include "../Util/audio_utilities.h"

using namespace bkshepherd;


static const int s_paramCount = 6;
static const ParameterMetaData s_metaData[s_paramCount] = {{name: "NA", valueType: ParameterValueType::FloatMagnitude, defaultValue: 74, knobMapping: 0, midiCCMapping: 22},
                                                           {name: "NA", valueType: ParameterValueType::FloatMagnitude, defaultValue: 74, knobMapping: 1, midiCCMapping: 24},
                                                           {name: "Level", valueType: ParameterValueType::FloatMagnitude, defaultValue: 74, knobMapping: 2, midiCCMapping: 23},
                                                           {name: "PitchFun", valueType: ParameterValueType::FloatMagnitude, defaultValue: 74, knobMapping: 3, midiCCMapping: 25},
                                                           {name: "VerbTime", valueType: ParameterValueType::FloatMagnitude, defaultValue: 74, knobMapping: 4, midiCCMapping: 26},
                                                           {name: "VerbDamp", valueType: ParameterValueType::FloatMagnitude, defaultValue: 74, knobMapping: 5, midiCCMapping: 27}
                                                           };

// Default Constructor
MidiPitchModule::MidiPitchModule() : BaseEffectModule(),
                                                        m_freqMin(300.0f),
                                                        m_freqMax(20000.0f),
                                                        m_verbMin(0.3f),
                                                        m_verbMax(1.0f),
                                                        m_cachedEffectMagnitudeValue(1.0f)
{
    // Set the name of the effect
    m_name = "MidiPitch";

    // Setup the meta data reference for this Effect
    m_paramMetaData = s_metaData;
    
    // Initialize Parameters for this Effect
    this->InitParams(s_paramCount);
}

// Destructor
MidiPitchModule::~MidiPitchModule()
{
    // No Code Needed
}

void MidiPitchModule::Init(float sample_rate)
{
    BaseEffectModule::Init(sample_rate);

    pitch.Init(sample_rate);
    //verb.Init(sample_rate);

    pitch.SetDelSize(1536); // 32 ms shift
}

void MidiPitchModule::ParameterChanged(int parameter_id)
{
    if (parameter_id == 0) {  
        //modalvoice.SetStructure(GetParameterAsMagnitude(0));

    } else if (parameter_id == 1) { 
        //modalvoice.SetBrightness(GetParameterAsMagnitude(1));

    } else if (parameter_id == 3) { 
        pitch.SetFun(GetParameterAsMagnitude(3));

    } else if (parameter_id == 4) { 
        //verb.SetFeedback(m_verbMin + (m_verbMax - m_verbMin) * (GetParameterAsMagnitude(4)));

    } else if (parameter_id == 5) { 
        //verb.SetLpFreq(m_freqMin + (m_freqMax - m_freqMin) * (1.0 - GetParameterAsMagnitude(5) * GetParameterAsMagnitude(5)));
    }
}

void MidiPitchModule::OnNoteOn(float notenumber, float velocity)
{
    // Note Off can come in as Note On w/ 0 Velocity
    if(velocity == 0.f)
    {
        
    }
    else
    {

        float trans = notenumber - 60;
        pitch.SetTransposition(trans);
    }
}

void MidiPitchModule::OnNoteOff(float notenumber, float velocity)
{
    // Currently note off does nothing
}

void MidiPitchModule::ProcessMono(float in)
{
    BaseEffectModule::ProcessMono(in); // This doesn't matter since not using input audio

    float pitch_out = pitch.Process(m_audioLeft);
    float wetl = 0.0;
    float wetr = 0.0;
    float combined = 0.0;
    //float combined = pitch_out + m_audioLeft;
    //verb.Process(combined, combined, &wetl, &wetr);


    m_audioLeft     = (combined + wetl) * GetParameterAsMagnitude(2) * 1.5;  // Doing 50/50 mix of dry/reverb, 0.2 is volume reduction
    m_audioRight    = (combined + wetr) * GetParameterAsMagnitude(2) * 1.5;

}

void MidiPitchModule::ProcessStereo(float inL, float inR)
{    
    // Calculate the mono effect
    ProcessMono(inL);

    // Do the base stereo calculation (which resets the right signal to be the inputR instead of combined mono)
    //BaseEffectModule::ProcessStereo(m_audioLeft, inR);

}



float MidiPitchModule::GetBrightnessForLED(int led_id)
{    
    float value = BaseEffectModule::GetBrightnessForLED(led_id);

    if (led_id == 1)
    {
        return value * m_cachedEffectMagnitudeValue;
    }

    return value;
}
