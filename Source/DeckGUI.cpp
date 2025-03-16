#include "../JuceLibraryCode/JuceHeader.h"
#include "DeckGUI.h"

DeckGUI::DeckGUI(DJAudioPlayer* _player,
    AudioFormatManager& formatManagerToUse,
    AudioThumbnailCache& cacheToUse,
    const String& label)
    : player(_player),
    waveformDisplay(formatManagerToUse, cacheToUse),
    deckLabel(label)
{
    // --- Set up buttons ---
    addAndMakeVisible(playButton);
    addAndMakeVisible(stopButton);
    addAndMakeVisible(loadButton);

    // --- Set up waveform display ---
    addAndMakeVisible(waveformDisplay);

    // --- Register button listeners ---
    playButton.addListener(this);
    stopButton.addListener(this);
    loadButton.addListener(this);

    // --- Initialize the color list with 10 distinct colours ---
    colorList.push_back(Colours::red);
    colorList.push_back(Colours::green);
    colorList.push_back(Colours::blue);
    colorList.push_back(Colours::yellow);
    colorList.push_back(Colours::orange);
    colorList.push_back(Colours::purple);
    colorList.push_back(Colours::cyan);
    colorList.push_back(Colours::magenta);
    colorList.push_back(Colours::lime);
    colorList.push_back(Colours::pink);

    // Set the starting time for the color transition and start the timer
    lastColorUpdateTime = Time::getMillisecondCounterHiRes();
    startTimer(50); // Timer interval of 50ms
}

DeckGUI::~DeckGUI()
{
    stopTimer();
}

void DeckGUI::paint(Graphics& g)
{
    // Background colour
    g.fillAll(Colours::rebeccapurple.darker(2.0f));

    // Draw color-changing border
    double currentTime = Time::getMillisecondCounterHiRes();
    double elapsed = currentTime - lastColorUpdateTime;
    // Limit time to 1 second (1000ms)
    float factor = static_cast<float>(jlimit(0.0, 1000.0, elapsed) / 1000.0);
    int nextColorIndex = (currentColorIndex + 1) % static_cast<int>(colorList.size());
    Colour effectiveColour = colorList[currentColorIndex].interpolatedWith(colorList[nextColorIndex], factor);
    g.setColour(effectiveColour);
    g.drawRect(getLocalBounds(), 4);

    // Top colour changing bar
    {
        auto topBarArea = Rectangle<int>(0, 0, getWidth(), getHeight() / 30);
        g.setColour(Colours::black.withAlpha(0.8f));
        g.fillRect(topBarArea.reduced(2));
        g.setColour(effectiveColour);
        g.fillRect(topBarArea.reduced(2));
    }

    // Turntable drawing
    {
        auto centerArea = getLocalBounds().reduced(10);
        // Diameter of the turntable wheel.
        auto wheelDiameter = jmin(centerArea.getWidth(), centerArea.getHeight()) - 20;
        float centerX = static_cast<float>(centerArea.getCentreX());
        float centerY = static_cast<float>(centerArea.getCentreY() + 60);

        // Save the current state of the graphics
        Graphics::ScopedSaveState state(g);
        // Rotate the turntable
        g.addTransform(AffineTransform::rotation(rotationAngle, centerX, centerY));

        // Draw the main turntable circle.
        g.setColour(Colours::black);
        g.fillEllipse(centerX - wheelDiameter / 2, centerY - wheelDiameter / 2, wheelDiameter, wheelDiameter);
        // Draw the outer border of the turntable.
        g.setColour(Colours::darkgrey);
        g.drawEllipse(centerX - wheelDiameter / 2, centerY - wheelDiameter / 2, wheelDiameter, wheelDiameter, 3.0f);
        // Draw an additional border.
        g.setColour(Colours::red);
        g.drawEllipse(centerX - wheelDiameter / 2 - 2, centerY - wheelDiameter / 2 - 2, wheelDiameter + 4, wheelDiameter + 4, 2.0f);

        // Draw inner circles for turntable
        int numCircles = 5;
        float outerRadius = wheelDiameter / 2.0f;
        for (int j = 1; j <= numCircles; j++)
        {
            float radius = outerRadius - j * (outerRadius / (numCircles + 1));
            float diameter = radius * 2.0f;
            float x = centerX - radius;
            float y = centerY - radius;
            g.setColour(Colours::grey);
            g.drawEllipse(x, y, diameter, diameter, 1.0f);
        }

        //Draw L or R in turntable
        if (!deckLabel.isEmpty())
        {
            float innerRadius = outerRadius - numCircles * (outerRadius / (numCircles + 1));
            float innerDiameter = innerRadius * 2.0f;
            g.setColour(Colours::red);
            Font labelFont(innerDiameter * 0.8f, Font::bold);
            g.setFont(labelFont);
            Rectangle<int> labelBounds(static_cast<int>(centerX - innerRadius),
                static_cast<int>(centerY - innerRadius),
                static_cast<int>(innerDiameter),
                static_cast<int>(innerDiameter));
            g.drawFittedText(deckLabel, labelBounds, Justification::centred, 1);
        }
    }

    // Bottom colour changing bar 
    {
        auto bottomBarArea = Rectangle<int>(0, getHeight() - getHeight() / 30, getWidth(), getHeight() / 30);
        g.setColour(Colours::black.withAlpha(0.8f));
        g.fillRect(bottomBarArea.reduced(2));
        g.setColour(effectiveColour);
        g.fillRect(bottomBarArea.reduced(2));
    }
}

void DeckGUI::resized()
{
    auto area = getLocalBounds();
    // Reserve space for the top colour changing bar
    area.removeFromTop(getHeight() / 30 + 2);

    // Set bounds for the waveform display
    auto waveHeight = area.getHeight() / 6;
    waveformDisplay.setBounds(area.removeFromTop(waveHeight).reduced(5));

    // Set bounds for the control buttons
    auto buttonHeight = area.getHeight() / 10;
    auto buttonArea = area.removeFromTop(buttonHeight);
    int buttonWidth = buttonArea.getWidth() / 3;
    playButton.setBounds(buttonArea.removeFromLeft(buttonWidth).reduced(5));
    stopButton.setBounds(buttonArea.removeFromLeft(buttonWidth).reduced(5));
    loadButton.setBounds(buttonArea.removeFromLeft(buttonWidth).reduced(5));
}

void DeckGUI::buttonClicked(Button* button)
{
    if (button == &playButton)
    {
        std::cout << "Play button was clicked\n";
        player->start();
    }
    else if (button == &stopButton)
    {
        std::cout << "Stop button was clicked\n";
        player->stop();
    }
    else if (button == &loadButton)
    {
        std::cout << "Load button was clicked\n";
        auto fileChooserFlags = FileBrowserComponent::canSelectFiles;
        fChooser.launchAsync(fileChooserFlags, [this](const FileChooser& chooser)
            {
                auto chosen = chooser.getResult();
                if (chosen.existsAsFile())
                {
                    loadFile(chosen);
                }
            });
    }
}

bool DeckGUI::isInterestedInFileDrag(const StringArray& files)
{
    std::cout << "DeckGUI::isInterestedInFileDrag" << std::endl;
    return true;
}

void DeckGUI::filesDropped(const StringArray& files, int x, int y)
{
    std::cout << "DeckGUI::filesDropped" << std::endl;
    if (files.size() == 1)
    {
        File file{ files[0] };
        if (file.existsAsFile())
            loadFile(file);
    }
}

void DeckGUI::timerCallback()
{
    // Update waveform position relative to playback position
    waveformDisplay.setPositionRelative(player->getPositionRelative());

    // Color transition for the border
    double currentTime = Time::getMillisecondCounterHiRes();
    if (currentTime - lastColorUpdateTime >= 1000.0)
    {
        currentColorIndex = (currentColorIndex + 1) % static_cast<int>(colorList.size());
        lastColorUpdateTime = currentTime;
    }
    repaint();
}

void DeckGUI::mouseDown(const MouseEvent& event)
{
    auto centerArea = getLocalBounds().reduced(10);
    auto wheelDiameter = jmin(centerArea.getWidth(), centerArea.getHeight()) - 20;
    Point<float> center(static_cast<float>(centerArea.getCentreX()),
        static_cast<float>(centerArea.getCentreY()));
    float distance = event.position.getDistanceFrom(center);
    // Start dragging if the click is within the turntable
    if (distance <= wheelDiameter / 2.0f)
    {
        draggingTurntable = true;
        // Calculate the initial angle relative to the center
        lastMouseAngle = std::atan2(event.position.y - center.getY(), event.position.x - center.getX());
        accumulatedDeltaAngle = 0.0;
        initialTrackPosition = player->getCurrentPosition();
    }
}

void DeckGUI::mouseDrag(const MouseEvent& event)
{
    if (draggingTurntable)
    {
        auto centerArea = getLocalBounds().reduced(10);
        Point<float> center(static_cast<float>(centerArea.getCentreX()),
            static_cast<float>(centerArea.getCentreY()));
        // Calculate the current angle of the mouse relative to the center
        float currentMouseAngle = std::atan2(event.position.y - center.getY(), event.position.x - center.getX());
        float deltaAngle = currentMouseAngle - lastMouseAngle;

        // Wrap deltaAngle to account for full circle rotation
        if (deltaAngle > MathConstants<float>::pi)
            deltaAngle -= 2.0f * MathConstants<float>::pi;
        else if (deltaAngle < -MathConstants<float>::pi)
            deltaAngle += 2.0f * MathConstants<float>::pi;

        accumulatedDeltaAngle += deltaAngle;
        lastMouseAngle = currentMouseAngle;
        rotationAngle += deltaAngle;

        // Calculate the corresponding track offset
        double secondsPerRadian = 5.0 / (2.0 * MathConstants<double>::pi);
        double offset = accumulatedDeltaAngle * secondsPerRadian;
        double newPosition = initialTrackPosition + offset;
        double trackLength = player->getTrackLength();
        // Clamp the new position in track length.
        newPosition = jlimit(0.0, trackLength, newPosition);
        player->setPosition(newPosition);
        repaint();
    }
}

void DeckGUI::mouseUp(const MouseEvent& /*event*/)
{
    // End dragging of turntable
    draggingTurntable = false;
}

void DeckGUI::loadFile(const File& file)
{
    if (file.existsAsFile())
    {
        // Load file into both audio player and waveform display
        player->loadURL(juce::URL(file));
        waveformDisplay.loadURL(juce::URL(file));
    }
}
