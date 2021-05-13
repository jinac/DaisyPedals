#include "KiBoost.h"
#include "daisysp.h"
#include <math.h>

using namespace daisysp;

void KiBoost::Init(float samplerate) {
    sample_rate_ = samplerate;

    lpf_.Init(samplerate);
    lpf_.SetFreq(freq_cutoff_lpf_);
    hpf_.Init(samplerate);
    hpf_.SetFreq(freq_cutoff_hpf_);

    boost_ = false;
    bright_ = false;
}

void KiBoost::SetBright(bool flag) {
    bright_ = flag;
    if (bright_) {
        lpf_.SetFreq(freq_cutoff_lpf_bright_);
    } else {
        lpf_.SetFreq(freq_cutoff_lpf_);
    }
}

void KiBoost::SetBoost(bool flag) {
    boost_ = flag;
}

void KiBoost::SetLevel(float level) {
    gain_ = level;
    if (boost_) {
        gain_ += 0.7523391963649502;
    }
}

float KiBoost::Process(float &in) {
    float low_signal, high_signal, out;

    low_signal = hpf_.Process(in);
    high_signal = lpf_.Process(low_signal);
    out = gain_ * high_signal;

    return out;
}
