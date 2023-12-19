#include "neural_net_module.h"
#include "../Util/audio_utilities.h"
#include "NeuralModels/model_data_gru9.h"

using namespace bkshepherd;

static const char* s_modelBinNames[9] = {"Fender57", "Matcheless", "Klon", "Mesa", "HAK", "Bassman", "5150", "Splawn", "Klon HighG"};

static const int s_paramCount = 3;
static const ParameterMetaData s_metaData[s_paramCount] = {{name: "Gain", valueType: ParameterValueType::FloatMagnitude, defaultValue: 74, knobMapping: 0, midiCCMapping: 20},
                                                           {name: "Model", valueType: ParameterValueType::Binned, valueBinCount: 9, valueBinNames: s_modelBinNames, defaultValue: 0, knobMapping: 1, midiCCMapping: 23},
                                                           {name: "Level", valueType: ParameterValueType::FloatMagnitude, defaultValue: 74, knobMapping: 2, midiCCMapping: 21}};

RTNeural::ModelT<float, 1, 1,
    RTNeural::GRULayerT<float, 1, 9>,
    RTNeural::DenseT<float, 9, 1>> model;

// Default Constructor
NeuralNetModule::NeuralNetModule() : BaseEffectModule(),
                                                        m_gainMin(0.0f),
                                                        m_gainMax(1.0f),
                                                        m_cachedEffectMagnitudeValue(1.0f)
{
    // Set the name of the effect
    m_name = "NeuralNet";

    // Setup the meta data reference for this Effect
    m_paramMetaData = s_metaData;
    
    // Initialize Parameters for this Effect
    this->InitParams(s_paramCount);
}

// Destructor
NeuralNetModule::~NeuralNetModule()
{
    // No Code Needed
}

void NeuralNetModule::Init(float sample_rate)
{
    BaseEffectModule::Init(sample_rate);
    setupWeights(); // in the model data .h file
    SelectModel();
}

void NeuralNetModule::ParameterChanged(int parameter_id)
{
    if (parameter_id == 1) {  // Delay Time
        SelectModel();
    } 
}

void NeuralNetModule::SelectModel()
{
    int modelIndex = GetParameterAsBinnedValue(1) - 1;
    //int modelIndex = 4;
    //modelIndex = modelIndex_temp;
    auto& gru = (model).template get<0>();
    auto& dense = (model).template get<1>();
    //modelInSize = 1;
    gru.setWVals(model_collection[modelIndex].rec_weight_ih_l0);
    gru.setUVals(model_collection[modelIndex].rec_weight_hh_l0);
    gru.setBVals(model_collection[modelIndex].rec_bias);
    dense.setWeights(model_collection[modelIndex].lin_weight);
    dense.setBias(model_collection[modelIndex].lin_bias.data());
    model.reset();

    //nnLevelAdjust = model_collection[modelIndex].levelAdjust;
}

void NeuralNetModule::ProcessMono(float in)
{
    BaseEffectModule::ProcessMono(in);

    float ampOut;
    float input_arr[1] = { 0.0 };    // Neural Net Input
    //float input = in;
    input_arr[0] = m_audioLeft * GetParameterAsMagnitude(0);


    // Process Neural Net Model //
    ampOut = model.forward (input_arr) + input_arr[0];   // Run Model and add Skip Connection
    //ampOut *= nnLevelAdjust;

    m_audioLeft = ampOut * GetParameterAsMagnitude(2);
    m_audioRight = m_audioLeft;
}

void NeuralNetModule::ProcessStereo(float inL, float inR)
{    
    // Calculate the mono effect
    ProcessMono(inL);

    // Do the base stereo calculation (which resets the right signal to be the inputR instead of combined mono)
    //BaseEffectModule::ProcessStereo(m_audioLeft, inR);
    
    //m_audioRight = m_audioLeft;  // Currently only processing mono for neural net, probably can't keep up with stereo in realtime with gru9 models
    

    // Use the same magnitude as already calculated for the Left Audio
    //m_audioRight = m_audioRight * m_cachedEffectMagnitudeValue;
}


float NeuralNetModule::GetBrightnessForLED(int led_id)
{    
    float value = BaseEffectModule::GetBrightnessForLED(led_id);

    if (led_id == 1)
    {
        return value * m_cachedEffectMagnitudeValue;
    }

    return value;
}
