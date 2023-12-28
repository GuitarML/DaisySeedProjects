//
//  ImpulseResponse.cpp
//  NeuralAmpModeler-macOS
//
//  Created by Steven Atkinson on 12/30/22.
//


#include "ImpulseResponse.h"


ImpulseResponse::ImpulseResponse()
{
}

// Destructor
ImpulseResponse::~ImpulseResponse()
{
    // No Code Needed
}


void ImpulseResponse::Init(std::vector<float> irData, const float sampleRate)
{
  //const size_t irLength = irData.size();
  //.resize(irLength);
  mRawAudio = irData;
  mRawAudioSampleRate = sampleRate;
  _SetWeights();
}

float ImpulseResponse::Process(float inputs)
{

  _UpdateHistory(inputs);

  int j = mHistoryIndex - mHistoryRequired;
  auto input = Eigen::Map<const Eigen::VectorXf>(&mHistory[j], mHistoryRequired + 1);
  
  //for (size_t i = 0, j = mHistoryIndex - mHistoryRequired; i < numFrames; i++, j++)
  //{
  //  auto input = Eigen::Map<const Eigen::VectorXf>(&mHistory[j], mHistoryRequired + 1);
  //  mOutputs[0][i] = (float)mWeight.dot(input);
  //}

  _AdvanceHistoryIndex(1); // KAB MOD - for Daisy implementation numFrames is always 1

  return (float)mWeight.dot(input);

}

void ImpulseResponse::_SetWeights()
{

  const size_t irLength = std::min(mRawAudio.size(), mMaxLength);
  mWeight.resize(irLength);
  // Gain reduction.
  // https://github.com/sdatkinson/NeuralAmpModelerPlugin/issues/100#issuecomment-1455273839
  // Add sample rate-dependence
  //const float gain = pow(10, -18 * 0.05) * 48000 / mSampleRate;  // This made a very bad/loud sound on Daisy Seed
  for (size_t i = 0, j = irLength - 1; i < irLength; i++, j--)
    //mWeight[j] = gain * mRawAudio[i];  
    mWeight[j] = mRawAudio[i];
  mHistoryRequired = irLength - 1;

  // Moved from HISTORY::EnsureHistorySize since only doing once (for testing purposes)
  //   TODO: If this works on Daisy, find a more efficient method using indexing,
  //         rather than copying the end of the vector (length of IR) back to the beginning all at once.
  const size_t requiredHistoryArraySize = 5 * mHistoryRequired; // Just so we don't spend too much time copying back. // was 10 *
  mHistory.resize(requiredHistoryArraySize);
  std::fill(mHistory.begin(), mHistory.end(), 0.0f);
  mHistoryIndex = mHistoryRequired; // Guaranteed to be less than
                                                  // requiredHistoryArraySize

}
