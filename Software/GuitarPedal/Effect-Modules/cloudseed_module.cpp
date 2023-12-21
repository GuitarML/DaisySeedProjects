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

static const int s_paramCount = 39;
static const ParameterMetaData s_metaData[s_paramCount] = {{name: "DryOut", valueType: ParameterValueType::FloatMagnitude, defaultValue: 74, knobMapping: 0, midiCCMapping: 1},
                                                           {name: "EarlyOut", valueType: ParameterValueType::FloatMagnitude, defaultValue: 74, knobMapping: 1, midiCCMapping: 2},
                                                           {name: "MainOut", valueType: ParameterValueType::FloatMagnitude, defaultValue: 74, knobMapping: 2, midiCCMapping: 3},
                                                           {name: "Time", valueType: ParameterValueType::FloatMagnitude, defaultValue: 74, knobMapping: 3, midiCCMapping: 4},
                                                           {name: "PreDelay", valueType: ParameterValueType::FloatMagnitude, defaultValue: 74, knobMapping: 4, midiCCMapping: 5},
                                                           {name: "Damp", valueType: ParameterValueType::FloatMagnitude, defaultValue: 74, knobMapping: 5, midiCCMapping: 6},
                                                           {name: "Preset", valueType: ParameterValueType::Binned, valueBinCount: 8, valueBinNames: s_presetNames, defaultValue: 0, knobMapping: -1, midiCCMapping: 7},

                                                  /*7*/         {name: "Pre HPass", valueType: ParameterValueType::FloatMagnitude, defaultValue: 74, knobMapping: -1, midiCCMapping: 8},
                                                  /*8*/         {name: "Pre LPass", valueType: ParameterValueType::FloatMagnitude, defaultValue: 74, knobMapping: -1, midiCCMapping: 9},
                                                  /*9*/         {name: "TapLength", valueType: ParameterValueType::FloatMagnitude, defaultValue: 74, knobMapping: -1, midiCCMapping: 10},
                                                  /*10*/         {name: "TapGain", valueType: ParameterValueType::FloatMagnitude, defaultValue: 74, knobMapping: -1, midiCCMapping: 11},
                                                  /*11*/         {name: "TapDecay", valueType: ParameterValueType::FloatMagnitude, defaultValue: 74, knobMapping: -1, midiCCMapping: 12},
                                                  /*12*/         {name: "TapCount", valueType: ParameterValueType::FloatMagnitude, defaultValue: 74, knobMapping: -1, midiCCMapping: 13},

                                                  /*13*/         {name: "PreDiffus", valueType: ParameterValueType::Bool, valueBinCount: 0, defaultValue: 1, knobMapping: -1, midiCCMapping: 14},
                                                  /*14*/         {name: "DiffDelay", valueType: ParameterValueType::FloatMagnitude, defaultValue: 74, knobMapping: -1, midiCCMapping: 15},
                                                  /*15*/         {name: "DiffFdbk", valueType: ParameterValueType::FloatMagnitude, defaultValue: 74, knobMapping: -1, midiCCMapping: 16},
                                                  /*16*/         {name: "LineDelay", valueType: ParameterValueType::FloatMagnitude, defaultValue: 74, knobMapping: -1, midiCCMapping: 17},
                                                  /*17*/         {name: "LineDecay", valueType: ParameterValueType::FloatMagnitude, defaultValue: 74, knobMapping: -1, midiCCMapping: 18},

                                                  /*18*/         {name: "LateDiffus", valueType: ParameterValueType::Bool, valueBinCount: 0, defaultValue: 1, knobMapping: -1, midiCCMapping: 19},
                                                  /*19*/         {name: "LtDiffDelay", valueType: ParameterValueType::FloatMagnitude, defaultValue: 74, knobMapping: -1, midiCCMapping: 20},
                                                  /*20*/         {name: "LtDiffFdbk", valueType: ParameterValueType::FloatMagnitude, defaultValue: 74, knobMapping: -1, midiCCMapping: 21},

                                                  /*21*/         {name: "P LowShelfG", valueType: ParameterValueType::FloatMagnitude, defaultValue: 74, knobMapping: -1, midiCCMapping: 22},
                                                  /*22*/         {name: "P LowShelfF", valueType: ParameterValueType::FloatMagnitude, defaultValue: 74, knobMapping: -1, midiCCMapping: 23},

                                                  /*23*/         {name: "P HiShelfG", valueType: ParameterValueType::FloatMagnitude, defaultValue: 74, knobMapping: -1, midiCCMapping: 24},
                                                  /*24*/         {name: "P HiShelfF", valueType: ParameterValueType::FloatMagnitude, defaultValue: 74, knobMapping: -1, midiCCMapping: 25},

                                                  /*25*/         {name: "E DiffModA", valueType: ParameterValueType::FloatMagnitude, defaultValue: 74, knobMapping: -1, midiCCMapping: 26},
                                                  /*26*/         {name: "E DiffModR", valueType: ParameterValueType::FloatMagnitude, defaultValue: 74, knobMapping: -1, midiCCMapping: 27},

                                                  /*27*/         {name: "LineModA", valueType: ParameterValueType::FloatMagnitude, defaultValue: 74, knobMapping: -1, midiCCMapping: 28},
                                                  /*28*/         {name: "LineModR", valueType: ParameterValueType::FloatMagnitude, defaultValue: 74, knobMapping: -1, midiCCMapping: 29},

                                                  /*29*/         {name: "L DiffModA", valueType: ParameterValueType::FloatMagnitude, defaultValue: 74, knobMapping: -1, midiCCMapping: 30},
                                                  /*30*/         {name: "L DiffModR", valueType: ParameterValueType::FloatMagnitude, defaultValue: 74, knobMapping: -1, midiCCMapping: 31},

                                                  /*31*/         {name: "TapSeed", valueType: ParameterValueType::FloatMagnitude, defaultValue: 74, knobMapping: -1, midiCCMapping: 32},
                                                  /*32*/         {name: "DiffSeed", valueType: ParameterValueType::FloatMagnitude, defaultValue: 74, knobMapping: -1, midiCCMapping: 33},
                                                  /*33*/         {name: "DelaySeed", valueType: ParameterValueType::FloatMagnitude, defaultValue: 74, knobMapping: -1, midiCCMapping: 34},
                                                  /*34*/         {name: "PDiffSeed", valueType: ParameterValueType::FloatMagnitude, defaultValue: 74, knobMapping: -1, midiCCMapping: 35},
                                                  /*35*/         {name: "CrossSeed", valueType: ParameterValueType::FloatMagnitude, defaultValue: 74, knobMapping: -1, midiCCMapping: 36},

                                                  /*36*/         {name: "PreDelOut", valueType: ParameterValueType::FloatMagnitude, defaultValue: 74, knobMapping: -1, midiCCMapping: 37},

                                                  /*37*/         {name: "L StageTap", valueType: ParameterValueType::Bool, valueBinCount: 0, defaultValue: 1, knobMapping: -1, midiCCMapping: 38},
                                                  /*38*/         {name: "Interp", valueType: ParameterValueType::Bool, valueBinCount: 0, defaultValue: 1, knobMapping: -1, midiCCMapping: 39}

};

// NOTES ABOUT THE CLOUDSEED PARAMETERS
// 1. I changed the "Parameter" class to be "Parameter2" to deconflict with the DaisySP Parameter. Likely there was a much easier way to remedy this.
// 2. I have hard coded the presets to work with the limited processing on the Daisy Seed. The main parameters that affect processing are:
//        Mostly these:  LineCount, LateDiffusionStages      And to a lesser degree these:  DiffusionStages, TapCount (TODO Added tapcount as param, see if it can handle full range)
//     Increasing the above params past what I have them set at in "ReverbController.h" may freeze the pedal processing. 
//     Currently the max stereo line count is 2 on the Daisy Seed, so the presets will sound different than the desktop CloudSeed plugin.
//     


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
    //reverb->initFactoryChorus(); // Not setting a preset at the beginning allows you to save the previous preset
    reverb->SetParameter(::Parameter2::LineCount, 2); // 2 on factory chorus for stereo is max, 3 froze it
}

void CloudSeedModule::ParameterChanged(int parameter_id)
{
    if (parameter_id == 6) {  // Preset
        changePreset();
    } else if (parameter_id == 0) {  // DryOut
        reverb->SetParameter(::Parameter2::DryOut, GetParameterAsMagnitude(0));
    } else if (parameter_id == 1) {  // EarlyOut
        reverb->SetParameter(::Parameter2::EarlyOut, GetParameterAsMagnitude(1));
    } else if (parameter_id == 2) {  // MainOut
        reverb->SetParameter(::Parameter2::MainOut, GetParameterAsMagnitude(2));
    } else if (parameter_id == 3) {  // TimeOut
        reverb->SetParameter(::Parameter2::LineDecay, GetParameterAsMagnitude(3));
    } else if (parameter_id == 4) {  // PreDelay
        reverb->SetParameter(::Parameter2::PreDelay, GetParameterAsMagnitude(4));
    } else if (parameter_id == 5) {  // Damp
        reverb->SetParameter(::Parameter2::CutoffEnabled, 1.0); // If this knob is moved, turn on the cutoff filter, the presets will reset this on/off as needed
        reverb->SetParameter(::Parameter2::PostCutoffFrequency, GetParameterAsMagnitude(5));


    } else if (parameter_id == 7) {  // HighPass
        reverb->SetParameter(::Parameter2::HiPassEnabled, 1.0); // If this param is moved, set as enabled
        reverb->SetParameter(::Parameter2::HighPass, GetParameterAsMagnitude(7));

    } else if (parameter_id == 8) {  // LowPass
        reverb->SetParameter(::Parameter2::LowPassEnabled, 1.0); // If this param is moved, set as enabled
        reverb->SetParameter(::Parameter2::LowPass, GetParameterAsMagnitude(8));

    } else if (parameter_id == 9) {  // TapLength
        reverb->SetParameter(::Parameter2::TapLength, GetParameterAsMagnitude(9));

    } else if (parameter_id == 10) {  // TapGain
        reverb->SetParameter(::Parameter2::TapGain, GetParameterAsMagnitude(10));

    } else if (parameter_id == 11) {  // TapDecay
        reverb->SetParameter(::Parameter2::TapDecay, GetParameterAsMagnitude(11));

    } else if (parameter_id == 12) {  // TapCount
        reverb->SetParameter(::Parameter2::TapCount, GetParameterAsMagnitude(12)/ 2.0); // to slow at high counts

    } else if (parameter_id == 13) {  // DiffusionEnabled
        reverb->SetParameter(::Parameter2::DiffusionEnabled, GetParameterAsMagnitude(13));

    } else if (parameter_id == 14) {  // DiffusionDelay
        reverb->SetParameter(::Parameter2::DiffusionDelay, GetParameterAsMagnitude(14));

    } else if (parameter_id == 15) {  // DiffusionFeedback
        reverb->SetParameter(::Parameter2::DiffusionFeedback, GetParameterAsMagnitude(15));

    } else if (parameter_id == 16) {  // LineDelay
        reverb->SetParameter(::Parameter2::LineDelay, GetParameterAsMagnitude(16));

    } else if (parameter_id == 17) {  // LineDecay
        reverb->SetParameter(::Parameter2::LineDecay, GetParameterAsMagnitude(17));

    } else if (parameter_id == 18) {  // LateDiffusionEnabled
        reverb->SetParameter(::Parameter2::LateDiffusionEnabled, GetParameterAsMagnitude(18));

    } else if (parameter_id == 19) {  // LateDiffusionDelay
        reverb->SetParameter(::Parameter2::LateDiffusionDelay, GetParameterAsMagnitude(19));

    } else if (parameter_id == 20) {  // LateDiffusionFeedback
        reverb->SetParameter(::Parameter2::LateDiffusionFeedback, GetParameterAsMagnitude(20));

    } else if (parameter_id == 21) {  // PostLowShelfGain
        reverb->SetParameter(::Parameter2::LowShelfEnabled, 1.0); // If this param is moved, set as enabled
        reverb->SetParameter(::Parameter2::PostLowShelfGain, GetParameterAsMagnitude(21));

    } else if (parameter_id == 22) {  // PostLowShelfFrequency
        reverb->SetParameter(::Parameter2::LowShelfEnabled, 1.0); // If this param is moved, set as enabled
        reverb->SetParameter(::Parameter2::PostLowShelfFrequency, GetParameterAsMagnitude(21));

    } else if (parameter_id == 23) {  // PostHighShelfGain
        reverb->SetParameter(::Parameter2::HighShelfEnabled, 1.0); // If this param is moved, set as enabled
        reverb->SetParameter(::Parameter2::PostHighShelfGain, GetParameterAsMagnitude(21));

    } else if (parameter_id == 24) {  // PreDelay
        reverb->SetParameter(::Parameter2::HighShelfEnabled, 1.0); // If this param is moved, set as enabled
        reverb->SetParameter(::Parameter2::PostHighShelfFrequency, GetParameterAsMagnitude(21));

    } else if (parameter_id == 25) {  // EarlyDiffusionModAmount
        reverb->SetParameter(::Parameter2::EarlyDiffusionModAmount, GetParameterAsMagnitude(25));

    } else if (parameter_id == 26) {  // EarlyDiffusionModRate
        reverb->SetParameter(::Parameter2::EarlyDiffusionModRate, GetParameterAsMagnitude(26));

    } else if (parameter_id == 27) {  // LineModAmount
        reverb->SetParameter(::Parameter2::LineModAmount, GetParameterAsMagnitude(27));

    } else if (parameter_id == 28) {  // LineModRate
        reverb->SetParameter(::Parameter2::LineModRate, GetParameterAsMagnitude(28));

    } else if (parameter_id == 29) {  // LateDiffusionModAmount
        reverb->SetParameter(::Parameter2::LateDiffusionModAmount, GetParameterAsMagnitude(29));

    } else if (parameter_id == 30) {  // LateDiffusionModRate
        reverb->SetParameter(::Parameter2::LateDiffusionModRate, GetParameterAsMagnitude(30));

    } else if (parameter_id == 31) {  // TapSeed
        reverb->SetParameter(::Parameter2::TapSeed, GetParameterAsMagnitude(31));

    } else if (parameter_id == 32) {  // DiffusionSeed
        reverb->SetParameter(::Parameter2::DiffusionSeed, GetParameterAsMagnitude(32));

    } else if (parameter_id == 33) {  // DelaySeed
        reverb->SetParameter(::Parameter2::DelaySeed, GetParameterAsMagnitude(33));

    } else if (parameter_id == 34) {  // PostDiffusionSeed
        reverb->SetParameter(::Parameter2::PostDiffusionSeed, GetParameterAsMagnitude(34));

    } else if (parameter_id == 35) {  // CrossSeed
        reverb->SetParameter(::Parameter2::CrossSeed, GetParameterAsMagnitude(35));

    } else if (parameter_id == 36) {  // PredelayOut
        reverb->SetParameter(::Parameter2::PredelayOut, GetParameterAsMagnitude(36));

    } else if (parameter_id == 37) {  // LateStageTap
        reverb->SetParameter(::Parameter2::LateStageTap, GetParameterAsBool(37));

    } else if (parameter_id == 38) {  // Interpolation
        reverb->SetParameter(::Parameter2::Interpolation, GetParameterAsBool(38));
    }
}

void CloudSeedModule::changePreset()
{
    int c = (GetParameterAsBinnedValue(6) - 1);
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

void CloudSeedModule::SetTempo(uint32_t bpm)
{
    // Cycles Presets with the left footswitch (on double tap)
    /*
    u_int8_t bin = GetParameterAsBinnedValue(6);
    bin += 1;
    if (bin > 8) {
        bin = 1;
    }
    SetParameterAsBinnedValue(6, bin);
    */
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