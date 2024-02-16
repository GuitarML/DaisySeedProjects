#include "midipitch_module.h"
#include "../Util/audio_utilities.h"

using namespace bkshepherd;

PitchShifter voices[3];  // Didn't work until I moved this outside the module class

static const char* s_numVoices[3] = {"One", "Two", "Three"};
static const char* s_voiceType[2] = {"DryOff", "DryOn"};

static const int s_paramCount = 6;
static const ParameterMetaData s_metaData[s_paramCount] = {{name: "NumVoices", valueType: ParameterValueType::Binned, valueBinCount: 3, valueBinNames: s_numVoices, defaultValue: 0, knobMapping: 0, midiCCMapping: 21},
                                                           {name: "VoiceType", valueType: ParameterValueType::Binned, valueBinCount: 2, valueBinNames: s_voiceType, defaultValue: 0, knobMapping: 1, midiCCMapping: 22},
                                                           {name: "Level", valueType: ParameterValueType::FloatMagnitude, defaultValue: 74, knobMapping: 2, midiCCMapping: 23},
                                                           {name: "PitchFun", valueType: ParameterValueType::FloatMagnitude, defaultValue: 74, knobMapping: 3, midiCCMapping: 25},
                                                           {name: "Spread", valueType: ParameterValueType::FloatMagnitude, defaultValue: 74, knobMapping: 4, midiCCMapping: 26},
                                                           {name: "LP Filter", valueType: ParameterValueType::FloatMagnitude, defaultValue: 74, knobMapping: 5, midiCCMapping: 27}
                                                           };

// Default Constructor
MidiPitchModule::MidiPitchModule() : BaseEffectModule(),
                                                        m_freqMin(500.0f),
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
    for (int i = 0; i < maxVoices; i++ ){
        voices[i].Init(sample_rate);
    }
    //verb.Init(sample_rate);
    for (int i = 0; i < maxVoices; i++ ){
        env_[i].Init(sample_rate);
        env_[i].SetSustainLevel(0.5f);
        env_[i].SetTime(ADSR_SEG_ATTACK, 0.005f);
        env_[i].SetTime(ADSR_SEG_DECAY, 0.005f);
        //env_[i].SetTime(ADSR_SEG_ATTACK, 0.02f);
        //env_[i].SetTime(ADSR_SEG_DECAY, 0.02f);
        env_[i].SetTime(ADSR_SEG_RELEASE, 0.2f);
    }
    tone.Init(sample_rate);
}

void MidiPitchModule::ParameterChanged(int parameter_id)
{
    if (parameter_id == 3) { 
        for (int i = 0; i < maxVoices; i++) {
            voices[i].SetFun(GetParameterAsMagnitude(3));
        }

    } else if (parameter_id == 4) {
        int spread_int1 = GetParameterAsMagnitude(4) * 4600;
        int spread_int2 = GetParameterAsMagnitude(4) * 9200;
        int spread_int3 = GetParameterAsMagnitude(4) * 13800;
        voices[0].SetDelSize(1600 + spread_int1);
        voices[1].SetDelSize(1600 + spread_int2);
        voices[2].SetDelSize(1600 + spread_int3); 

    } else if (parameter_id == 0 ) {
        numVoices = GetParameterAsBinnedValue(0); // 1 to 3  // TODO If you change this in the middle of playing keys will it mess up
    } else if (parameter_id == 5) {
        tone.SetFreq(m_freqMin + GetParameterAsMagnitude(5) * GetParameterAsMagnitude(5) * (m_freqMax - m_freqMin)); 
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


        int grabIndex = numVoices + 1;
        float trans = notenumber - 60;  // Set no tranposition to middle c
        for (int i = 0; i < numVoices; i++) {
            if (voiceKeys[i] == 0.0) {
                voiceKeys[i] = trans;
                grabIndex = i;

                break;
            } 
        }
        if (grabIndex == numVoices + 1) {  // If no available voices, take one anyway
            grabIndex = 0;
            voiceKeys[0] = trans;  // Just grabbing the first voice index, TODO better way to do this by assigning the least recent voice
        }
        env_gate_[grabIndex] = true;

        voices[grabIndex].SetTransposition(voiceKeys[grabIndex]);

    }
}

void MidiPitchModule::OnNoteOff(float notenumber, float velocity)
{
    float trans = notenumber - 60;  // Set no tranposition to middle c
    for (int i = 0; i < maxVoices; i++) { // Looping to maxVoices here in case numVoices is changed mid key
        if (voiceKeys[i] == trans) {
            voiceKeys[i] = 0.0;   // Reset pitchshift to 0.0 if the particular key is let go, freeing it up for the next key
            //voices[i].SetTransposition(0.0);
            env_gate_[i] = false;
            break;
        } 
    }
    
}

void MidiPitchModule::ProcessMono(float in)
{
    BaseEffectModule::ProcessMono(in); // This doesn't matter since not using input audio
    float sum = 0.0;
    for (int i=0; i < numVoices; i++) {
        float temp = voices[i].Process(m_audioLeft); // Apply level adjustment here too
        float amp = env_[i].Process(env_gate_[i]);
        if(!env_[i].IsRunning())
            voices[i].SetTransposition(voiceKeys[i]);
        sum += temp * amp;
    }

    float combined = 0.0;
    if (GetParameterAsBinnedValue(1) - 1 == 0) {
        combined = sum;
    } else { 
        combined = sum + m_audioLeft;
    }
    // Apply LP Filter
    float filter_out = tone.Process(combined);  // Apply tone Low Pass filter

    m_audioLeft     = filter_out * GetParameterAsMagnitude(2); 
    m_audioRight    = filter_out * GetParameterAsMagnitude(2);

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
