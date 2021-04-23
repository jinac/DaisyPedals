#include "daisy_petal.h"
#include "daisysp.h"
// #include "swell.h"

using namespace daisy;
using namespace daisysp;

// Declare a local daisy_petal for hardware access
DaisyPetal hw;

Parameter vtime, vfreq, vsend, vattack, vsens, vol;
float buffer[25];
int buffer_idx;
float ssend;
bool bypass_autoswell, bypass_verb, gate;
ReverbSc verb;
Adsr autoswell_env;

// This runs at a fixed rate, to prepare audio samples
void callback(float *in, float *out, size_t size)
{
    float dryl, dryr, wetl, wetr, sendl, sendr, sig_abs, sig_max, thresh;
    hw.ProcessDigitalControls();
    sig_max = 0.0f;

    autoswell_env.SetTime(ADSR_SEG_ATTACK, vattack.Process());
    thresh = vsens.Process();
    // ssend = vol.Process();

    verb.SetFeedback(vtime.Process());
    verb.SetLpFreq(vfreq.Process());
    vsend.Process(); // Process Send to use later

    if(hw.switches[DaisyPetal::SW_1].RisingEdge())
        bypass_autoswell = !bypass_autoswell;
    if(hw.switches[DaisyPetal::SW_2].RisingEdge())
        bypass_verb = !bypass_verb;

    ssend = 1.0f;
    if (!bypass_autoswell)
        ssend = autoswell_env.Process(gate);

    for(size_t i = 0; i < size; i += 2) {
        dryl  = ssend * in[i];
        dryr  = ssend * in[i + 1];
        sendl = dryl * vsend.Value();
        sendr = dryr * vsend.Value();
        verb.Process(sendl, sendr, &wetl, &wetr);
        if(bypass_verb) {
            out[i]     = dryl;     // left
            out[i + 1] = dryr; // right
        }
        else {
            out[i]     = dryl + wetl;
            out[i + 1] = dryr + wetr;
        }
        sig_abs = fabs(in[i]);
        if (sig_abs > sig_max) {
            sig_max = sig_abs;
        }
    }

    buffer[buffer_idx] = sig_max;
    float tmp;
    for (int i = 0; i < 25; i++) {
        tmp = buffer[i];
        if (tmp > sig_max) {
            sig_max = tmp;
        }
    }
    sig_max = 20.f * fastlog10f(sig_max); 
    if (sig_max > thresh) {
        gate = true;
    } else {
        gate = false;
    }
}

int main(void)
{
    hw.Init();
    float samplerate = hw.AudioSampleRate();

    // Init controls.
    // autoswell controls.
    vattack.Init(hw.knob[hw.KNOB_1], 0.01f, 0.1f, Parameter::LOGARITHMIC);
    vsens.Init(hw.knob[hw.KNOB_2], -60.0f, 0.0f, Parameter::LINEAR);
    vol.Init(hw.expression, 0.0f, 1.0f, Parameter::LINEAR);
    for (size_t i = 0; i < 25; i++) {
        buffer[i] = 0;
    }
    buffer_idx = 0;

    // verb controls.
    vtime.Init(hw.knob[hw.KNOB_3], 0.6f, 0.999f, Parameter::LOGARITHMIC);
    vfreq.Init(hw.knob[hw.KNOB_4], 500.0f, 20000.0f, Parameter::LOGARITHMIC);
    vsend.Init(hw.knob[hw.KNOB_5], 0.0f, 1.0f, Parameter::LINEAR);

    // Init fx.
    verb.Init(samplerate);

    autoswell_env.Init(samplerate);
    autoswell_env.SetTime(ADSR_SEG_ATTACK, vattack.Process());
    autoswell_env.SetTime(ADSR_SEG_DECAY, 0.0);
    autoswell_env.SetTime(ADSR_SEG_RELEASE, 0.01);
    autoswell_env.SetSustainLevel(1.0f);

    hw.StartAdc();
    hw.StartAudio(callback);
    while(1)
    {
        // Do Stuff InfInitely Here
        System::Delay(10);
        hw.ClearLeds();
        hw.SetFootswitchLed(hw.FOOTSWITCH_LED_1, bypass_autoswell ? 0.0f : 1.0f);
        hw.SetFootswitchLed(hw.FOOTSWITCH_LED_2, bypass_verb ? 0.0f : 1.0f);
        hw.SetFootswitchLed(hw.FOOTSWITCH_LED_3, !gate ? 0.0f : 1.0f);
        hw.UpdateLeds();
    }
}