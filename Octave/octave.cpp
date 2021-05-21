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

    // Reset env_fol_.
    // env_fol_.Init(sample_rate_);
    // env_fol_.SetFreq(freq_env_fol_);

    // Reset osc_.
    // osc_.Init(sample_rate_);
    // osc_.SetAmp(0.5);
    // osc_.SetWaveform(osc_.WAVE_SIN);
    // osc_.Reset();

    // Reset comp_.
    atk_coeff_ = expf(-1.0 / (sample_rate_ * 0.075));
    rel_coeff_ = expf(-1.0 / (sample_rate_ * 0.01));
    env_ = 0.0f;

    // sq_wv_ = 0.0f;
    skip_ = 12;
    count_ = 0;
}

void Octave::Init(float samplerate) {
    sample_rate_ = samplerate;
    sq_wv_ = 1.0f;
    flip_ = false;

    Reset();
}

float Octave::Process(float &in) {
    float out, tmp;

    // Input Low pass filter.
    out = in_lpf_.Process(in);
    // out = in;

    // Zero cross and calculate freq.
    // cycles_since_last_pos_zero_cross_++;
    if (count_ >= skip_) {
        if (last_sample_ <= 0.0f && out > 0.0f) {
            // freq_ = sample_rate_ / (float) (2 * cycles_since_last_pos_zero_cross_);
            // osc_.SetFreq(freq_);
            
            sq_wv_ = -sq_wv_;
            // if (flip_) {
            //     sq_wv_ = -sq_wv_;
            //     flip_ = false;
            // } else {
            //     flip_ = true;
            // }

            // cycles_since_last_pos_zero_cross_ = 0;
        }
        last_sample_ = out;
        count_ = 0;
    } else {
        count_++;
    }

    // Side chain compression as envelope follower.
    tmp = fabs(in);
    if (tmp > env_) {
        env_ = atk_coeff_ * (env_ - tmp) + tmp;
    } else {
        env_ = rel_coeff_ * (env_ - tmp) + tmp;
    }
    // out = comp_.Process(osc_.Process(), env_fol_.Process(out));
    // out = comp_.Process(env_fol_.Process(out)) * osc_.Process();

    tmp = env_ * sq_wv_;
    return out_lpf_.Process(tmp);
}
