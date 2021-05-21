#pragma once
#ifndef OCTAVE_H
#define OCTAVE_H

#include <stdint.h>
#include "daisysp.h"
// #include <vector.h>
#ifdef __cplusplus

namespace daisysp {

class Octave {
  public:
      Octave() {}
      ~Octave() {}

      void Init(float samplerate);

      float Process(float &in);

  private:
      float sample_rate_, last_sample_, freq_, env_, atk_coeff_, rel_coeff_, sq_wv_;
      int skip_, count_;
      // int cycles_since_last_pos_zero_cross_, skip_, count_;
      bool flip_;

      void Reset();

      float in_freq_cutoff_ = 284.0f;
      float out_freq_cutoff_ = 393.0f;
      // float freq_cutoff_lpf_ = 1200.0f;
      // float freq_env_fol_ = 60.0f;
      // float freq_cutoff_lpf_guitar = 1200.0f;
      Tone in_lpf_;
      Tone out_lpf_;
      // Tone env_fol_;
      // Oscillator osc_;

};

}
#endif
#endif