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
static FIR<FIRFILTER_USER_MEMORY> flt;
// FIR<1024, 48> flt;

Parameter volume;

// static constexpr size_t kMaxFiles   = 8;
// static constexpr size_t kBufferSize = 512;

float flt_buffer[1024];
float coeff_arr[1024] = {0};

float samplerate;
bool bypass;
char TEST_FILE_NAME[] = "07_Ampeg_SVT-810E_by_Shift_Line.wav";


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

    // output = s162f(sampler.Stream()) * 0.5f;
    for(size_t i = 0; i < size; i += 2) {
        // out[i] = out[i + 1] = in[i];
        if (bypass) {
            out[i] = out[i + 1] = volume.Value() * in[i];
        } else {
            out[i] = out[i + 1] = volume.Value() * flt.Process(in[i]);
        }
    }
}

//TODO: fix the interpolation from mismatching sample rates
void LoadIRFromWavFile(const WavInfo *file_info) {
    size_t bits_per_sample = file_info->raw_data.BitPerSample;
    // size_t num_channels = file_info->raw_data.NbrChannels;
    size_t data_size = file_info->raw_data.SubCHunk2Size;
    size_t num_samples = data_size / bits_per_sample;
    int byte_chunk_size = bits_per_sample / 8;
    int wav_sample_rate = file_info->raw_data.SampleRate;

 
    // if (wav_sample_rate - samplerate < 1.0f) {
    //     match = true;
    // }
    // if (num_samples == 1000)
    //     match = true;
    // if (wav_sample_rate == 48000) {
    //     hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);
    // }

    // unsigned char data[byte_chunk_size];
    // float new_floatt;
    // int new_int;
    // float tmp_int = 0;
    // unsigned char *data = (unsigned char *) &tmp_int;
    // char data[4] = {0};
    // float tmp_int = (* (float*) data);
    // float tmp = 0.;

    // int temp;
    // unsigned char *ptr = (unsigned char *)&temp;
    int temp;
    unsigned char bytes[3];
    size_t bytesread;
    for (int i = 0; i < (int)num_samples; i++) {
        temp = 0;
        // switch (bits_per_sample) {
        //     case 8:
        //         f_read(&SDFile, &data, byte_chunk_size, &bytesread);
        //         tmp = tmp_int / 128.0f;
        //         break;
        //     case 16:
        //         f_read(&SDFile, &data, byte_chunk_size, &bytesread);
        //         tmp = tmp_int / 32768.0f;
        //         break;
        //     case 24:
        //         f_read(&SDFile, &data[1], byte_chunk_size, &bytesread);
        //         tmp = tmp_int / 2147483648.0f;
        //         break;
        //     case 32:
        //         f_read(&SDFile, &data, byte_chunk_size, &bytesread);
        //         tmp = tmp_int;
        //         break;
        // }d
        // https://stackoverflow.com/questions/35876000/converting-24-bit-integer-2s-complement-to-32-bit-integer-in-c

        // https://github.com/erwincoumans/DaisyCloudSeed/blob/0.43/patch/SamplePlayer/b3ReadWavFile.cpp
        // f_read(&SDFile, ptr + 1, byte_chunk_size, &bytesread);
        f_read(&SDFile, bytes, byte_chunk_size, &bytesread);
        // temp &= 0xffffff00;
        temp = bytes[2] << 16 | bytes[1] << 8 | bytes[0];
        if (temp & 0x800000) //  if the 24th bit is set, this is a negative number in 24-bit world
            temp = temp | ~0xFFFFFF; // so make sure sign is extended to the 32 bit float
        coeff_arr[i] = (float) temp / 8388608.;
        // coeff_arr[i] = s242f(temp);
    }
    flt.SetStateBuffer(flt_buffer, num_samples + 1);
    flt.SetIR(coeff_arr, num_samples, false);
    flt.Reset();
    // match = true;
}

void LoadIR() {
    WavInfo file_info;
    strcpy(file_info.name, TEST_FILE_NAME);
    // file_info.name = "07_Ampeg_SVT-810E_by_Shift_Line.wav";
    size_t bytesread;
    if(f_open(&SDFile, file_info.name, (FA_OPEN_EXISTING | FA_READ)) == FR_OK) {
    // if(f_open(&SDFile, TEST_FILE_NAME, (FA_OPEN_EXISTING | FA_READ)) == FR_OK) {
        // Get WAV Info of TEST_FILE
        if(f_read(&SDFile, (void *)&file_info.raw_data,
                  sizeof(WAV_FormatTypeDef), &bytesread) == FR_OK) {

            // Load WAV to filter.
            LoadIRFromWavFile(&file_info);

        }
        f_close(&SDFile);
    }
}


int main(void) {
    // initialize petal hardware
    // Init daisy.
    hw.Init();
    samplerate = hw.AudioSampleRate();

    // Init SDMMC and load wav.
    coeff_arr[0] = 1;
    flt.SetStateBuffer(flt_buffer, 2);
    flt.Init(coeff_arr, 1, false);

    SdmmcHandler::Config sd_cfg;
    sd_cfg.Defaults();
    sd.Init(sd_cfg);
    dsy_fatfs_init();
    f_mount(&SDFatFS, SDPath, 1);
    LoadIR();

    // Initialize knobs.
    volume.Init(hw.knob[hw.KNOB_3], 0.0f, 1.0f, Parameter::LINEAR);

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