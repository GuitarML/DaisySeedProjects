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


static const int s_paramCount = 2;
static const ParameterMetaData s_metaData[s_paramCount] = {{name: "Gain", valueType: ParameterValueType::FloatMagnitude, defaultValue: 74, knobMapping: 1, midiCCMapping: 20},
                                                           {name: "Level", valueType: ParameterValueType::FloatMagnitude, defaultValue: 74, knobMapping: 2, midiCCMapping: 21}};

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
    reverb->SetParameter(::Parameter2::LineCount, 4);
}


void CloudSeedModule::ProcessMono(float in)
{
    BaseEffectModule::ProcessMono(in);

    
    float ins[1];
    float outs[1];

    ins[0] = m_audioLeft;

    reverb->Process(ins, outs, 48);
   
    m_audioLeft = outs[0] * GetParameterAsMagnitude(1);
    m_audioRight = m_audioLeft;
}

void CloudSeedModule::ProcessStereo(float inL, float inR)
{    
    // Calculate the mono effect
    ProcessMono(inL);

    // Do the base stereo calculation (which resets the right signal to be the inputR instead of combined mono)
    BaseEffectModule::ProcessStereo(m_audioLeft, inR);
    
    m_audioRight = m_audioLeft;  
    

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
