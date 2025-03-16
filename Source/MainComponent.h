#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "DJAudioPlayer.h"
#include "DeckGUI.h"
#include "PlaylistComponent.h"

// MainComponent sets overall UI and audio routing
class MainComponent : public AudioAppComponent,
    private Timer
{
public:
    // Constructs MainComponent + initializes audio channels and child components
    MainComponent();

    // Destructor
    ~MainComponent() override;

    // Prepares audio sources for playback
    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;

    // Provides next block of audio data from mixer
    void getNextAudioBlock(const AudioSourceChannelInfo& bufferToFill) override;

    // Releases audio resources
    void releaseResources() override;

    // Paints the background carousel and text
    void paint(Graphics& g) override;

    // Lays out child components
    void resized() override;

private:
    // Timer callback: updates the carousel scroll
    void timerCallback() override;

    // Carousel background 
    Colour colours[10] =
    {
        Colour::fromRGB(255, 105, 180),
        Colour::fromRGB(57, 255, 20),
        Colour::fromRGB(0, 255, 255),
        Colour::fromRGB(255, 165, 0),
        Colour::fromRGB(191, 0, 255),
        Colour::fromRGB(255, 255, 102),
        Colour::fromRGB(0, 255, 200),
        Colour::fromRGB(255, 0, 255),
        Colour::fromRGB(204, 255, 51),
        Colour::fromRGB(0, 245, 255)
    };

    float scrollOffset = 0.0f;

    AudioFormatManager formatManager;
    AudioThumbnailCache thumbCache{ 100 };

    // Primary players and decks
    DJAudioPlayer player1{ formatManager };
    DeckGUI deckGUI1{ &player1, formatManager, thumbCache, "L" };

    DJAudioPlayer player2{ formatManager };
    DeckGUI deckGUI2{ &player2, formatManager, thumbCache, "R" };

    // Drum player
    DJAudioPlayer drumPlayer{ formatManager };

    MixerAudioSource mixerSource;

    // Pointers to players, decks, and drum player
    PlaylistComponent playlistComponent{ &player1, &player2, &deckGUI1, &deckGUI2, &drumPlayer };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
