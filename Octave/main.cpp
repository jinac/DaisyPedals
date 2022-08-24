#include "daisy_petal.h"
#include "daisysp.h"
#include "octave.h"

using namespace daisy;
using namespace daisysp;

// Declare a local daisy_petal for hardware access
DaisyPetal hw;

Octave octaver;
Decimator decimator;

Parameter vlevel, vblend, dcrush, drate;
bool bypass_octave, bypass_decimator;

// This runs at a fixed rate, to prepare audio samples
void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
    float level, blend, tmp, raw;
    hw.ProcessDigitalControls();

    level = vlevel.Process();
    blend = vblend.Process();
    decimator.SetDownsampleFactor(drate.Process());
    decimator.SetBitcrushFactor(dcrush.Process());

    if (hw.switches[DaisyPetal::SW_1].RisingEdge()) {
        bypass_octave = !bypass_octave;
    }
    if (hw.switches[DaisyPetal::SW_2].RisingEdge()) {
        bypass_decimator = !bypass_decimator;
    }

    // Process output.
    for (size_t i = 0; i < size; i++) {
        raw = in[0][i];
        tmp = raw;
        tmp = bypass_octave ? tmp : octaver.Process(tmp);
        tmp = bypass_decimator ? tmp : decimator.Process(tmp);

        out[0][i] = out[1][i] = level * (blend * tmp + (1 - blend) * raw) ;
    }
}

int main(void)
{
    hw.Init();
    hw.SetAudioBlockSize(4);
    float samplerate = hw.AudioSampleRate();

    // Init controls.
    vlevel.Init(hw.knob[hw.KNOB_3], 0.01f, 10.0f, Parameter::LOGARITHMIC);
    vblend.Init(hw.knob[hw.KNOB_4], 0.0f, 1.0f, Parameter::LINEAR);
    dcrush.Init(hw.knob[hw.KNOB_5], 0.0f, 1.0f, Parameter::EXPONENTIAL);
    drate.Init(hw.knob[hw.KNOB_6], 0.01f, 1.0f, Parameter::LINEAR);

    // Init fx.
    octaver.Init(samplerate);
    decimator.Init();
    decimator.SetBitcrushFactor(dcrush.Process());
    decimator.SetDownsampleFactor(drate.Process());
    bypass_octave = false;
    bypass_decimator = false;

    hw.StartAdc();
    hw.StartAudio(AudioCallback);
    while(1)
    {
        // Do Stuff InfInitely Here
        System::Delay(10);
        hw.ClearLeds();
        hw.SetFootswitchLed(hw.FOOTSWITCH_LED_1, bypass_octave ? 0.0f : 1.0f);
        hw.SetFootswitchLed(hw.FOOTSWITCH_LED_2, bypass_decimator ? 0.0f : 1.0f);
        hw.UpdateLeds();
    }
}