#include "daisy_petal.h"
#include "daisysp.h"
#include "KiBoost.h"

using namespace daisy;
using namespace daisysp;

// Declare a local daisy_petal for hardware access
DaisyPetal hw;

KiBoost booster;

Parameter level;
bool bypass_booster, bright_flag, boost_flag;

// This runs at a fixed rate, to prepare audio samples
void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
    bool tmp_boost, tmp_bright;
    hw.ProcessDigitalControls();

    tmp_boost = hw.switches[DaisyPetal::SW_5].Pressed();
    tmp_bright = hw.switches[DaisyPetal::SW_6].Pressed();
    if(tmp_boost != boost_flag) {
        boost_flag = tmp_boost;
        booster.SetBoost(boost_flag);
    }
    if(tmp_bright != bright_flag) {
        bright_flag = tmp_bright;
        booster.SetBright(bright_flag);
    }
    booster.SetLevel(level.Process());

    if(hw.switches[DaisyPetal::SW_1].RisingEdge())
        bypass_booster = !bypass_booster;

    for(size_t i = 0; i < size; i += 2) {
        if(bypass_booster) {
            out[0][i] = in[0][i];     // left
            out[1][i] = in[1][i]; // right
        }
        else {
            out[0][i] = out[1][i] = booster.Process(in[0][i]);
        }
    }
}

int main(void)
{
    hw.Init();
    float samplerate = hw.AudioSampleRate();

    // Init controls.
    // booster controls
    level.Init(hw.knob[hw.KNOB_3], 0.01f, 10.0f, Parameter::LOGARITHMIC);

    // Init fx.
    booster.Init(samplerate);

    hw.StartAdc();
    hw.StartAudio(AudioCallback);
    while(1)
    {
        // Do Stuff InfInitely Here
        System::Delay(10);
        hw.ClearLeds();
        hw.SetFootswitchLed(hw.FOOTSWITCH_LED_1, bypass_booster ? 0.0f : 1.0f);
        hw.UpdateLeds();
    }
}
