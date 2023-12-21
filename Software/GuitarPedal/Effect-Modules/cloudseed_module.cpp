#include "cloudseed_module.h"
#include "../Util/audio_utilities.h"

// This is used in the modified CloudSeed code for allocating 
// delay line memory to SDRAM (64MB available on Daisy)
#define CUSTOM_POOL_SIZE (48*1024*1024)
DSY_SDRAM_BSS char custom_pool[CUSTOM_POOL_SIZE];
size_t pool_index = 0;
int allocation_count = 0;
void* custom_pool_allocate(size_t size)
{
        if (pool_index + size >= CUSTOM_POOL_SIZE)
        {
                return 0;
        }
        void* ptr = &custom_pool[pool_index];
        pool_index += size;
        return ptr;
}

using namespace bkshepherd;

static const char* s_presetNames[8] = {"FChorus", "DullEchos", "Hyperplane", "MedSpace", "Hallway", "RubiKa", "SmallRoom", "90s"};

static const int s_paramCount = 5;
static const ParameterMetaData s_metaData[s_paramCount] = {{name: "DryOut", valueType: ParameterValueType::FloatMagnitude, defaultValue: 74, knobMapping: 0, midiCCMapping: 1},
                                                           {name: "EarlyOut", valueType: ParameterValueType::FloatMagnitude, defaultValue: 74, knobMapping: 1, midiCCMapping: 2},
                                                           {name: "MainOut", valueType: ParameterValueType::FloatMagnitude, defaultValue: 74, knobMapping: 2, midiCCMapping: 3},
                                                           {name: "Time", valueType: ParameterValueType::FloatMagnitude, defaultValue: 74, knobMapping: 3, midiCCMapping: 4},
                                                           {name: "Preset", valueType: ParameterValueType::Binned, valueBinCount: 8, valueBinNames: s_presetNames, defaultValue: 0, knobMapping: -1, midiCCMapping: 50}};

// Default Constructor
CloudSeedModule::CloudSeedModule() : BaseEffectModule(),
                                                        m_gainMin(0.0f),
                                                        m_gainMax(1.0f),
                                                        m_cachedEffectMagnitudeValue(1.0f)
{
    // Set the name of the effect
    m_name = "CloudSeed";

    // Setup the meta data reference for this Effect
    m_paramMetaData = s_metaData;
    
    // Initialize Parameters for this Effect
    this->InitParams(s_paramCount);
}

// Destructor
CloudSeedModule::~CloudSeedModule()
{
    // No Code Needed
}

void CloudSeedModule::Init(float sample_rate)
{
    BaseEffectModule::Init(sample_rate);

    AudioLib::ValueTables::Init();
    CloudSeed::FastSin::Init();
    
    reverb = new CloudSeed::ReverbController(sample_rate);
    reverb->ClearBuffers();
    reverb->initFactoryChorus();
    reverb->SetParameter(::Parameter2::LineCount, 2); // 2 on factory chorus for stereo is max, 3 froze it
}

void CloudSeedModule::ParameterChanged(int parameter_id)
{
    if (parameter_id == 4) {  // Preset
        changePreset();
    } else if (parameter_id == 0) {  // DryOut
        reverb->SetParameter(::Parameter2::DryOut, GetParameterAsMagnitude(0));
    } else if (parameter_id == 1) {  // EarlyOut
        reverb->SetParameter(::Parameter2::EarlyOut, GetParameterAsMagnitude(1));
    } else if (parameter_id == 2) {  // MainOut
        reverb->SetParameter(::Parameter2::MainOut, GetParameterAsMagnitude(2));
    } else if (parameter_id == 3) {  // TimeOut
        reverb->SetParameter(::Parameter2::LineDecay, GetParameterAsMagnitude(3));
    } 

}

void CloudSeedModule::changePreset()
{
    int c = (GetParameterAsBinnedValue(4) - 1);
    reverb->ClearBuffers();
        
    if ( c == 0 ) {
            reverb->initFactoryChorus();
    } else if ( c == 1 ) {
            reverb->initFactoryDullEchos();
    } else if ( c == 2 ) {
            reverb->initFactoryHyperplane();
    } else if ( c == 3 ) {
            reverb->initFactoryMediumSpace();
    } else if ( c == 4 ) {
            reverb->initFactoryNoiseInTheHallway();
    } else if ( c == 5 ) {
            reverb->initFactoryRubiKaFields();
    } else if ( c == 6 ) {
            reverb->initFactorySmallRoom();
    } else if ( c == 7 ) {
            reverb->initFactory90sAreBack();
    }
}


void CloudSeedModule::ProcessMono(float in)
{
    BaseEffectModule::ProcessMono(in);

    
    float inL[1];
    float outL[1];
    float inR[1];
    float outR[1];

    inL[0] = m_audioLeft;
    inR[0] = m_audioRight;

    reverb->Process(inL, inR, outL, outR, 1);
   
    m_audioLeft = outL[0];
    m_audioRight = outR[0];
}

void CloudSeedModule::ProcessStereo(float inL, float inR)
{    
    // Calculate the mono effect
    ProcessMono(inL);

    // Do the base stereo calculation (which resets the right signal to be the inputR instead of combined mono)
    //BaseEffectModule::ProcessStereo(m_audioLeft, inR);
    
    //m_audioRight = m_audioLeft;  
    

    // Use the same magnitude as already calculated for the Left Audio
    //m_audioRight = m_audioRight * m_cachedEffectMagnitudeValue;
}


float CloudSeedModule::GetBrightnessForLED(int led_id)
{    
    float value = BaseEffectModule::GetBrightnessForLED(led_id);

    if (led_id == 1)
    {
        return value * m_cachedEffectMagnitudeValue;
    }

    return value;
}
