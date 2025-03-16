#include "MainComponent.h"

MainComponent::MainComponent()
{
    setSize(800, 600);

    // Check and request audio recording permission
    if (RuntimePermissions::isRequired(RuntimePermissions::recordAudio)
        && !RuntimePermissions::isGranted(RuntimePermissions::recordAudio))
    {
        RuntimePermissions::request(RuntimePermissions::recordAudio,
            [this](bool granted) { if (granted) setAudioChannels(0, 2); });
    }
    else
    {
        setAudioChannels(0, 2);
    }

    // Add child components
    addAndMakeVisible(deckGUI1);
    addAndMakeVisible(deckGUI2);
    addAndMakeVisible(playlistComponent);

    // Register basic audio formats
    formatManager.registerBasicFormats();

    // Start timer for the background carousel animation at 60 Hz
    startTimerHz(60);
}

MainComponent::~MainComponent()
{
    stopTimer();
    shutdownAudio();
}

void MainComponent::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    // Prepare each audio player and the mixer source for playback
    player1.prepareToPlay(samplesPerBlockExpected, sampleRate);
    player2.prepareToPlay(samplesPerBlockExpected, sampleRate);
    drumPlayer.prepareToPlay(samplesPerBlockExpected, sampleRate);

    mixerSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
    mixerSource.addInputSource(&player1, false);
    mixerSource.addInputSource(&player2, false);
    mixerSource.addInputSource(&drumPlayer, false);
}

void MainComponent::getNextAudioBlock(const AudioSourceChannelInfo& bufferToFill)
{
    // Retrieve the next audio block from mixer
    mixerSource.getNextAudioBlock(bufferToFill);
}

void MainComponent::releaseResources()
{
    // Release resources for all audio players and mixer
    player1.releaseResources();
    player2.releaseResources();
    drumPlayer.releaseResources();
    mixerSource.releaseResources();
}

void MainComponent::paint(Graphics& g)
{
    // Background colour
    g.fillAll(Colours::black);

    // Draws scrolling carousel of rectangles
    const int visibleBlocks = 30;
    int rectWidth = getWidth() / visibleBlocks;
    int numRectangles = visibleBlocks + 2;
    for (int i = 0; i < numRectangles; i++)
    {
        // Calculate the current index and x-position for each rectangle
        int overallIndex = (i + static_cast<int>(scrollOffset / rectWidth)) % 10;
        int x = -(static_cast<int>(scrollOffset) % rectWidth) + i * rectWidth;
        g.setColour(colours[overallIndex]);
        g.fillRect(x, 0, rectWidth, getHeight());
    }

    // Draw playlist text
    g.setColour(Colours::white);
    g.setFont(14.0f);
    g.drawText("PlaylistComponent", getLocalBounds(), Justification::centred, true);
}

void MainComponent::resized()
{
    // Define margins and calculate bounds for child components
    const int margin = 10;
    const int playlistWidth = 350;
    const int height = getHeight() - 2 * margin;
    const int availableWidth = getWidth() - 2 * margin - playlistWidth;
    const int deckWidth = availableWidth / 2;

    // Set bounds for deck and playlist
    deckGUI1.setBounds(margin, margin, deckWidth, height);
    playlistComponent.setBounds(margin + deckWidth, margin, playlistWidth, height);
    deckGUI2.setBounds(margin + deckWidth + playlistWidth, margin, deckWidth, height);
}

void MainComponent::timerCallback()
{
    // Update scroll offset for the carousel background
    float speed = 1.5f;
    scrollOffset += speed;

    // Determine the width of a cycle
    const int visibleBlocks = 20;
    int rectWidth = getWidth() / visibleBlocks;
    int cycleWidth = rectWidth * 10;

    // Loop the scroll offset when a full cycle is reached
    if (scrollOffset >= cycleWidth)
        scrollOffset -= cycleWidth;

    repaint();
}
