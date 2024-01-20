#include "nam_module.h"
#include "../Util/audio_utilities.h"
#include "namData/nam_model_data_nano.h"
#include "../NeuralAmpModelerCore/NAM/wavenet.h"
#include "../NeuralAmpModelerCore/NAM/wavenet.cpp"
#include "../NeuralAmpModelerCore/NAM/dsp.h"
#include "../NeuralAmpModelerCore/NAM/dsp.cpp"
#include "../NeuralAmpModelerCore/NAM/activations.h"
#include "../NeuralAmpModelerCore/NAM/activations.cpp"
//#include "ImpulseResponse/ir_data.h"

using namespace bkshepherd;
using namespace nam;

/*
static const char* s_modelBinNames[14] = {"Klon", "Fender57", "TS9", "Bassman", "5150 G75",
                                          "5150 G5", "ENGLInvG5", "ENGLInvG75", "TS7 Hot", "Matchless",
                                          "Mesa Amp", "Victory", "Ethos"};
*/

static const char* s_modelBinNames[1] = {"Klone"};

//static const char* s_irNames[10] = {"Rhythm", "Lead", "Clean", "Marsh", "Bogn",
//                                   "Proteus", "Rectify", "Rhythm2", "US Deluxe", "British"};

static const int s_paramCount = 4;
static const ParameterMetaData s_metaData[s_paramCount] = {{name: "Gain", valueType: ParameterValueType::FloatMagnitude, defaultValue: 64, knobMapping: 0, midiCCMapping: 1},
                                                           //{name: "Mix", valueType: ParameterValueType::FloatMagnitude, defaultValue: 127, knobMapping: 1, midiCCMapping: 2},
                                                           {name: "Level", valueType: ParameterValueType::FloatMagnitude, defaultValue: 64, knobMapping: 2, midiCCMapping: 3},
                                                           //{name: "Tone", valueType: ParameterValueType::FloatMagnitude, defaultValue: 127, knobMapping: 3, midiCCMapping: 4},
                                                           {name: "Model", valueType: ParameterValueType::Binned, valueBinCount: 1, valueBinNames: s_modelBinNames, defaultValue: 0, knobMapping: 4, midiCCMapping: 5},
                                                           //{name: "IR", valueType: ParameterValueType::Binned, valueBinCount: 10, valueBinNames: s_irNames, defaultValue: 0, knobMapping: 5, midiCCMapping: 6},
                                                           {name: "NeuralModel", valueType: ParameterValueType::Bool, valueBinCount: 0, defaultValue: 127, knobMapping: -1, midiCCMapping: 10},
                                                           //{name: "IR On", valueType: ParameterValueType::Bool, valueBinCount: 0, defaultValue: 127, knobMapping: -1, midiCCMapping: 11}
};

//RTNeural::ModelT<float, 1, 1,
//    RTNeural::GRULayerT<float, 1, 9>,
//    RTNeural::DenseT<float, 9, 1>> model;
// 12 is currently the max size GRU I was able to get working with OPT flag on, 13 froze it
// 11 seems to be more practical, can add a few quality of life features

wavenet::WaveNet namModel;

/*
layer_array_params.push_back(wavenet::LayerArrayParams(nam_collection[0].input_size, nam_collection[0].condition_size, nam_collection[0].head_size,
                            nam_collection[0].channels, nam_collection[0].kernel_size, nam_collection[0].dilations,
                            nam_collection[0].activation, nam_collection[0].gated, nam_collection[0].head_bias));

layer_array_params.push_back(wavenet::LayerArrayParams(nam_collection[0].input_size2, nam_collection[0].condition_size2, nam_collection[0].head_size2,
                            nam_collection[0].channels2, nam_collection[0].kernel_size2, nam_collection[0].dilations2,
                            nam_collection[0].activation2, nam_collection[0].gated2, nam_collection[0].head_bias2));
*/
//nam::wavenet::WaveNet::WaveNet(const std::vector<nam::wavenet::LayerArrayParams>& layer_array_params,
//                              const float head_scale, const bool with_head, std::vector<float> weights,
//                               const float expected_sample_rate)
//wavenet::WaveNet namModel(layer_array_params, nam_collection[0].head_scale, nam_collection[0].with_head, nam_collection[0].weights, 48000.0);

//std::unique_ptr<DSP> namModel = nullptr;
//namModel = std::make_unique<wavenet::WaveNet>(layer_array_params, nam_collection[0].head_scale, nam_collection[0].with_head, nam_collection[0].weights, 48000.0);

//wavenet::WaveNet namModel(layer_array_params, nam_collection[0].head_scale, nam_collection[0].with_head, nam_collection[0].weights, 48000.0);

//nam::wavenet::WaveNet::WaveNet(std::vector<nam::wavenet::LayerArrayParams, std::allocator<nam::wavenet::LayerArrayParams> > const&, float, bool, std::vector<float, std::allocator<float> >, float)'

//auto namModel = wavenet::WaveNet(layer_array_params, nam_collection[0].head_scale, nam_collection[0].with_head, nam_collection[0].weights, 48000.0);

// Default Constructor
NamModule::NamModule() : BaseEffectModule(),
                                                        m_gainMin(0.0f),
                                                        m_gainMax(2.0f),
                                                        m_toneFreqMin(400.0f),
                                                        m_toneFreqMax(20000.0f),
                                                        m_cachedEffectMagnitudeValue(1.0f)
{
    // Set the name of the effect
    m_name = "NAM";

    // Setup the meta data reference for this Effect
    m_paramMetaData = s_metaData;
    
    // Initialize Parameters for this Effect
    this->InitParams(s_paramCount);
}

// Destructor
NamModule::~NamModule()
{
    // No Code Needed
}

void NamModule::Init(float sample_rate)
{
    BaseEffectModule::Init(sample_rate);
    setupNamWeights(); // in the model data .h file
    SelectModel();
    

}

void NamModule::ParameterChanged(int parameter_id)
{
    if (parameter_id == 2) {  // Change Model
        SelectModel();
    } 
}
void NamModule::SelectModel()
{
    int modelIndex = GetParameterAsBinnedValue(2) - 1;
    
    if (m_currentModelindex != modelIndex) {  



        m_currentModelindex = modelIndex;
    }
    nam::activations::Activation::enable_fast_tanh();

    wavenet::LayerArrayParams test1(nam_collection[0].input_size, nam_collection[0].condition_size, nam_collection[0].head_size,
                        nam_collection[0].channels, nam_collection[0].kernel_size, nam_collection[0].dilations,
                        nam_collection[0].activation, nam_collection[0].gated, nam_collection[0].head_bias);
    //wavenet::LayerArrayParams test2(nam_collection[0].input_size2, nam_collection[0].condition_size2, nam_collection[0].head_size2,
    //                            nam_collection[0].channels2, nam_collection[0].kernel_size2, nam_collection[0].dilations2,
    //                            nam_collection[0].activation2, nam_collection[0].gated2, nam_collection[0].head_bias2);

    std::vector<wavenet::LayerArrayParams> layer_array_params = {test1};
    //wavenet::WaveNet namModel(layer_array_params, nam_collection[0].head_scale, nam_collection[0].with_head, nam_collection[0].weights, 48000.0);
    namModel.Init(layer_array_params, nam_collection[0].head_scale, nam_collection[0].with_head, nam_collection[0].weights, 48000.0);

}





void NamModule::ProcessMono(float in)
{
    BaseEffectModule::ProcessMono(in);

    // GAIN and PREPARE INPUT // 
    // Order of processing is Gain -> Neural Model -> EQ filter w/ level balance -> Wet/Dry mix -> Impulse Response -> Output Level


    // NEURAL MODEL //
    float input = m_audioLeft;
    float output = 0.0;
    if (GetParameterAsBool(3))
    {
        //void nam::wavenet::WaveNet::process(NAM_SAMPLE* input, NAM_SAMPLE* output, 1)
        //output = namModel.process(input, 1);

    } else {
        output = input;
    }


    m_audioLeft = output * GetParameterAsMagnitude(1);
    m_audioRight = output *  GetParameterAsMagnitude(1);
}

void NamModule::ProcessStereo(float inL, float inR)
{    
    // Calculate the mono effect
    ProcessMono(inL);

    // NOTE: Running the Neural Nets in stereo is currently not feasible due to processing limitations, this will remain a MONO ONLY effect for now.
    //       The left channel output is copied to the right output, but the right input is ignored in this effect module.

    // Do the base stereo calculation (which resets the right signal to be the inputR instead of combined mono)
    //BaseEffectModule::ProcessStereo(m_audioLeft, inR);
    
    //m_audioRight = m_audioLeft;
    
    // Use the same magnitude as already calculated for the Left Audio
    //m_audioRight = m_audioRight * m_cachedEffectMagnitudeValue;
}


float NamModule::GetBrightnessForLED(int led_id)
{    
    float value = BaseEffectModule::GetBrightnessForLED(led_id);

    if (led_id == 1)
    {
        return value * m_cachedEffectMagnitudeValue;
    }

    return value;
}
