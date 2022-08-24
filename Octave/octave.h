#pragma once
#ifndef OCTAVE_H
#define OCTAVE_H

#include <stdint.h>
#include "daisysp.h"
#ifdef __cplusplus

namespace daisysp {

class Octave {
  public:
      Octave() {}
      ~Octave() {}

      void Init(float samplerate);

      float Process(float &in);

  private:
      float sample_rate_, last_sample_, env_, atk_coeff_, rel_coeff_, sq_wv_;

      void Reset();

      float in_freq_cutoff_ = 284.0f;
    //   float out_freq_cutoff_ = 393.0f;
      float out_freq_cutoff_ = 1200.0f;
      Tone in_lpf_;
      Tone out_lpf_;

};

}
#endif
#endif