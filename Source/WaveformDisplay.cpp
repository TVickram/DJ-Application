#include "../JuceLibraryCode/JuceHeader.h"
#include "WaveformDisplay.h"

//------------------------------------------------------------------------------
WaveformDisplay::WaveformDisplay(AudioFormatManager& formatManagerToUse,
    AudioThumbnailCache& cacheToUse)
    : audioThumb(1000, formatManagerToUse, cacheToUse),
    fileLoaded(false),
    position(0)
{
    // Register this component as a listener to the audio thumbnail
    audioThumb.addChangeListener(this);
}

//------------------------------------------------------------------------------
WaveformDisplay::~WaveformDisplay()
{

}

//------------------------------------------------------------------------------
void WaveformDisplay::paint(Graphics& g)
{
    // Background colour
    g.fillAll(Colours::lightseagreen.darker(1.0f));

    // Draw outline around component
    g.setColour(Colours::grey);
    g.drawRect(getLocalBounds(), 1);

    // If audio file is loaded, draw its waveform
    g.setColour(Colours::orange);
    if (fileLoaded)
    {
        // Draw the audio thumbnail across the entire component
        audioThumb.drawChannel(g,
            getLocalBounds(),
            0,
            audioThumb.getTotalLength(),
            0,
            1.0f
        );

        // Draw a rectangle to indicate the current playhead position
        g.setColour(Colours::lightgreen);
        g.drawRect(position * getWidth(), 0, getWidth() / 20, getHeight());
    }
    else
    {
        // If no file is loaded, display placeholder text
        g.setFont(20.0f);
        g.drawText("File not loaded...", getLocalBounds(),
            Justification::centred, true);
    }
}

//------------------------------------------------------------------------------
void WaveformDisplay::resized()
{
    // No child components to layout
}

//------------------------------------------------------------------------------
void WaveformDisplay::loadURL(URL audioURL)
{
    // Clears any previous thumbnail data
    audioThumb.clear();

    // Sets source for the audio thumbnail using URLInputSource
    fileLoaded = audioThumb.setSource(new URLInputSource(audioURL));

    // Logs load status and trigger repaint
    if (fileLoaded)
    {
        std::cout << "wfd: loaded!" << std::endl;
        repaint();
    }
    else
    {
        std::cout << "wfd: not loaded!" << std::endl;
    }
}

//------------------------------------------------------------------------------
void WaveformDisplay::changeListenerCallback(ChangeBroadcaster* source)
{
    // Log change event was received
    std::cout << "wfd: change received!" << std::endl;

    // Repaint component to update waveform display
    repaint();
}

//------------------------------------------------------------------------------
void WaveformDisplay::setPositionRelative(double pos)
{
    // Only update and repaint if position changed
    if (pos != position)
    {
        position = pos;
        repaint();
    }
}
