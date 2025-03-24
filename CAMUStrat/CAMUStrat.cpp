#include "daisy_petal.h"
#include "daisysp.h"

using namespace daisy;
using namespace daisysp;

DaisyPetal hw;
Compressor comp;
ReverbSc verb;
Parameter cThreshold, cAttack, cRelease;
Parameter vTime, vFreq, vSend;
bool cBypass, vBypass;
float drysend, wetsend;
float dryl, dryr, wetl, wetr, sendl, sendr, out_vol;

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size) {
	hw.ProcessAllControls();

	comp.SetThreshold(cThreshold.Process());
	comp.SetAttack(cAttack.Process());
	comp.SetRelease(cRelease.Process());
	if (hw.switches[DaisyPetal::SW_1].RisingEdge()) {
		cBypass = !cBypass;
	}

    verb.SetFeedback(vTime.Process());
    verb.SetLpFreq(vFreq.Process());
	vSend.Process();
	if (hw.switches[DaisyPetal::SW_2].RisingEdge()) {
		vBypass = !vBypass;
	}

	wetsend = vSend.Value();
	for (size_t i = 0; i < size; i++) {
        dryl  = in[0][i];
        dryr  = in[1][i];

		if (!cBypass) {
			dryl = dryr = comp.Process(dryl);
		}

        sendl = dryl * wetsend;
        sendr = dryr * wetsend;
		verb.Process(sendl, sendr, &wetl, &wetr);

        if(vBypass) {
            out[0][i] = out_vol * dryl; // left
            out[1][i] = out_vol * dryr; // right
        } else {
            out[0][i] = out_vol * (dryl + wetl);
            out[1][i] = out_vol * (dryr + wetr);
        }

		// out[0][i] = comp.Process(in[0][i]);
		// out[1][i] = in[1][i];
	}
}

int main(void) {
	hw.Init();
	float samplerate = hw.AudioSampleRate();

	out_vol = 1.0f;

	// Setup compressor
	float cRatio = 3.0;
	cBypass = false;
	comp.Init(samplerate);
	comp.SetRatio(cRatio);
    cThreshold.Init(hw.knob[hw.KNOB_1], -80.0f, 0.0f, Parameter::LINEAR);
    cAttack.Init(hw.knob[hw.KNOB_2], 0.001f, 10.0f, Parameter::LOGARITHMIC);
    cRelease.Init(hw.knob[hw.KNOB_3], 0.001f, 10.0f, Parameter::LOGARITHMIC);

	// Setup reverb
	vBypass = false;
	verb.Init(samplerate);
    vTime.Init(hw.knob[hw.KNOB_4], 0.6f, 0.999f, Parameter::LOGARITHMIC);
    vFreq.Init(hw.knob[hw.KNOB_5], 500.0f, 20000.0f, Parameter::LOGARITHMIC);
    vSend.Init(hw.knob[hw.KNOB_6], 0.0f, 1.0f, Parameter::LINEAR);

	// hw.SetAudioBlockSize(4); // number of samples handled per callback
	// hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);
	hw.StartAdc();
	hw.StartAudio(AudioCallback);
	while(1) {
		hw.SetFootswitchLed(hw.FOOTSWITCH_LED_1, cBypass ? 0.0f : 1.0f);
		hw.SetFootswitchLed(hw.FOOTSWITCH_LED_2, vBypass ? 0.0f : 1.0f);
		hw.UpdateLeds();
		System::Delay(10);
	}
}
