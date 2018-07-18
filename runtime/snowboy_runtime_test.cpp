#include <iostream>
#include <sys/time.h>

#include "snowboy-detect.h"

int main(int argc, char *argv[]) {
    if (argc != 4) {
        std::cout << "usage: pv_snowboy_speed_test wav_path resource_path model_path" << std::endl;
        return 1;
    }

    const char *wav_path = argv[1];
    std::string resource_path = argv[2];
    std::string model_path = argv[3];

    FILE *wav = fopen(wav_path, "rb");
    if (!wav) {
        std::cout << "failed to open wav file located at " << argv[1] << std::endl;
        return 1;
    }

    static const int WAV_HEADER_SIZE_BYTES = 44;

    if (fseek(wav, WAV_HEADER_SIZE_BYTES, SEEK_SET) != 0) {
        std::cout << "failed to skip the wav header";
        return 1;
    }

    const size_t frame_length = 512;

    auto *pcm = (int16_t*) malloc(sizeof(int16_t) * frame_length);
    if (!pcm) {
        std::cout << "failed to allocate memory for audio buffer" << std::endl;
        return 1;
    }

    snowboy::SnowboyDetect detector(resource_path, model_path);
    detector.SetSensitivity(std::string("0.5"));
    detector.SetAudioGain(1.0);

    static const int SAMPLE_RATE = 16000;

    double total_cpu_time_usec = 0;
    double total_processed_time_usec = 0;


    while(fread(pcm, sizeof(int16_t), frame_length, wav) == frame_length) {
        struct timeval before, after;
        gettimeofday(&before, NULL);

        detector.RunDetection(pcm, frame_length);

        gettimeofday(&after, NULL);

        total_cpu_time_usec += (after.tv_sec - before.tv_sec) * 1e6 + (after.tv_usec - before.tv_usec);
        total_processed_time_usec += (frame_length * 1e6) / SAMPLE_RATE;
    }

    const double real_time_factor = total_processed_time_usec / total_cpu_time_usec;
    printf("real time factor is: %f\n", real_time_factor);

    fclose(wav);

    return 0;
}
