#include "daisy_petal.h"
#include "daisysp.h"
#include "level.h"

using namespace daisy;
using namespace daisysp;

// Declare a local daisy_petal for hardware access
DaisyPetal hw;

Parameter vtime, vfreq, vsend, vattack, vsens, vol;
float buffer[25];
int buffer_idx;
float drysend, wetsend;
bool bypass_autoswell, bypass_verb, gate;
ReverbSc verb;
Adsr autoswell_env;
Level vol_meter;

// This runs at a fixed rate, to prepare audio samples
void callback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
    float dryl, dryr, wetl, wetr, sendl, sendr, sig_abs, sig_max, thresh, out_vol;
    hw.ProcessAllControls();
    sig_max = 0.0f;

    autoswell_env.SetTime(ADSR_SEG_ATTACK, vattack.Process());
    thresh = vsens.Process();

    verb.SetFeedback(vtime.Process());
    verb.SetLpFreq(vfreq.Process());
    vsend.Process(); // Process Send to use later
    out_vol = vol.Process();

    if(hw.switches[DaisyPetal::SW_1].RisingEdge())
        bypass_autoswell = !bypass_autoswell;
    if(hw.switches[DaisyPetal::SW_2].RisingEdge())
        bypass_verb = !bypass_verb;

    drysend = 1.0f;
    if (!bypass_autoswell)
        drysend = autoswell_env.Process(gate);
    wetsend = vsend.Value();

    for(size_t i = 0; i < size; i++) {
        dryl  = drysend * in[0][i];
        dryr  = drysend * in[1][i];
        sendl = dryl * wetsend;
        sendr = dryr * wetsend;
        verb.Process(sendl, sendr, &wetl, &wetr);
        if(bypass_verb) {
            out[0][i] = out_vol * dryl; // left
            out[1][i] = out_vol * dryr; // right
        }
        else {
            out[0][i] = out_vol * (dryl + wetl);
            out[1][i] = out_vol * (dryr + wetr);
        }
        sig_abs = fabs(in[0][i]);
        if (sig_abs > sig_max) {
            sig_max = sig_abs;
        }
    }

    // buffer[buffer_idx] = sig_max;
    // float tmp;
    // for (int i = 0; i < 25; i++) {
    //     tmp = buffer[i];
    //     if (tmp > sig_max) {
    //         sig_max = tmp;
    //     }
    // }
    // sig_max = 20.f * fastlog10f(sig_max); 
    sig_max = vol_meter.ProcessBlockMax(sig_max);
    gate = sig_max >= thresh;
}

int main(void)
{
    hw.Init();
    float samplerate = hw.AudioSampleRate();

    // Init controls.
    // autoswell controls.
    vattack.Init(hw.knob[hw.KNOB_1], 0.01f, 0.1f, Parameter::LOGARITHMIC);
    vsens.Init(hw.knob[hw.KNOB_2], -60.0f, 0.0f, Parameter::LINEAR);
    vol.Init(hw.knob[hw.KNOB_6], 0.0f, 1.0f, Parameter::LOGARITHMIC);
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

    vol_meter.Init(samplerate, hw.AudioBlockSize());

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