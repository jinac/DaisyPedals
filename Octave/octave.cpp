#include "octave.h"
#include "daisysp.h"
#include <math.h>

using namespace daisysp;

void Octave::Reset() {
    // cycles_since_last_pos_zero_cross_ = 0;

    // Reset in_lpf_.
    in_lpf_.Init(sample_rate_);
    in_lpf_.SetFreq(in_freq_cutoff_);

    // Reset out_lpf_.
    out_lpf_.Init(sample_rate_);
    out_lpf_.SetFreq(out_freq_cutoff_);

    // Reset comp_.
    atk_coeff_ = expf(-1.0 / (sample_rate_ * 0.075));
    rel_coeff_ = expf(-1.0 / (sample_rate_ * 0.01));
    env_ = 0.0f;

    sq_wv_ = 1.0f;
}

void Octave::Init(float samplerate) {
    sample_rate_ = samplerate;
    sq_wv_ = 1.0f;

    Reset();
}

float Octave::Process(float &in) {
    float out, tmp;

    // Input Low pass filter.
    out = in_lpf_.Process(in);

    // Zero cross and calculate freq.
    if (last_sample_ <= 0.0f && out > 0.0f) {
        sq_wv_ = -sq_wv_;
    }
    last_sample_ = out;

    // Side chain compression as envelope follower.
    tmp = fabs(in);
    if (tmp > env_) {
        env_ = atk_coeff_ * (env_ - tmp) + tmp;
    } else {
        env_ = rel_coeff_ * (env_ - tmp) + tmp;
    }

    tmp = env_ * sq_wv_;
    return out_lpf_.Process(tmp);
}
