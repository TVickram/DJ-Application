#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

// WaveformDisplay renders audio waveform using AudioThumbnail
// Also allows setting playhead position
class WaveformDisplay : public Component,
    public ChangeListener
{
public:
    // Constructs WaveformDisplay using AudioFormatManager and AudioThumbnailCache
    WaveformDisplay(AudioFormatManager& formatManagerToUse,
        AudioThumbnailCache& cacheToUse);

    // Destructor
    ~WaveformDisplay();

    // Draws waveform if file loaded. Else placeholder text displayed
    void paint(Graphics&) override;

    void resized() override;

    // Called when change event is received from AudioThumbnail
    void changeListenerCallback(ChangeBroadcaster* source) override;

    // Loads audio file from given URL into the waveform display
    void loadURL(URL audioURL);

    // Sets relative position of the playhead and repaints display.
    void setPositionRelative(double pos);

private:
    AudioThumbnail audioThumb;
    bool fileLoaded;
    double position;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WaveformDisplay)
};
