#include "AudioModule.h"

#include <AL/al.h>
#include <AL/alc.h>
#include <stdexcept>

namespace AudioModule
{
void AudioModule::startup()
{
    LT_PROFILE_SCOPE("AudioModule::startup");
    m_device = alcOpenDevice(nullptr);
    if (!m_device)
    {
        throw std::runtime_error("Failed to open OpenAL device.");
    }

    LT_LOG(LogVerbosity::Info, "AudioModule", "Create OpenAL context");

    m_context = alcCreateContext(m_device, nullptr);
    if (!m_context)
    {
        alcCloseDevice(m_device);
        throw std::runtime_error("Failed to create OpenAL context.");
    }

    alcMakeContextCurrent(m_context);
}

void AudioModule::shutdown()
{
    LT_PROFILE_SCOPE("AudioModule::shutdown");
    LT_LOG(LogVerbosity::Info, "AudioModule", "shutdown");

    if (m_context)
    {
        alcMakeContextCurrent(nullptr);
        alcDestroyContext(m_context);
        m_context = nullptr;
    }

    if (m_device)
    {
        alcCloseDevice(m_device);
        m_device = nullptr;
    }
}

void AudioModule::playSoundAsync()
{
    m_audioThread = std::thread(
        []()
        {
            LT_PROFILE_SCOPE("AudioModule::playSoundAsync");
            int sampleRate  = 44100;
            float duration  = 0.05f;
            float frequency = 50.f;
            int numSamples  = static_cast<int>(sampleRate * duration);
            short* samples  = new short[numSamples];

            for (int i = 0; i < numSamples; ++i)
            {
                float time  = i / static_cast<float>(sampleRate);
                float value = sin(2 * 3.14 * frequency * time);
                samples[i]  = static_cast<short>(value * 32767);
            }

            ALuint buffer;
            alGenBuffers(1, &buffer);
            alBufferData(buffer, AL_FORMAT_MONO16, samples, numSamples * sizeof(short), sampleRate);

            ALuint source;
            alGenSources(1, &source);
            alSourcei(source, AL_BUFFER, buffer);
            alSourcePlay(source);

            ALint state;
            alGetSourcei(source, AL_SOURCE_STATE, &state);
            while (state == AL_PLAYING)
            {
                alGetSourcei(source, AL_SOURCE_STATE, &state);
            }

            delete[] samples;
            alDeleteSources(1, &source);
            alDeleteBuffers(1, &buffer);
        });

    m_audioThread.detach();
}
} // namespace AudioModule
