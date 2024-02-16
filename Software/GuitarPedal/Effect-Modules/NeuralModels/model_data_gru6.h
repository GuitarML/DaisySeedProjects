struct modelData {
  std::vector<std::vector<float>> rec_weight_ih_l0; 
  std::vector<std::vector<float>> rec_weight_hh_l0;  
  std::vector<std::vector<float>> lin_weight;
  std::vector<float> lin_bias;
  std::vector<std::vector<float>> rec_bias;
  float levelAdjust;
};

// ADD YOUR MODEL IDENTIFIER HERE ////////////////////////////////// < -------------------
modelData Model1; 



/////////////////////////////////////////////////////////////////

std::vector<modelData> model_collection;

/*========================================================================*/

void setupWeights() {

// COPY AND PASTE YOUR MODEL WEIGHTS BELOW (After converting .json to .h file) ////////////////////////////////// < -------------------
//   ADD AND REMOVE MODELS AS DESIRED (CAN HOLD AROUND 15-16 MODELS IN FLASH MEMORY)


//========================================================================
//../newNeuralSeedModel
/*
model : SimpleRNN
input_size : 1
skip : 1
output_size : 1
unit_type : GRU
num_layers : 1
hidden_size : 2
bias_fl : True
*/

  Model1.rec_weight_ih_l0 = {{1.0139787197113037, 2.9825797080993652, 1.2472124099731445, 3.150390863418579, -1.5450150966644287, 1.8350377082824707}}; 

  Model1.rec_weight_hh_l0 = {{0.698763906955719, 3.882492780685425, 0.9913920164108276, 2.9897453784942627, -0.5614499449729919, 2.19010066986084}, 
                            { -0.2833782434463501, 2.6996567249298096, -0.3616560399532318, -0.09247095137834549, -0.3450881540775299, 1.490613341331482}}; 

  Model1.lin_weight = {{1.1280078887939453, -1.1408512592315674}}; 

  Model1.lin_bias = {-0.08455488085746765}; 

  Model1.rec_bias = {{1.5979323387145996, -0.6241311430931091, -0.21615047752857208, 0.07251371443271637, 0.42379361391067505, -0.4128590524196625}, 
                    { 1.5481172800064087, -0.18020188808441162, 0.06090473383665085, 0.7659458518028259, -0.8148460984230042, 0.4995129108428955}}; 



  Model1.levelAdjust = 0.9;

 

  /*========================================================================*/

  // ADD YOUR MODEL IDENTIFIER HERE ////////////////////////////////// < -------------------------
  model_collection = {  Model1
                     };
}