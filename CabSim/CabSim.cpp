// #include <stdio.h>
#include <string.h>
#include "daisysp.h"
#include "daisy_core.h"
#include "daisy_petal.h"
#include "fatfs.h"

using namespace daisysp;
using namespace daisy;

DaisyPetal hw;
SdmmcHandler sd; // Handler for SD Card Hardware
// static FIR<FIRFILTER_USER_MEMORY> flt;
FIR<1024, 48> flt;

Parameter volume;

// static constexpr size_t kMaxFiles   = 8;
// static constexpr size_t kBufferSize = 512;

float flt_buffer[1024 + 1]; /*< Impl-specific storage */

bool bypass;
char TEST_FILE_NAME[256] = "07_Ampeg_SVT-810E_by_Shift_Line.wav";


struct WavInfo
{
    WAV_FormatTypeDef raw_data;               /*< Raw wav data*/
    char              name[WAV_FILENAME_MAX]; /**< Wav filename */
};

void AudioCallback(float *in, float *out, size_t size) {
    // float output;
    hw.ProcessDigitalControls();
    volume.Process();
 
    if(hw.switches[DaisyPetal::SW_1].RisingEdge())
        bypass = !bypass;

    // output = volume.Value() * flt.Process(in[i]);
    // output = volume.Value() * s242f(sampler.Stream());

    // for(size_t i = 0; i < size; i += 2) {
    //     if (bypass) {
    //         out[i] = in[i];
    //         out[i + 1] = in[i + 1];
    //     } else {
    //         out[i] = out[i + 1] = flt.Process(in[i]);
    //     }
    // }

    // output = s162f(sampler.Stream()) * 0.5f;
    for(size_t i = 0; i < size; i += 2) {
        out[i] = out[i + 1] = volume.Value() * flt.Process(in[i]);
    }
}

//TODO: fix the interpolation from mismatching sample rates
void LoadIRFromWavFile(const WavInfo *file_info) {
    float coeff_arr[1024];
    size_t bits_per_sample = file_info->raw_data.BitPerSample;
    // size_t num_channels = file_info->raw_data.NbrChannels;
    size_t data_size = file_info->raw_data.SubCHunk2Size;
    size_t num_samples = data_size / bits_per_sample;
    int byte_chunk_size = bits_per_sample / 8;
    int wav_sample_rate = file_info->raw_data.SampleRate;

    // if (wav_sample_rate == 48000) {
    //     hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);
    // }

    float tmp_int;
    unsigned char *data = (unsigned char *) &tmp_int;
    // char data[4] = {0};
    // float tmp_int = (* (float*) data);
    float tmp = 0.;
    size_t bytesread;
    for (int i = 0; i < (int)num_samples; i++) {
        switch (bits_per_sample) {
            case 8:
                f_read(&SDFile, &data, byte_chunk_size, &bytesread);
                tmp = tmp_int / 128.0f;
                break;
            case 16:
                f_read(&SDFile, &data, byte_chunk_size, &bytesread);
                tmp = tmp_int / 32768.0f;
                break;
            case 24:
                f_read(&SDFile, &data[1], byte_chunk_size, &bytesread);
                tmp = tmp_int / 2147483648.0f;
                break;
            case 32:
                f_read(&SDFile, &data, byte_chunk_size, &bytesread);
                tmp = tmp_int;
                break;
        }
        coeff_arr[i] = tmp;
    }
    flt.SetIR(coeff_arr, num_samples, false);
}

void LoadIR() {
    WavInfo file_info;
    // file_info.name = TEST_FILE_NAME;
    size_t bytesread;
    // if(f_open(&SDFile, file_info.name, (FA_OPEN_EXISTING | FA_READ)) == FR_OK) {
    if(f_open(&SDFile, TEST_FILE_NAME, (FA_OPEN_EXISTING | FA_READ)) == FR_OK) {
        // Get WAV Info of TEST_FILE
        if(f_read(&SDFile, (void *)&file_info.raw_data,
                  sizeof(WAV_FormatTypeDef), &bytesread) != FR_OK) {

            // Load WAV to filter.
            LoadIRFromWavFile(&file_info);

        }
        f_close(&SDFile);
    }
}


int main(void) {
    // initialize petal hardware
    // int sample_rate = 24000;
    // size_t blocksize = 48;
    // hw.AudioBlockSize()

    // Init daisy.
    hw.Init();
    // float sample_rate = hw.AudioSampleRate();

    // Init SDMMC and load wav.
    // flt.SetStateBuffer(flt_buffer, num_samples + 1);
    // flt.Init(coeff_arr, num_samples, false);

    SdmmcHandler::Config sd_cfg;
    sd_cfg.Defaults();
    sd.Init(sd_cfg);
    dsy_fatfs_init();
    f_mount(&SDFatFS, SDPath, 1);
    LoadIR();

    // Initialize knobs.
    volume.Init(hw.knob[hw.KNOB_3], 0.0f, 1.0f, Parameter::LOGARITHMIC);

    // Initialize fx.

    // Start pedal fx.
    hw.StartAdc();
    hw.StartAudio(AudioCallback);

    // Pedal fx update loop.
    while(1) {
        // sampler.Prepare();
        System::Delay(10);
        hw.ClearLeds();
        hw.SetFootswitchLed(hw.FOOTSWITCH_LED_1, bypass ? 0.0f : 1.0f);
        hw.UpdateLeds();
    }
}