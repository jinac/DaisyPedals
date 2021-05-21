#include "daisy_petal.h"
#include "daisysp.h"
#include "octave.h"

using namespace daisy;
using namespace daisysp;

// Declare a local daisy_petal for hardware access
DaisyPetal hw;

DcBlock block;
Octave octaver;

Parameter vlevel, vblend;
bool bypass_octave;

// This runs at a fixed rate, to prepare audio samples
void callback(float *in, float *out, size_t size)
{
    float level, blend, intermediate;
    hw.ProcessDigitalControls();

    level = vlevel.Process();
    blend = vblend.Process();

    if(hw.switches[DaisyPetal::SW_1].RisingEdge())
        bypass_octave = !bypass_octave;

    // Process output.
    for (size_t i = 0; i < size; i += 2) {
        // intermediate = block.Process(in[i]);
        intermediate = in[i];
        if(bypass_octave) {
            out[i]     = level * intermediate;     // left
            out[i + 1] = level * in[i + 1]; // right
        }
        else {
            // dry = blend * intermediate;
            // oct = (1 - blend) * octaver.Process(intermediate);
            out[i] = out[i + 1] = level * octaver.Process(intermediate);
            // out[i] = out[i + 1] = level * fabs(in[i]);
        }
    }
}

int main(void)
{
    hw.Init();
    float samplerate = hw.AudioSampleRate();

    // Init controls.
    // booster controls
    vlevel.Init(hw.knob[hw.KNOB_3], 0.01f, 10.0f, Parameter::LOGARITHMIC);
    vblend.Init(hw.knob[hw.KNOB_4], 0.0f, 1.0f, Parameter::LINEAR);

    // Init fx.
    octaver.Init(samplerate);
    bypass_octave = false;

    block.Init(samplerate);

    hw.StartAdc();
    hw.StartAudio(callback);
    while(1)
    {
        // Do Stuff InfInitely Here
        System::Delay(10);
        hw.ClearLeds();
        hw.SetFootswitchLed(hw.FOOTSWITCH_LED_1, bypass_octave ? 0.0f : 1.0f);
        hw.UpdateLeds();
    }
}