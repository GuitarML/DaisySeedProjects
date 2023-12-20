#include "neural_net_module.h"
#include "../Util/audio_utilities.h"
#include "NeuralModels/model_data_gru12.h"

using namespace bkshepherd;

static const char* s_modelBinNames[1] = {"Klon"};

static const int s_paramCount = 5;
static const ParameterMetaData s_metaData[s_paramCount] = {{name: "Gain", valueType: ParameterValueType::FloatMagnitude, defaultValue: 64, knobMapping: 0, midiCCMapping: 1},
                                                           {name: "Mix", valueType: ParameterValueType::FloatMagnitude, defaultValue: 64, knobMapping: 1, midiCCMapping: 2},
                                                           {name: "Level", valueType: ParameterValueType::FloatMagnitude, defaultValue: 64, knobMapping: 2, midiCCMapping: 3},
                                                           {name: "Tone", valueType: ParameterValueType::FloatMagnitude, valueBinCount: 0, defaultValue: 64, knobMapping: 3, midiCCMapping: 4},
                                                           {name: "Model", valueType: ParameterValueType::Binned, valueBinCount: 1, valueBinNames: s_modelBinNames, defaultValue: 0, knobMapping: -1, midiCCMapping: 20},
};

RTNeural::ModelT<float, 1, 1,
    RTNeural::GRULayerT<float, 1, 11>,
    RTNeural::DenseT<float, 11, 1>> model;
// 12 is currently the max size GRU I was able to get working with OPT flag on, 13 froze it
// 11 seems to be more practical, can add a few quality of life features

// Default Constructor
NeuralNetModule::NeuralNetModule() : BaseEffectModule(),
                                                        m_gainMin(0.0f),
                                                        m_gainMax(2.0f),
                                                        m_toneFreqMin(400.0f),
                                                        m_toneFreqMax(20000.0f),
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
    CalculateMix();
    tone.Init(sample_rate);
    bal.Init(sample_rate);
    CalculateTone();
}

void NeuralNetModule::ParameterChanged(int parameter_id)
{
    if (parameter_id == 4) {  // Change Model
        SelectModel();
    } else if (parameter_id == 1) {
        CalculateMix();
    } else if (parameter_id == 3) {
        CalculateTone();
    }
}
void NeuralNetModule::SelectModel()
{
    int modelIndex = GetParameterAsBinnedValue(1) - 1;
    auto& gru = (model).template get<0>();
    auto& dense = (model).template get<1>();
    gru.setWVals(model_collection[modelIndex].rec_weight_ih_l0);
    gru.setUVals(model_collection[modelIndex].rec_weight_hh_l0);
    gru.setBVals(model_collection[modelIndex].rec_bias);
    dense.setWeights(model_collection[modelIndex].lin_weight);
    dense.setBias(model_collection[modelIndex].lin_bias.data());
    model.reset();
    nnLevelAdjust = model_collection[modelIndex].levelAdjust;
}

void NeuralNetModule::CalculateMix()
{
    //    A computationally cheap mostly energy constant crossfade from SignalSmith Blog
    //    https://signalsmith-audio.co.uk/writing/2021/cheap-energy-crossfade/

    float mixKnob = GetParameterAsMagnitude(1);
    float x2 = 1.0 - mixKnob;
    float A = mixKnob*x2;
    float B = A * (1.0 + 1.4186 * A);
    float C = B + mixKnob;
    float D = B + x2;

    wetMix = C * C;
    dryMix = D * D;
}

void NeuralNetModule::CalculateTone()
{
    // Set low pass filter as exponential taper 
    tone.SetFreq(m_toneFreqMin + GetParameterAsMagnitude(3) * GetParameterAsMagnitude(3) * (m_toneFreqMax - m_toneFreqMin)); 
}


void NeuralNetModule::ProcessMono(float in)
{
    BaseEffectModule::ProcessMono(in);

    float ampOut;
    float input_arr[1] = { 0.0 };    // Neural Net Input
    input_arr[0] = m_audioLeft * (m_gainMin + (m_gainMax - m_gainMin) * GetParameterAsMagnitude(0));


    // Process Neural Net Model //
    ampOut = model.forward (input_arr) + input_arr[0];   // Run Model and add Skip Connection
    ampOut *= nnLevelAdjust;

    // TONE //
    float filter_out = tone.Process(ampOut);  // Apply tone Low Pass filter
    float balanced_out = bal.Process(filter_out, ampOut); // Apply level adjustment to increase level of filtered signal


    m_audioLeft = (balanced_out * wetMix) * GetParameterAsMagnitude(2) / 4.0 + input_arr[0] * dryMix; // Applies model level adjustment, wet/dry mix, and output level
    m_audioRight = m_audioLeft;
}

void NeuralNetModule::ProcessStereo(float inL, float inR)
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


float NeuralNetModule::GetBrightnessForLED(int led_id)
{    
    float value = BaseEffectModule::GetBrightnessForLED(led_id);

    if (led_id == 1)
    {
        return value * m_cachedEffectMagnitudeValue;
    }

    return value;
}
