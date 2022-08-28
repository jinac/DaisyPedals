#include "daisy_petal.h"
#include "daisysp.h"

#define MAX_DELAY static_cast<size_t>(48000 * 10.f)
#define LPG_FREQ_MIN 120.0f

using namespace daisy;
using namespace daisysp;

DaisyPetal hw;

DelayLine<float, MAX_DELAY> DSY_SDRAM_BSS delay;
Tone envlpf;

Parameter cvolume, cmix, cdamp, crate, cdepth, clpg;

float env, atk_coeff, rel_coeff, envlpffreq, feedback;
float currentDelayTime, delayTarget;
int cyclesToUpdate;
bool bypass_fx;

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
	hw.ProcessAllControls();

	// Update parameters.
	float volume, mix, damp, rate, depth, lpg;
	float lpgfreq, samplerate, damp_coeff;
	float wet, dry;
	volume = cvolume.Process();
	mix = cmix.Process();
	lpg = clpg.Process();
	damp = cdamp.Process();
	rate = crate.Process();
	depth = cdepth.Process();

	samplerate = hw.AudioSampleRate();

    if (hw.switches[DaisyPetal::SW_1].RisingEdge()) {
        bypass_fx = !bypass_fx;
    }

	for (size_t i = 0; i < size; i++)
	{
		dry = in[0][i];

		// Update LPG.
		if (fabs(dry) > env) {
			env = atk_coeff * (env - dry) + dry;
		} else {
			env = rel_coeff * (env - dry) + dry;
		}
		lpgfreq = LPG_FREQ_MIN + env * lpg;
		envlpf.SetFreq(lpgfreq);

		// Update modulating signal.
		cyclesToUpdate--;
		if (cyclesToUpdate <= 0) {
			delayTarget = (1.0 + depth * (rand() * kRandFrac - 0.5)) * 0.036f * samplerate;
			// delayTarget = 0.036f * samplerate;
			cyclesToUpdate = static_cast<int>(ceil(rate * samplerate));
		}

		// Process delay.
		damp_coeff = fclamp(1.0 / (damp * samplerate), 0.00001f, 1.0f);
		fonepole(currentDelayTime, delayTarget, damp_coeff);
		delay.SetDelay(currentDelayTime);
		wet = delay.Read();
		delay.Write(dry);

		if (!bypass_fx){
			dry = (1.0f - mix) * dry;
			wet = mix * envlpf.Process(wet);
			out[0][i] = out[1][i] = volume * (wet + dry);
		} else {
			out[0][i] = out[1][i] = dry;
		}
	}
}

int main(void)
{
	hw.Init();
	hw.SetAudioBlockSize(4); // number of samples handled per callback
	hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);
    float samplerate = hw.AudioSampleRate();

    // Init controls.
    cvolume.Init(hw.knob[hw.KNOB_1], 0.0f, 1.0f, Parameter::LOGARITHMIC);
    cmix.Init(hw.knob[hw.KNOB_2], 0.0f, 1.0f, Parameter::LINEAR);
    cdamp.Init(hw.knob[hw.KNOB_3], 0.001f, 0.1f, Parameter::LINEAR);
    crate.Init(hw.knob[hw.KNOB_4], 0.0f, 0.08f, Parameter::LINEAR);
    cdepth.Init(hw.knob[hw.KNOB_5], 0.0f, 1.0f, Parameter::LINEAR);
    clpg.Init(hw.knob[hw.KNOB_6], 0.0f, 5000.0f, Parameter::LOGARITHMIC);

	// Init fx.
	delayTarget = (1.0 + cdepth.Process() * (rand() * kRandFrac - 0.5)) * 0.036f * samplerate;
	currentDelayTime = delayTarget;
	delay.Init();
	delay.SetDelay(currentDelayTime);
	envlpf.Init(samplerate);
	float lpgfreq = LPG_FREQ_MIN + env * clpg.Process();
	feedback = 0.0f;
	envlpf.SetFreq(lpgfreq);
	bypass_fx = false;


	hw.StartAdc();
	hw.StartAudio(AudioCallback);
	while(1) {
		// hw.SetFootswitchLed(hw.FOOTSWITCH_LED_1, bypass_fx ? 0.0f : 1.0f);
	}
}
