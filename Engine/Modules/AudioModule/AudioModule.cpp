#include "AudioModule.h"

#include <AL/al.h>
#include <AL/alc.h>
#include <stdexcept>

#include "../LoggerModule/Logger.h"

namespace AudioModule
{
    void AudioModule::startup(const ModuleRegistry& moduleRegistry)
    {
        m_logger = std::dynamic_pointer_cast<Logger::Logger>(moduleRegistry.getModule("Logger"));


        m_logger->log(Logger::LogVerbosity::Info, "Startup", "AudioModule");

        m_logger->log(Logger::LogVerbosity::Info, "Open OpenAL device", "AudioModule");

        m_device = alcOpenDevice(nullptr);
        if (!m_device)
        {
            throw std::runtime_error("Failed to open OpenAL device.");
        }

        m_logger->log(Logger::LogVerbosity::Info, "Create OpenAL context", "AudioModule");

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
        m_logger->log(Logger::LogVerbosity::Info, "Shutdown", "AudioModule");

        m_logger->log(Logger::LogVerbosity::Info, "Destroy OpenAL context", "AudioModule");

        if (m_context)
        {
            alcMakeContextCurrent(nullptr);
            alcDestroyContext(m_context);
            m_context = nullptr;
        }

        m_logger->log(Logger::LogVerbosity::Info, "Close OpenAL device", "AudioModule");

        if (m_device)
        {
            alcCloseDevice(m_device);
            m_device = nullptr;
        }
    }

    void AudioModule::playSoundAsync()
    {
        m_audioThread = std::thread([]()
        {
            int sampleRate = 44100;
            float duration = 0.05f;
            float frequency = 50.f;
            int numSamples = static_cast<int>(sampleRate * duration);
            short* samples = new short[numSamples];

            for (int i = 0; i < numSamples; ++i)
            {
                float time = i / static_cast<float>(sampleRate);
                float value = sin(2 * 3.14 * frequency * time);
                samples[i] = static_cast<short>(value * 32767);
            }

            ALuint buffer;
            alGenBuffers(1, &buffer);
            alBufferData(buffer, AL_FORMAT_MONO16, samples,
                         numSamples * sizeof(short), sampleRate);

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
}
