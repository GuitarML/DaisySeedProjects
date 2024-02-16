#include "delay_module.h"
#include "../Util/audio_utilities.h"

using namespace bkshepherd;

static const char* s_waveBinNames[5] = {"Sine", "Triangle", "Saw", "Ramp", "Square"}; //, "Poly Tri", "Poly Saw", "Poly Sqr"};  // Horrible loud sound when switching to poly tri, not every time, TODO whats going on?
static const char* s_modParamNames[4] = {"None", "DelayTime", "DelayLevel", "DelayPan"};
static const char* s_delayModes[3] = {"Normal", "Triplett", "Dotted 8th"};
static const char* s_delayTypes[6] = {"Forward", "Reverse", "Octave", "ReverseOct", "Dual", "DualOct"};

DelayLineRevOct<float, MAX_DELAY> DSY_SDRAM_BSS delayLineLeft;
DelayLineRevOct<float, MAX_DELAY> DSY_SDRAM_BSS delayLineRight;
DelayLineReverse<float, MAX_DELAY_REV> DSY_SDRAM_BSS delayLineRevLeft;
DelayLineReverse<float, MAX_DELAY_REV> DSY_SDRAM_BSS delayLineRevRight;
DelayLine<float, MAX_DELAY_SPREAD> DSY_SDRAM_BSS delayLineSpread;

static const int s_paramCount = 12;   // TODO: TEST STARTING WITH THE EXTREMES OF ALL PARAMETERS (high and low, this is where errors tend to occur)
static const ParameterMetaData s_metaData[s_paramCount] = {{name: "Delay Time", valueType: ParameterValueType::FloatMagnitude, valueBinCount: 0, defaultValue: 57, knobMapping: 0, midiCCMapping: 1},  //mod
                                                           {name: "D Feedback", valueType: ParameterValueType::FloatMagnitude, valueBinCount: 0, defaultValue: 57, knobMapping: 1, midiCCMapping: 22},
                                                           {name: "Delay Mix", valueType: ParameterValueType::FloatMagnitude, valueBinCount: 0, defaultValue: 57, knobMapping: 2, midiCCMapping: 23},
                                                           //{name: " Time", valueType: ParameterValueType::FloatMagnitude, valueBinCount: 0, defaultValue: 57, knobMapping: 3, midiCCMapping: 24},
                                                           //{name: " Damp", valueType: ParameterValueType::FloatMagnitude, valueBinCount: 0, defaultValue: 40, knobMapping: 4, midiCCMapping: 25},  //mod
                                                           //{name: " Mix", valueType: ParameterValueType::FloatMagnitude, valueBinCount: 0, defaultValue: 57, knobMapping: 5, midiCCMapping: 26},
                                                           {name: "Delay Mode", valueType: ParameterValueType::Binned, valueBinCount: 3, valueBinNames: s_delayModes, defaultValue: 0, knobMapping: 3, midiCCMapping: 24},
                                                           {name: "Delay Type", valueType: ParameterValueType::Binned, valueBinCount: 6, valueBinNames: s_delayTypes, defaultValue: 0, knobMapping: 4, midiCCMapping: 25},
                                                           //{name: "Series D>R", valueType: ParameterValueType::Bool, valueBinCount: 0, defaultValue: 0, knobMapping: -1, midiCCMapping: 28},
                                                           //{name: "Reverse", valueType: ParameterValueType::Bool, valueBinCount: 0, defaultValue: 0, knobMapping: -1, midiCCMapping: 29},
                                                           //{name: "Octave", valueType: ParameterValueType::Bool, valueBinCount: 0, defaultValue: 0, knobMapping: -1, midiCCMapping: 30},
                                                           {name: "Delay LPF", valueType: ParameterValueType::FloatMagnitude, valueBinCount: 0, defaultValue: 120, knobMapping: 5, midiCCMapping: 26},  //mod
                                                  /*11*/   {name: "D Spread", valueType: ParameterValueType::FloatMagnitude, valueBinCount: 0, defaultValue: 30, knobMapping: -1, midiCCMapping: 27}, 
                                                           //{name: "Ping Pong", valueType: ParameterValueType::Bool, valueBinCount: 0, defaultValue: 0, knobMapping: -1, midiCCMapping: 33},
                                                  /*12*/   //{name: "Dual Delay", valueType: ParameterValueType::Bool, valueBinCount: 0, defaultValue: 0, knobMapping: -1, midiCCMapping: 33},
                                                           {name: "Mod Amt", valueType: ParameterValueType::FloatMagnitude, valueBinCount: 0, defaultValue: 20, knobMapping: -1, midiCCMapping: 28}, 
                                                           {name: "Mod Rate", valueType: ParameterValueType::FloatMagnitude, valueBinCount: 0, defaultValue: 30, knobMapping: -1, midiCCMapping: 29}, 
                                                           {name: "Mod Param", valueType: ParameterValueType::Binned, valueBinCount: 4, valueBinNames: s_modParamNames, defaultValue: 0, knobMapping: -1, midiCCMapping: 30},
                                                           {name: "Mod Wave", valueType: ParameterValueType::Binned, valueBinCount: 5, valueBinNames: s_waveBinNames, defaultValue: 0, knobMapping: -1, midiCCMapping: 31},
                                                           {name: "Sync Mod F", valueType: ParameterValueType::Bool, valueBinCount: 0, defaultValue: 0, knobMapping: -1, midiCCMapping: 32}
                                                           };

                                                           

// Default Constructor
DelayModule::DelayModule() : BaseEffectModule(),
                                        m_delaylpFreqMin(300.0f),
                                        m_delaylpFreqMax(20000.0f),
                                        m_delaySamplesMin(2400.0f),
                                        m_delaySamplesMax(192000.0f),
                                        m_delaySpreadMin(24.0f),
                                        m_delaySpreadMax(2400.0f),
                                        m_pdelRight_out(0.0),
                                        m_modOscFreqMin(0.0),
                                        m_modOscFreqMax(3.0),
                                        m_currentMod(1.0),
                                        m_LEDValue(1.0f)
{
    // Set the name of the effect
    m_name = "Delay";

    // Setup the meta data reference for this Effect
    m_paramMetaData = s_metaData;

    // Initialize Parameters for this Effect
    this->InitParams(s_paramCount);

}

// Destructor
DelayModule::~DelayModule()
{
    // No Code Needed
}

void DelayModule::UpdateLEDRate()
{
    // Update the LED oscillator frequency based on the current timeParam
    float timeParam = GetParameterAsMagnitude(0);
    float delaySamples = m_delaySamplesMin + (m_delaySamplesMax - m_delaySamplesMin) * timeParam;
    float delayFreq =  effect_samplerate / delaySamples;
    led_osc.SetFreq(delayFreq / 2.0);   
}

void DelayModule::CalculateDelayMix()
{
    // Handle Normal or Alternate Mode Mix Controls
    //    A computationally cheap mostly energy constant crossfade from SignalSmith Blog
    //    https://signalsmith-audio.co.uk/writing/2021/cheap-energy-crossfade/

    float delMixKnob = GetParameterAsMagnitude(2);
    float x2 = 1.0 - delMixKnob;
    float A = delMixKnob*x2;
    float B = A * (1.0 + 1.4186 * A);
    float C = B + delMixKnob;
    float D = B + x2;

    delayWetMix = C * C;
    delayDryMix = D * D;

}


void DelayModule::Init(float sample_rate)
{
    BaseEffectModule::Init(sample_rate);

    delayLineLeft.Init();
    delayLineRevLeft.Init();
    delayLeft.del = &delayLineLeft;
    delayLeft.delreverse = &delayLineRevLeft;
    delayLeft.delayTarget = 24000; // in samples
    delayLeft.feedback = 0.0;
    delayLeft.active = true;     // Default to no delay
    delayLeft.toneOctLP.Init(sample_rate);
    delayLeft.toneOctLP.SetFreq(20000.0);

    delayLineRight.Init();
    delayLineRevRight.Init();
    delayRight.del = &delayLineRight;
    delayRight.delreverse = &delayLineRevRight;
    delayRight.delayTarget = 24000; // in samples
    delayRight.feedback = 0.0;
    delayRight.active = true;     // Default to no 
    delayRight.toneOctLP.Init(sample_rate);
    delayRight.toneOctLP.SetFreq(20000.0);

    delayLineSpread.Init();
    delaySpread.del = &delayLineSpread;
    delaySpread.delayTarget = 1500; // in samples
    delaySpread.active = true; 

    effect_samplerate = sample_rate;

    led_osc.Init(sample_rate);
    led_osc.SetWaveform(1);
    led_osc.SetFreq(2.0);

    modOsc.Init(sample_rate);
    modOsc.SetAmp(1.0);

    CalculateDelayMix(); 
}

void DelayModule::ParameterChanged(int parameter_id)
{
    if (parameter_id == 0) {  // Delay Time
        UpdateLEDRate();
    } else if (parameter_id == 2) {  // Delay Mix
        CalculateDelayMix();
    } else if (parameter_id == 3) {  // Delay Mode
        int delay_mode_temp = (GetParameterAsBinnedValue(3) - 1);
        if (delay_mode_temp > 0) {
            delayLeft.secondTapOn = true; // triplett, dotted 8th
            delayRight.secondTapOn = true; // triplett, dotted 8th
            if (delay_mode_temp == 1) {
                delayLeft.del->set2ndTapFraction(0.6666667); // triplett
                delayRight.del->set2ndTapFraction(0.6666667); // triplett
            } else if (delay_mode_temp == 2) {
                delayLeft.del->set2ndTapFraction(0.75); // dotted eighth
                delayRight.del->set2ndTapFraction(0.75); // dotted eighth
            } 
        } else {
            delayLeft.secondTapOn = false;
            delayRight.secondTapOn = false;   
        }
    } else if (parameter_id == 5) {
        delayLeft.toneOctLP.SetFreq(m_delaylpFreqMin + (m_delaylpFreqMax - m_delaylpFreqMin) * GetParameterAsMagnitude(5));
        delayRight.toneOctLP.SetFreq(m_delaylpFreqMin + (m_delaylpFreqMax - m_delaylpFreqMin) * GetParameterAsMagnitude(5));
    }
}

void DelayModule::ProcessModulation()
{
    int modParam = (GetParameterAsBinnedValue(9) - 1);
    // Calculate Modulation
    modOsc.SetWaveform(GetParameterAsBinnedValue(10) - 1);
   
    if (GetParameterAsBool(11)) {  // If mod frequency synced to delay time, override mod rate setting
        float dividor;
        if (modParam == 2 || modParam == 3) {
            dividor = 2.0; 
        } else {
            dividor = 4.0; 
        }
        float freq = (effect_samplerate / delayLeft.delayTarget) / dividor;
        modOsc.SetFreq(freq);
    } else {
        modOsc.SetFreq(m_modOscFreqMin + (m_modOscFreqMax - m_modOscFreqMin) * GetParameterAsMagnitude(8));
    }
    
    // Ease the effect value into it's target to avoid clipping with square or sawtooth waves
    fonepole(m_currentMod, modOsc.Process(), .01f);
    float mod = m_currentMod;
    float mod_amount = GetParameterAsMagnitude(7);

     // {"None", "DelayTime", "DelayLevel", "Level", "DelayPan"};
    if (modParam == 1) {         
        float timeParam = GetParameterAsMagnitude(0);
        delayLeft.delayTarget = m_delaySamplesMin + (m_delaySamplesMax - m_delaySamplesMin) * timeParam + mod * mod_amount * 500;
        delayRight.delayTarget = m_delaySamplesMin + (m_delaySamplesMax - m_delaySamplesMin) * timeParam + mod * mod_amount * 500;


    } else if (modParam == 2) {
        float mod_level = mod * mod_amount + (1.0 - mod_amount); 
        delayLeft.level = mod_level; 
        delayRight.level = mod_level; 
        delayLeft.level_reverse = mod_level; 
        delayRight.level_reverse = mod_level; 

    } else if (modParam == 3) {
        _level = mod * mod_amount + (1.0 - mod_amount);

    } else if (modParam == 4) {
        float mod_level = mod * mod_amount + (1.0 - mod_amount); 
        delayLeft.level = mod_level; 
        delayRight.level = 1.0 - mod_level; 
        delayLeft.level_reverse = mod_level; 
        delayRight.level_reverse = 1.0 - mod_level; 
    }

}

void DelayModule::ProcessMono(float in)
{
    BaseEffectModule::ProcessMono(in);

    // Calculate the effect
    int delayType = GetParameterAsBinnedValue(4) - 1;

    float timeParam = GetParameterAsMagnitude(0);

    delayLeft.delayTarget = m_delaySamplesMin + (m_delaySamplesMax - m_delaySamplesMin) * timeParam;
    delayRight.delayTarget = m_delaySamplesMin + (m_delaySamplesMax - m_delaySamplesMin) * timeParam;

    delayLeft.feedback = GetParameterAsMagnitude(1);
    delayRight.feedback = GetParameterAsMagnitude(1);
    if (delayType == 1 || delayType == 3) {
        delayLeft.reverseMode = true;
        delayRight.reverseMode = true;
    } else {
        delayLeft.reverseMode = false;
        delayRight.reverseMode = false;
    }
    if (delayType == 2 || delayType == 3 || delayType == 5) {
        delayLeft.del->setOctave(true); 
        delayRight.del->setOctave(true);
    } else {
        delayLeft.del->setOctave(false); 
        delayRight.del->setOctave(false);
    }
    if (delayType == 4 || delayType == 5 ) {
        delayLeft.dual_delay = true; 
        delayRight.dual_delay = true;
    } else {
        delayLeft.dual_delay = false; 
        delayRight.dual_delay = false;
    }

    if (delayType == 4 || delayType == 5 ) {       // If dual delay is turned on, spread controls the L/R panning of the two delays
        delayLeft.level = GetParameterAsMagnitude(6) + 1.0;
        delayRight.level = 1.0 - GetParameterAsMagnitude(6);

        delayLeft.level_reverse = 1.0 - GetParameterAsMagnitude(6); 
        delayRight.level_reverse = GetParameterAsMagnitude(6) + 1.0;

    } else {  // If dual delay is off reset the levels to normal, spread controls the amount of additional delay applied to the right channel
        delayLeft.level = 1.0; 
        delayRight.level = 1.0;
        delayLeft.level_reverse = 1.0; 
        delayRight.level_reverse = 1.0;
    }


    // Modulation, this overwrites any previous parameter settings for the modulated param - TODO Better way to do this for less processing?
    ProcessModulation();

    float delLeft_out = delayLeft.Process(m_audioLeft);
    float delRight_out = delayRight.Process(m_audioRight);
    //float delRight_out = delLeft_out;

    // Calculate any delay spread 
    delaySpread.delayTarget = m_delaySpreadMin + (m_delaySpreadMax - m_delaySpreadMin) * GetParameterAsMagnitude(6);
    float delSpread_out = delaySpread.Process(delRight_out);  
    if (GetParameterRaw(6) > 0 && delayType != 4 && delayType != 5) {
        delRight_out = delSpread_out; 
    }

    m_audioLeft = delLeft_out * delayWetMix + m_audioLeft * delayDryMix;
    m_audioRight  = delRight_out * delayWetMix + m_audioRight * delayDryMix;


}

void DelayModule::ProcessStereo(float inL, float inR)
{
    // Calculate the mono effect
    //ProcessMono(inL); 

    // Do the base stereo calculation (which resets the right signal to be the inputR instead of combined mono)
    BaseEffectModule::ProcessStereo(inL, inR);
    // Calculate the effect
    int delayType = GetParameterAsBinnedValue(4) - 1;

    float timeParam = GetParameterAsMagnitude(0);

    delayLeft.delayTarget = m_delaySamplesMin + (m_delaySamplesMax - m_delaySamplesMin) * timeParam;
    delayRight.delayTarget = m_delaySamplesMin + (m_delaySamplesMax - m_delaySamplesMin) * timeParam;

    delayLeft.feedback = GetParameterAsMagnitude(1);
    delayRight.feedback = GetParameterAsMagnitude(1);
    if (delayType == 1 || delayType == 3) {
        delayLeft.reverseMode = true;
        delayRight.reverseMode = true;
    } else {
        delayLeft.reverseMode = false;
        delayRight.reverseMode = false;
    }
    if (delayType == 2 || delayType == 3 || delayType == 5) {
        delayLeft.del->setOctave(true); 
        delayRight.del->setOctave(true);
    } else {
        delayLeft.del->setOctave(false); 
        delayRight.del->setOctave(false);
    }
    if (delayType == 4 || delayType == 5 ) {
        delayLeft.dual_delay = true; 
        delayRight.dual_delay = true;
    } else {
        delayLeft.dual_delay = false; 
        delayRight.dual_delay = false;
    }

    if (delayType == 4 || delayType == 5 ) {       // If dual delay is turned on, spread controls the L/R panning of the two delays
        delayLeft.level = GetParameterAsMagnitude(6) + 1.0;
        delayRight.level = 1.0 - GetParameterAsMagnitude(6);

        delayLeft.level_reverse = 1.0 - GetParameterAsMagnitude(6); 
        delayRight.level_reverse = GetParameterAsMagnitude(6) + 1.0;

    } else {  // If dual delay is off reset the levels to normal, spread controls the amount of additional delay applied to the right channel
        delayLeft.level = 1.0; 
        delayRight.level = 1.0;
        delayLeft.level_reverse = 1.0; 
        delayRight.level_reverse = 1.0;
    }


    // Modulation, this overwrites any previous parameter settings for the modulated param - TODO Better way to do this for less processing?
    ProcessModulation();

    float delLeft_out = delayLeft.Process(m_audioLeft);
    float delRight_out = delayRight.Process(m_audioRight);
    //float delRight_out = delLeft_out;

    // Calculate any delay spread 
    delaySpread.delayTarget = m_delaySpreadMin + (m_delaySpreadMax - m_delaySpreadMin) * GetParameterAsMagnitude(6);
    float delSpread_out = delaySpread.Process(delRight_out);  
    if (GetParameterRaw(6) > 0 && delayType != 4 && delayType != 5) {
        delRight_out = delSpread_out; 
    }

    m_audioLeft = delLeft_out * delayWetMix + m_audioLeft * delayDryMix;
    m_audioRight  = delRight_out * delayWetMix + m_audioRight * delayDryMix;
   
}

// Set the delay time from the tap tempo  TODO: Currently the tap tempo led isn't set to delay time on pedal boot up, how to do this?
void DelayModule::SetTempo(uint32_t bpm)
{
    float freq = tempo_to_freq(bpm);
    float delay_in_samples = effect_samplerate / freq; 

    if (delay_in_samples <= m_delaySamplesMin)
    {
        SetParameterRaw(0, 0);
    }
    else if (delay_in_samples >= m_delaySamplesMax)
    {
        SetParameterRaw(0, 127);
    }
    else 
    {
        // Get the parameter as close as we can to target tempo
        SetParameterRaw(0, ((delay_in_samples - m_delaySamplesMin) / (m_delaySamplesMax - m_delaySamplesMin)) * 127); // TODO This was "* 128", verify 
    }
    UpdateLEDRate();
}

float DelayModule::GetBrightnessForLED(int led_id)
{    
    float value = BaseEffectModule::GetBrightnessForLED(led_id);

    float osc_val = led_osc.Process();
    if (osc_val > 0.45) { 
        m_LEDValue = 1.0;
    } else { 
        m_LEDValue = 0.0;
    }

    if (led_id == 1)
    {
        return value * m_LEDValue;
    }

    return value;
}