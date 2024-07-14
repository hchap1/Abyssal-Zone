#include <iostream>
#include <vector>
#include "OPENAL/al.h"
#include "OPENAL/alc.h"
#define DR_WAV_IMPLEMENTATION
#include "OPENAL/dr_wav.h"

std::vector<drwav_int16> load_wav(const std::string& filename,
    unsigned int& channels,
    unsigned int& sampleRate, // Changed to unsigned int
    uint8_t& bitsPerSample,
    ALsizei& size)
{
    drwav_uint64 totalFrames;
    drwav_int16* pWavData = drwav_open_file_and_read_pcm_frames_s16(filename.c_str(), &channels, &sampleRate, &totalFrames, NULL);
    if (pWavData == NULL) {
        std::cerr << "ERROR: Could not open \"" << filename << "\"" << std::endl;
        return {};
    }

    size = static_cast<ALsizei>(totalFrames * channels * sizeof(drwav_int16));

    // Convert data to char array
    std::vector<drwav_int16> data(size);
    memcpy(data.data(), pWavData, size);

    drwav_free(pWavData, nullptr);

    return data;
}

class Sound {
public:
    Sound(const std::string& filepath) {
        unsigned int channels;
        unsigned int sampleRate; // Changed to unsigned int
        uint8_t bitsPerSample = 16;
        ALsizei size;
        std::vector<drwav_int16> soundData = load_wav(filepath, channels, sampleRate, bitsPerSample, size);

        alGenBuffers(1, &buffer);

        ALenum format;
        std::cout << "C: " << channels << " BPS: " << bitsPerSample << std::endl;
        format = AL_FORMAT_MONO16;
        alBufferData(buffer, format, soundData.data(), size, sampleRate);
    }

    void play() {
        ALuint source;
        alGenSources(1, &source);
        //alSourcef(source, AL_PITCH, 1);
        //alSourcef(source, AL_GAIN, 1.0f);
        //alSource3f(source, AL_POSITION, 0, 0, 0);
        //alSource3f(source, AL_VELOCITY, 0, 0, 0);
        //alSourcei(source, AL_LOOPING, AL_FALSE);
        alSourcei(source, AL_BUFFER, buffer);
        alSourcePlay(source);
    }

private:
    ALuint buffer;
};

class AudioDevice {
public:
    AudioDevice() {
        device = alcOpenDevice(nullptr);
        context = alcCreateContext(device, nullptr);
        alcMakeContextCurrent(context);
    }

    void playSound(Sound* sound) {
        sound->play();
    }

private:
    ALCdevice* device;
    ALCcontext* context;
};
