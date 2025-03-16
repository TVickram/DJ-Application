#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include <cmath>

// DJAudioPlayer handles audio playback and processing
class DJAudioPlayer : public AudioSource {
public:
    // Constructs DJAudioPlayer using AudioFormatManager
    DJAudioPlayer(AudioFormatManager& _formatManager);

    // Destructor.
    ~DJAudioPlayer();

    // Prepares audio player for playback
    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;

    // Fills audio buffer with the next block of audio
    void getNextAudioBlock(const AudioSourceChannelInfo& bufferToFill) override;

    // Releases audio resources
    void releaseResources() override;

    // Loads audio file from the provided URL
    void loadURL(URL audioURL);

    // Sets gain level
    void setGain(double gain);

    // Sets playback speed ratio
    void setSpeed(double ratio);

    // Sets the playback position
    void setPosition(double posInSecs);

    // Sets the playback position as a relative value
    void setPositionRelative(double pos);

    // Sets the vocal mix slider value to control mid/side processing.
    void setVocalMix(double sliderValue);

    // Starts audio playback.
    void start();

    // Stops audio playback.
    void stop();

    // Returns relative playhead position
    double getPositionRelative();

    // Returns current playback position
    double getCurrentPosition();

    // Returns total length of track
    double getTrackLength();

private:
    AudioFormatManager& formatManager;
    std::unique_ptr<AudioFormatReaderSource> readerSource;
    AudioTransportSource transportSource;
    ResamplingAudioSource resampleSource{ &transportSource, false, 2 };

    // Current sample rate for audio processing
    double currentSampleRate = 44100.0;

    // Vocal mix parameter controlling mid/side processing
    double vocalMix = 0.5;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DJAudioPlayer)
};
