#pragma once
#include <AL/al.h>
#include <AL/alc.h>
#include <memory>
#include <stdexcept>
#include <thread>

/// <summary>
/// Manages OpenAL audio operations, including device/context setup and sound
/// playback. Implements a singleton pattern to ensure a single instance.
/// </summary>
class AudioModule {
  ALCdevice *m_device = nullptr;   ///< OpenAL audio device handle.
  ALCcontext *m_context = nullptr; ///< OpenAL audio context.
  std::thread m_audioThread;       ///< Thread for playing audio asynchronously.

public:
  /// <summary>
  /// Retrieves the singleton instance of the AudioModule.
  /// </summary>
  /// <returns>Reference to the singleton AudioModule instance.</returns>
  static AudioModule &getInstance() {
    static AudioModule audioModule;
    return audioModule;
  }

  /// <summary>
  /// Initializes the OpenAL audio system by opening a device and creating a
  /// context.
  /// </summary>
  /// <exception cref="std::runtime_error">Thrown if the audio device or context
  /// cannot be initialized.</exception>
  void startup() {
    LOG_INFO("AudioModule: Startup");

    m_device = alcOpenDevice(nullptr);
    if (!m_device) {
      throw std::runtime_error("Failed to open OpenAL device.");
    }

    m_context = alcCreateContext(m_device, nullptr);
    if (!m_context) {
      alcCloseDevice(m_device);
      throw std::runtime_error("Failed to create OpenAL context.");
    }

    alcMakeContextCurrent(m_context);
  }

  /// <summary>
  /// Shuts down the OpenAL audio system by destroying the context and closing
  /// the device.
  /// </summary>
  void shutDown() {
    LOG_INFO("AudioModule: Shut down");

    if (m_context) {
      alcMakeContextCurrent(nullptr);
      alcDestroyContext(m_context);
      m_context = nullptr;
    }

    if (m_device) {
      alcCloseDevice(m_device);
      m_device = nullptr;
    }
  }

  /// <summary>
  /// Plays a generated sound asynchronously on a separate thread.
  /// </summary>
  void playSoundAsync() {
    m_audioThread = std::thread([]() {
      int sampleRate = 44100;
      float duration = 0.05f; // Duration of the sound in seconds.
      float frequency = 50.f; // Frequency of the generated tone.
      int numSamples = static_cast<int>(sampleRate * duration);
      short *samples = new short[numSamples];

      for (int i = 0; i < numSamples; ++i) {
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
      while (state == AL_PLAYING) {
        alGetSourcei(source, AL_SOURCE_STATE, &state);
      }

      delete[] samples;
      alDeleteSources(1, &source);
      alDeleteBuffers(1, &buffer);
    });

    m_audioThread.detach();
  }
};
