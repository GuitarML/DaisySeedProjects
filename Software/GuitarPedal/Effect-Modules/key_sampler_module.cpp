#include "key_sampler_module.h"
#include "../Util/audio_utilities.h"

using namespace bkshepherd;

#define MAX_SIZE (48000 * 4) // 4 seconds of floats at 48 khz
float DSY_SDRAM_BSS buf[25][MAX_SIZE];  // [12] is middle c (original sample), 100 total seconds of audio

// Midi keys from 48 to 72 (2 octave), 60 is middle c

static const int s_paramCount = 1;
static const ParameterMetaData s_metaData[s_paramCount] = {{name: "Record", valueType: ParameterValueType::Bool, valueBinCount: 0, defaultValue: 0, knobMapping: 0, midiCCMapping: 21}
                                                           };

// Default Constructor
KeySamplerModule::KeySamplerModule() : BaseEffectModule(),
                                                        //m_freqMin(250.0f),
                                                        //m_freqMax(8500.0f),
                                                        bufferCounter(0),
                                                        currentSampleLen(0),
                                                        previousRecordState(false),
                                                        m_cachedEffectMagnitudeValue(1.0f)
{
    // Set the name of the effect
    m_name = "KeySampler";

    // Setup the meta data reference for this Effect
    m_paramMetaData = s_metaData;
    
    // Initialize Parameters for this Effect
    this->InitParams(s_paramCount);
}

// Destructor
KeySamplerModule::~KeySamplerModule()
{
    // No Code Needed
}

void KeySamplerModule::Init(float sample_rate)
{
    BaseEffectModule::Init(sample_rate);

    ResetBuffer(); // Zero out all the buffers

    pitchshifter.Init(sample_rate);
    pitchshifter.SetDelSize(1500);  // almost the minimum delay length for PitchShifter class (30ms)

}

void KeySamplerModule::OnNoteOn(float notenumber, float velocity)
{
    // Note Off can come in as Note On w/ 0 Velocity
    if(velocity == 0.f)
    {
        OnNoteOff(notenumber, velocity);
    }
    else
    {
        int index = notenumber - 48;
        // Midi keys from 48 to 72 (2 octave), 60 is middle c
        keysPlaying[index] = true;

        keyAmplitude[index] = velocity / 128;
    }
}

void KeySamplerModule::OnNoteOff(float notenumber, float velocity)
{
    int index = notenumber - 48;
    // Midi keys from 48 to 72 (2 octave), 60 is middle c
    keysPlaying[index] = false;
}

void KeySamplerModule::ResetBuffer()
{
    for (int i = 0; i < 25; i++) {
        for (int j = 0; j < MAX_SIZE; j++) {
            buf[i][j] = 0.0;
        }
    }
}

void KeySamplerModule::ProcessNewSample()
{
    // Pitchshift all non middle c samples
    int totalPitchShiftSamples = psDelay + currentSampleLen;
    float zero[1] = {0.0};

    for (int i = 0; i < 25; i++) {
       if (i == 12) { // If middle c (base sample) then skip
           continue;
       } else {
           float transpose = i - 12;
           pitchshifter.SetTransposition(transpose);  // Transpose the recording in semitones based on distance from middle c
       }

       for (int j = 0; j < totalPitchShiftSamples; j++) {
           if (j < psDelay) { // Skip recording to the buffer until sound outputs from the pitchshifter
               pitchshifter.Process(buf[12][j]);
               continue; 
           } else if (j > currentSampleLen) {   // If we've reached the end of the original sample, process 0's to the pitchshifter
            
               buf[i][j - psDelay] = pitchshifter.Process(zero[0]);
           } else {                                           // Else process the original sample to the pitchshifter
               buf[i][j - psDelay] = pitchshifter.Process(buf[12][j]); 
           }
       }
    }
}

void KeySamplerModule::RecordSample(float input)
{
    buf[12][bufferCounter] = input;
}

float KeySamplerModule::SamplePlayBack()
{
    float output = 0.0;

    for (int i = 0; i < 25; i++) {
        if (keysPlaying[i] == true) {
            if (keySampleIndex[i] < currentSampleLen) {
                output += buf[i][keySampleIndex[i]] * keyAmplitude[i];
                keySampleIndex[i] += 1;
            
            } else {
                keysPlaying[i] = false;
                keySampleIndex[i] = 0;
            }
        }
    }

    return output;
}

void KeySamplerModule::ProcessMono(float in)
{
    BaseEffectModule::ProcessMono(in);

    if (GetParameterAsBool(0)) { 
        if (previousRecordState == false) {
            bufferCounter = 0;
            previousRecordState = true;
        } else {
            bufferCounter += 1;
            if (bufferCounter > MAX_SIZE - 1) {
                SetParameterAsBool(0, false);
                currentSampleLen = bufferCounter;
                previousRecordState = false;
                ProcessNewSample();
            }
        }
        RecordSample(m_audioLeft);
    } else {
        if (previousRecordState == true) {
            currentSampleLen = bufferCounter;
            ProcessNewSample();
        }
    }


    if (GetParameterAsBool(0)) {
        m_audioLeft = m_audioLeft;
    } else {
        //m_audioLeft = ProcessSamplePlayBack();
    }

    m_audioRight = m_audioLeft;

}

void KeySamplerModule::ProcessStereo(float inL, float inR)
{    
    // Calculate the mono effect
    ProcessMono(inL);

    // Do the base stereo calculation (which resets the right signal to be the inputR instead of combined mono)
    //BaseEffectModule::ProcessStereo(m_audioLeft, inR);

}



float KeySamplerModule::GetBrightnessForLED(int led_id)
{    
    float value = BaseEffectModule::GetBrightnessForLED(led_id);

    if (led_id == 1)
    {
        return value * m_cachedEffectMagnitudeValue;
    }

    return value;
}
