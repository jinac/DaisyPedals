#include "Level.h"
#include "daisysp.h"
#include <math.h>

using namespace daisysp;

void Level::Init(float samplerate, size_t blocksize) {
    sample_rate_ = samplerate;
    block_size_ = blocksize;
    time_window_ms_ = 400;
    RecalculateBufferSize();
}

void Level::SetSampleRate(float samplerate) {
    sample_rate_ = samplerate;
    RecalculateBufferSize();
}

void Level::SetBlockSize(size_t blocksize) {
    block_size_ = blocksize;
    RecalculateBufferSize();
}

inline float CalculateLevel(float sig_max) {
    return 20.f * fastlog10f(sig_max);
}

float Level::ProcessBlock(float *in, size_t blocksize) {
    float in_max = -1.0f;
    float tmp;
    for (size_t i = 0; i < block_size_; i++) {
        tmp = fabs(in[i]);
        if (tmp > in_max) {
            in_max = tmp;
        }
    }

    buffer_[buffer_idx_] = in_max;

    float sig_max = -1.0f;
    for (int j = 0; j < buffer_size_; j++) {
        tmp = buffer_[j];
        if (tmp > sig_max) {
            sig_max = tmp;
        }
    }
    // for (float val : buffer_) {
    //     if (val > sig_max) {
    //         sig_max = val;
    //     }
    // }

    return CalculateLevel(sig_max);
}

float Level::ProcessBlockMax(float in) {
    buffer_[buffer_idx_] = fabs(in);
    buffer_idx_++;

    float sig_max = -1.0f;
    float tmp;
    for (int i = 0; i < buffer_size_; i++) {
        tmp = buffer_[i];
        if (tmp > sig_max) {
            sig_max = tmp;
        }
    }

    return CalculateLevel(sig_max);
}

void Level::RecalculateBufferSize() {
    buffer_size_ = (int) ((int) sample_rate_ / (time_window_ms_ * (int) block_size_));
    // buffer_.resize(buffer_size_);
    // buffer_idx_ = 0;
    if (buffer_ != NULL) {
        free(buffer_);
    }
    buffer_ = static_cast<float *>(malloc(buffer_size_ * sizeof(float)));
    buffer_idx_ = 0;

}