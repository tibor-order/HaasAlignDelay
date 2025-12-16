#pragma once

#include <vector>
#include <cmath>

namespace DSP
{

/**
 * @brief A simple delay line with linear interpolation
 *
 * This class provides a circular buffer implementation for audio delay
 * with sub-sample accuracy via linear interpolation.
 */
class DelayLine
{
public:
    DelayLine() = default;
    ~DelayLine() = default;

    /**
     * @brief Prepare the delay line for processing
     * @param sampleRate The sample rate in Hz
     * @param maxDelayMs Maximum delay time in milliseconds
     */
    void prepare(double sampleRate, float maxDelayMs)
    {
        currentSampleRate = sampleRate;
        bufferSize = static_cast<int>(sampleRate * maxDelayMs * 0.001) + 1;
        buffer.resize(static_cast<size_t>(bufferSize), 0.0f);
        writePosition = 0;
    }

    /**
     * @brief Clear the delay buffer
     */
    void clear()
    {
        std::fill(buffer.begin(), buffer.end(), 0.0f);
        writePosition = 0;
    }

    /**
     * @brief Release resources
     */
    void release()
    {
        buffer.clear();
        bufferSize = 0;
        writePosition = 0;
    }

    /**
     * @brief Push a sample into the delay line
     * @param sample The input sample
     */
    void push(float sample)
    {
        if (bufferSize > 0)
        {
            buffer[static_cast<size_t>(writePosition)] = sample;
            writePosition = (writePosition + 1) % bufferSize;
        }
    }

    /**
     * @brief Read a delayed sample with linear interpolation
     * @param delayMs Delay time in milliseconds
     * @return The interpolated delayed sample
     */
    float read(float delayMs) const
    {
        if (bufferSize <= 0)
            return 0.0f;

        float delaySamples = delayMs * 0.001f * static_cast<float>(currentSampleRate);

        // Calculate read position (behind write position)
        float readPos = static_cast<float>(writePosition) - delaySamples - 1.0f;
        if (readPos < 0.0f)
            readPos += static_cast<float>(bufferSize);

        // Linear interpolation
        int readPosInt = static_cast<int>(readPos);
        float frac = readPos - static_cast<float>(readPosInt);

        int nextPos = (readPosInt + 1) % bufferSize;
        readPosInt = readPosInt % bufferSize;

        if (readPosInt < 0) readPosInt += bufferSize;
        if (nextPos < 0) nextPos += bufferSize;

        return buffer[static_cast<size_t>(readPosInt)] * (1.0f - frac) +
               buffer[static_cast<size_t>(nextPos)] * frac;
    }

    /**
     * @brief Process a single sample through the delay line
     * @param inputSample The input sample
     * @param delayMs Delay time in milliseconds
     * @return The delayed sample
     */
    float process(float inputSample, float delayMs)
    {
        float output = read(delayMs);
        push(inputSample);
        return output;
    }

    /**
     * @brief Convert milliseconds to samples at current sample rate
     * @param ms Time in milliseconds
     * @return Number of samples
     */
    float msToSamples(float ms) const
    {
        return ms * 0.001f * static_cast<float>(currentSampleRate);
    }

    /**
     * @brief Get the current sample rate
     * @return Sample rate in Hz
     */
    double getSampleRate() const { return currentSampleRate; }

    /**
     * @brief Check if the delay line is prepared
     * @return true if prepared and ready for processing
     */
    bool isPrepared() const { return bufferSize > 0; }

private:
    std::vector<float> buffer;
    int bufferSize = 0;
    int writePosition = 0;
    double currentSampleRate = 44100.0;
};

} // namespace DSP
