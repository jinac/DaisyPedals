#pragma once
#ifndef KIBOOST_H
#define KIBOOST_H

#include <stdint.h>
#include "daisysp.h"
// #include <vector.h>
#ifdef __cplusplus

namespace daisysp {

class KiBoost {
  public:
      KiBoost() {}
      ~KiBoost() {}

      void Init(float samplerate);

      float Process(float &in);

      void SetBright(bool flag);

      void SetBoost(bool flag);

      void SetLevel(float level);

  private:
      // values set from kinky boost: https://gainstaginggeekery.com/2018/02/14/the-kinky-boost/
      float sample_rate_, gain_;
      float freq_cutoff_hpf_ = 20.0f; // HPF filter cutoff ~ 20Hz
      float freq_cutoff_lpf_ = 2000.0f; // LPF filter cutoff
      float freq_cutoff_lpf_bright_ = 6000.0f; // LPF filter cutoff with bright switch

      bool boost_, bright_;

      Tone lpf_;
      ATone hpf_;

      void Reset();

};

}
#endif
#endif