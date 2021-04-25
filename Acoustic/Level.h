#pragma once
#ifndef LEVEL_H
#define LEVEL_H

#include <stdint.h>
// #include <vector.h>
#ifdef __cplusplus

namespace daisysp {

class Level {
  public:
      Level() {}
      ~Level() {}

      void Init(float samplerate, size_t blocksize);

      float ProcessBlock(float* in, size_t blocksize);

      float ProcessBlockMax(float in);

      void SetSampleRate(float samplerate);

      void SetBlockSize(size_t blocksize);

  private:
    float *buffer_;
    // std::vector<float> buffer_;
    float sample_rate_;
    size_t block_size_;
    int buffer_size_, buffer_idx_;
    static const int time_window_ms_ = 400; // Momentary Loudness defined as 400 ms.

    void RecalculateBufferSize();

};

}
#endif
#endif