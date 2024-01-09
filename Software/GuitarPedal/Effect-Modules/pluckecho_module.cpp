#include "pluckecho_module.h"
#include "../Util/audio_utilities.h"

using namespace bkshepherd;

DelayLine<float, MAX_DELAY> DSY_SDRAM_BSS delay;

static const int s_paramCount = 4;
static const ParameterMetaData s_metaData[s_paramCount] = {{name: "StringDecay", valueType: ParameterValueType::FloatMagnitude, defaultValue: 74, knobMapping: 0, midiCCMapping: 22},
                                                           {name: "DelayTime", valueType: ParameterValueType::FloatMagnitude, defaultValue: 74, knobMapping: 1, midiCCMapping: 24},
                                                           {name: "DelayFdbk", valueType: ParameterValueType::FloatMagnitude, defaultValue: 74, knobMapping: 2, midiCCMapping: 23},
                                                           {name: "Level", valueType: ParameterValueType::FloatMagnitude, defaultValue: 74, knobMapping: 3, midiCCMapping: 25}
                                                           };

// Default Constructor
PluckEchoModule::PluckEchoModule() : BaseEffectModule(),
                                                        m_freqMin(300.0f),
                                                        m_freqMax(20000.0f),
                                                        m_verbMin(0.3f),
                                                        m_verbMax(1.0f),
                                                        nn(0.0f),
                                                        trig(1.0),
                                                        m_cachedEffectMagnitudeValue(1.0f)
{
    // Set the name of the effect
    m_name = "PluckEcho";

    // Setup the meta data reference for this Effect
    m_paramMetaData = s_metaData;
    
    // Initialize Parameters for this Effect
    this->InitParams(s_paramCount);
}

// Destructor
PluckEchoModule::~PluckEchoModule()
{
    // No Code Needed
}

void PluckEchoModule::Init(float sample_rate)
{
    BaseEffectModule::Init(sample_rate);

    synth.Init(sample_rate);

    delay.Init();
    delay.SetDelay(sample_rate * 0.8f); // half second delay

    verb.Init(sample_rate);
    verb.SetFeedback(0.85f);
    verb.SetLpFreq(2000.0f);
}

void PluckEchoModule::ParameterChanged(int parameter_id)
{
    if (parameter_id == 0) {  
        float decay = 0.5f + GetParameterAsMagnitude(0) * 0.5f;
        synth.SetDecay(decay);

    }
}

void PluckEchoModule::OnNoteOn(float notenumber, float velocity)
{
    // Note Off can come in as Note On w/ 0 Velocity
    if(velocity == 0.f)
    {
        
    }
    else
    {
        nn = notenumber;
        trig = 1.0;
    }
}

void PluckEchoModule::OnNoteOff(float notenumber, float velocity)
{
    // Currently note off does nothing
}

void PluckEchoModule::ProcessMono(float in)
{
    BaseEffectModule::ProcessMono(in); // This doesn't matter since not using input audio


    float kval    = GetParameterAsMagnitude(1);
    float deltime = (0.001f + (kval * kval) * 5.0f) * 48000; // samplerate=48000
    
    float delfb   = GetParameterAsMagnitude(2);

    // Smooth delaytime, and set.
    fonepole(smooth_time, deltime, 0.0005f);
    delay.SetDelay(smooth_time);

    // Synthesize Plucks
    float sig = synth.Process(trig, nn);
    trig = 0.0;

    //		// Handle Delay
    float  delsig = delay.Read();
    delay.Write(sig + (delsig * delfb));

    // Create Reverb Send
    float dry  = sig + delsig;
    float send = dry * 0.6f;
    float wetl, wetr;
    verb.Process(send, send, &wetl, &wetr);

    // Output
    m_audioLeft  = (dry + wetl) * GetParameterAsMagnitude(3) * 1.5;
    m_audioRight = (dry + wetr) * GetParameterAsMagnitude(3) * 1.5;



    //m_audioLeft     = (voice_out + wetl) * GetParameterAsMagnitude(2) * 0.1;  // Doing 50/50 mix of dry/reverb, 0.2 is volume reduction
    //m_audioRight    = (voice_out + wetr) * GetParameterAsMagnitude(2) * 0.1;

}

void PluckEchoModule::ProcessStereo(float inL, float inR)
{    
    // Calculate the mono effect
    ProcessMono(inL);

    // Do the base stereo calculation (which resets the right signal to be the inputR instead of combined mono)
    //BaseEffectModule::ProcessStereo(m_audioLeft, inR);

}



float PluckEchoModule::GetBrightnessForLED(int led_id)
{    
    float value = BaseEffectModule::GetBrightnessForLED(led_id);

    if (led_id == 1)
    {
        return value * m_cachedEffectMagnitudeValue;
    }

    return value;
}
