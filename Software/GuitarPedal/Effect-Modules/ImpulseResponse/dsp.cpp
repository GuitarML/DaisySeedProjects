/*
 * File: dsp.cpp
 * Created Date: March 17, 2023
 * Author: Steven Atkinson (steven@atkinson.mn)
 */


#include "dsp.h"


History::History()
{
}

// Destructor
History::~History()
{
    // No Code Needed
}


void History::_AdvanceHistoryIndex(const size_t bufferSize)
{
  mHistoryIndex += bufferSize;
}


void History::_RewindHistory()
{
  // TODO memcpy?  Should be fine w/ history array being >2x the history length.
  for (size_t i = 0, j = mHistoryIndex - mHistoryRequired; i < mHistoryRequired; i++, j++)
    mHistory[i] = mHistory[j];
  mHistoryIndex = mHistoryRequired;
}

void History::_UpdateHistory(float inputs)
{
  //if (mHistoryIndex + numFrames >= mHistory.size())
  if (mHistoryIndex + 1 >= mHistory.size())
    _RewindHistory();

  mHistory[mHistoryIndex] = inputs;

  // Grabs channel 1, drops hannel 2.
  //for (size_t i = 0, j = mHistoryIndex; i < numFrames; i++, j++)
    // Convert down to float here.
    //mHistory[j] = (float)inputs[i];
}
