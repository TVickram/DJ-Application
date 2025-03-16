#include "DJAudioPlayer.h"

// Constructs DJAudioPlayer using AudioFormatManager
DJAudioPlayer::DJAudioPlayer(AudioFormatManager& _formatManager)
    : formatManager(_formatManager)
{
}


DJAudioPlayer::~DJAudioPlayer()
{
}

// Prepares audio player for playback
void DJAudioPlayer::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    transportSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
    resampleSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
    currentSampleRate = sampleRate;
}

// Fills buffer with next block of audio
void DJAudioPlayer::getNextAudioBlock(const AudioSourceChannelInfo& bufferToFill)
{
    // Get the next audio block from resampling source
    resampleSource.getNextAudioBlock(bufferToFill);

    // Processes audio
    if (bufferToFill.buffer != nullptr && bufferToFill.buffer->getNumChannels() >= 2)
    {
        const int numSamples = bufferToFill.numSamples;
        float* leftChannel = bufferToFill.buffer->getWritePointer(0, bufferToFill.startSample);
        float* rightChannel = bufferToFill.buffer->getWritePointer(1, bufferToFill.startSample);

        for (int i = 0; i < numSamples; ++i)
        {
            float L = leftChannel[i];
            float R = rightChannel[i];

            // Compute mid and side components
            float mid = (L + R) * 0.5f;
            float side = (L - R) * 0.5f;

            float midGain, sideGain;
            if (vocalMix < 0.5)
            {
                // For vocalMix in [0.0, 0.5): blend from side-only to original stereo
                midGain = 2.0f * static_cast<float>(vocalMix); // 0 at 0.0 to 1 at 0.5
                sideGain = 1.0f; // full side channel always
            }
            else
            {
                // For vocalMix in [0.5, 1.0]: blend from original stereo to mid-only
                midGain = 1.0f; // full mid channel always
                sideGain = 2.0f - 2.0f * static_cast<float>(vocalMix); // 1 at 0.5 to 0 at 1.0
            }

            leftChannel[i] = midGain * mid + sideGain * side;
            rightChannel[i] = midGain * mid - sideGain * side;
        }
    }
}

// Releases audio resources
void DJAudioPlayer::releaseResources()
{
    transportSource.releaseResources();
    resampleSource.releaseResources();
}

// Loads an audio file from the given URL
void DJAudioPlayer::loadURL(URL audioURL)
{
    auto* reader = formatManager.createReaderFor(audioURL.createInputStream(false));
    if (reader != nullptr)
    {
        std::unique_ptr<AudioFormatReaderSource> newSource(new AudioFormatReaderSource(reader, true));
        transportSource.setSource(newSource.get(), 0, nullptr, reader->sampleRate);
        readerSource.reset(newSource.release());
    }
}

// Sets gain for the audio playback
void DJAudioPlayer::setGain(double gain)
{
    if (gain < 0 || gain > 1.0)
        std::cout << "DJAudioPlayer::setGain gain should be between 0 and 1" << std::endl;
    else
        transportSource.setGain(gain);
}

// Sets playback speed by adjusting resampling ratio.
void DJAudioPlayer::setSpeed(double ratio)
{
    if (ratio < 0 || ratio > 100.0)
        std::cout << "DJAudioPlayer::setSpeed ratio should be between 0 and 100" << std::endl;
    else
        resampleSource.setResamplingRatio(ratio);
}

// Sets current playback position
void DJAudioPlayer::setPosition(double posInSecs)
{
    transportSource.setPosition(posInSecs);
}

// Sets playback position relative to track's length
void DJAudioPlayer::setPositionRelative(double pos)
{
    if (pos < 0 || pos > 1.0)
        std::cout << "DJAudioPlayer::setPositionRelative pos should be between 0 and 1" << std::endl;
    else
    {
        double posInSecs = transportSource.getLengthInSeconds() * pos;
        setPosition(posInSecs);
    }
}

// Sets the vocal mix parameter for mid/side processing
void DJAudioPlayer::setVocalMix(double sliderValue)
{
    vocalMix = jlimit(0.0, 1.0, sliderValue);
}

// Starts audio playback
void DJAudioPlayer::start()
{
    transportSource.start();
}

// Stops audio playback
void DJAudioPlayer::stop()
{
    transportSource.stop();
}

// Returns relative position of playhead
double DJAudioPlayer::getPositionRelative()
{
    return transportSource.getCurrentPosition() / transportSource.getLengthInSeconds();
}

// Returns current playback position
double DJAudioPlayer::getCurrentPosition()
{
    return transportSource.getCurrentPosition();
}

// Returns total track length
double DJAudioPlayer::getTrackLength()
{
    return transportSource.getLengthInSeconds();
}
