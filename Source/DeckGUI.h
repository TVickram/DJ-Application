#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "DJAudioPlayer.h"
#include "WaveformDisplay.h"

// Constructs DeckGUI object
class DeckGUI : public Component,
    public Button::Listener,
    public FileDragAndDropTarget,
    public Timer
{
public:
    // Constructs DeckGUI
    DeckGUI(DJAudioPlayer* player,
        AudioFormatManager& formatManagerToUse,
        AudioThumbnailCache& cacheToUse,
        const String& deckLabel = String());

    // Destroys DeckGUI, stopping any running timers
    ~DeckGUI();

    // Draws components
    void paint(Graphics&) override;

    // Resizes child components: waveform display + control buttons
    void resized() override;

    // Button click event for DeckGUI buttons: play, stop, load
    void buttonClicked(Button*) override;

    // Indicates file drag-and-drop events
    bool isInterestedInFileDrag(const StringArray& files) override;

    // Handles files dropped onto component
    void filesDropped(const StringArray& files, int x, int y) override;

    // Timer callback
    void timerCallback() override;

    // Handles mousePress for turntable
    void mouseDown(const MouseEvent& event) override;

    // Handles mouseDrag that updates turntable
    void mouseDrag(const MouseEvent& event) override;

    // Handles mouseRelease events for turntable
    void mouseUp(const MouseEvent& event) override;

    // Loads audio file
    void loadFile(const File& file);

private:
    TextButton playButton{ "PLAY" };
    TextButton stopButton{ "STOP" };
    TextButton loadButton{ "LOAD" };

    FileChooser fChooser{ "Select a file..." };
    WaveformDisplay waveformDisplay;
    DJAudioPlayer* player;

    // Deck label for L and R of turntable
    String deckLabel;

    // Rotation angle of turntable (radian)
    float rotationAngle = 0.0f;

    // Variables that handles turntable rotation
    float lastMouseAngle = 0.0f;
    double accumulatedDeltaAngle = 0.0; 

    // Stores the track position (in seconds)
    double initialTrackPosition = 0.0;

    bool draggingTurntable = false;

    // Variables for color change of the border outline
    int currentColorIndex = 0;
    double lastColorUpdateTime = 0.0;
    std::vector<Colour> colorList;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DeckGUI)
};
