#pragma once

#include <JuceHeader.h>
#include <vector>
#include <string>
#include "DJAudioPlayer.h"
#include "DeckGUI.h"
#include <cmath> // For std::cos and std::sin

// CustomButton with original design.
class CustomButton : public juce::TextButton
{
public:
    // Constructs a CustomButton with the specified button name.
    CustomButton(const juce::String& buttonName) : juce::TextButton(buttonName) {}
    // Draws the button with custom styling.
    void paintButton(juce::Graphics& g, bool isMouseOverButton, bool isButtonDown) override;
};

// Custom look and feel for rotary sliders (knob design).
class CustomKnobLookAndFeel : public juce::LookAndFeel_V4
{
public:
    // Constructs a CustomKnobLookAndFeel and sets default colours.
    CustomKnobLookAndFeel()
    {
        setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::grey);
        setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colours::black);
    }

    // Draws a rotary slider with tick marks.
    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
        const float rotaryStartAngle, const float rotaryEndAngle, juce::Slider& slider) override
    {
        auto radius = juce::jmin(width, height) / 2.0f - 8.0f;
        auto centreX = x + width * 0.5f;
        auto centreY = y + height * 0.5f;
        auto knobRect = juce::Rectangle<float>(centreX - radius, centreY - radius, radius * 2, radius * 2);

        g.setColour(slider.findColour(juce::Slider::rotarySliderFillColourId));
        g.fillEllipse(knobRect);

        g.setColour(slider.findColour(juce::Slider::rotarySliderOutlineColourId));
        g.drawEllipse(knobRect, 2.0f);

        int numTicks = 20;
        float tickLength = 4.0f;
        float tickThickness = 2.0f;
        for (int i = 0; i < numTicks; ++i)
        {
            float tickValue = i / float(numTicks - 1);
            float tickAngle = rotaryStartAngle + tickValue * (rotaryEndAngle - rotaryStartAngle);
            float tickStartRadius = radius + 4.0f;
            float tickEndRadius = tickStartRadius + tickLength;
            float startX = centreX + tickStartRadius * std::cos(tickAngle);
            float startY = centreY + tickStartRadius * std::sin(tickAngle);
            float endX = centreX + tickEndRadius * std::cos(tickAngle);
            float endY = centreY + tickEndRadius * std::sin(tickAngle);
            juce::Colour tickColour = (tickValue <= sliderPos) ? juce::Colours::green : juce::Colours::white;
            g.setColour(tickColour);
            g.drawLine(startX, startY, endX, endY, tickThickness);
        }
    }
};

// Custom look and feel for the crossfader slider.
class CrossfaderLookAndFeel : public juce::LookAndFeel_V4
{
public:
    // Constructs a CrossfaderLookAndFeel.
    CrossfaderLookAndFeel() {}

    // Draws a linear slider styled as a crossfader.
    void drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
        float sliderPos, float minSliderPos, float maxSliderPos,
        const juce::Slider::SliderStyle style, juce::Slider& slider) override
    {
        auto trackHeight = height * 0.4f;
        juce::Rectangle<float> trackRect(x, y + (height - trackHeight) / 2.0f, width, trackHeight);
        g.setColour(juce::Colours::darkgrey);
        g.fillRect(trackRect);

        float thumbWidth = 20.0f;
        juce::Rectangle<float> thumbRect(sliderPos - thumbWidth / 2.0f, y, thumbWidth, height);
        g.setColour(juce::Colours::grey);
        g.fillRect(thumbRect);
        g.setColour(juce::Colours::black);
        g.drawRect(thumbRect, 1.0f);
    }
};

// PlaylistComponent handles track listing, file loading, and assigning tracks to decks.
class PlaylistComponent : public juce::Component,
    public juce::TableListBoxModel,
    public juce::Button::Listener,
    public juce::Slider::Listener
{
public:
    // Constructs a PlaylistComponent with pointers to the players and deck GUIs.
    PlaylistComponent(DJAudioPlayer* deck1, DJAudioPlayer* deck2,
        DeckGUI* leftGUI, DeckGUI* rightGUI,
        DJAudioPlayer* drumPlayer);
    // Destructor.
    ~PlaylistComponent() override;

    // Paints the component's background.
    void paint(juce::Graphics&) override;
    // Lays out the component's child elements.
    void resized() override;

    // Returns the number of rows in the track list.
    int getNumRows() override;
    // Paints the background of a row in the table.
    void paintRowBackground(juce::Graphics& g, int rowNumber, int width, int height, bool rowIsSelected) override;
    // Paints a cell in the table.
    void paintCell(juce::Graphics& g, int rowNumber, int columnId, int width, int height, bool rowIsSelected) override;
    // Provides or refreshes a component for a cell.
    juce::Component* refreshComponentForCell(int rowNumber, int columnId, bool isRowSelected, juce::Component* existingComponentToUpdate) override;

    // Handles button click events.
    void buttonClicked(juce::Button* button) override;
    // Handles slider value changes.
    void sliderValueChanged(juce::Slider* slider) override;

    // Assigns the track from the given row to a deck (left if assignLeft is true).
    void assignTrackToDeck(int row, bool assignLeft);

    // Inner class for track assignment buttons.
    class TrackButtonsComponent : public juce::Component,
        public juce::Button::Listener
    {
    public:
        // Constructs TrackButtonsComponent for the specified row and parent component.
        TrackButtonsComponent(int row, PlaylistComponent* parentComp);
        // Lays out the button components.
        void resized() override;
        // Updates the row ID for this component.
        void setRowId(int row);
        // Handles button click events.
        void buttonClicked(juce::Button* button) override;
    private:
        int rowId;
        juce::TextButton leftButton{ "Left" };
        juce::TextButton rightButton{ "Right" };
        PlaylistComponent* parent = nullptr;
    };

private:
    juce::TableListBox tableComponent;
    std::vector<std::string> trackTitles;
    std::vector<juce::File> trackFiles;

    juce::Label headerLabel;
    juce::TextButton loadButton{ "Load" };

    // A simple labeled slider.
    class LabeledSlider : public juce::Slider
    {
    public:
        // Constructs a LabeledSlider with the specified label.
        LabeledSlider(const juce::String& sliderLabel)
            : label(sliderLabel)
        {
            setSliderStyle(juce::Slider::Rotary);
            setTextBoxStyle(juce::Slider::TextBoxLeft, false, 70, 20);
        }
        // Returns the label text instead of the numeric value.
        juce::String getTextFromValue(double) override { return label; }
    private:
        juce::String label;
    };

    LabeledSlider volSlider1;
    LabeledSlider speedSlider1;
    LabeledSlider posSlider1;

    LabeledSlider volSlider2;
    LabeledSlider speedSlider2;
    LabeledSlider posSlider2;

    juce::Slider crossfaderSlider;
    juce::Label crossfaderLabel;

    juce::Component bottomPlaceholder;
    CustomKnobLookAndFeel customKnobLookAndFeel;
    CrossfaderLookAndFeel crossfaderLF;
    // Bottom buttons with custom design.
    CustomButton bottomButton1{ "Drum 1" };
    CustomButton bottomButton2{ "Lets Go" };
    CustomButton bottomButton3{ "Siren" };
    CustomButton bottomButton4{ "Drum 2" };
    CustomButton bottomButton5{ "Glasses Up" };
    CustomButton bottomButton6{ "Airhorn" };

    DJAudioPlayer* deck1;
    DJAudioPlayer* deck2;
    DeckGUI* leftDeckGUI;
    DeckGUI* rightDeckGUI;
    DJAudioPlayer* drumPlayer; // Dedicated drum player

    // Updates the gain values based on slider positions
    void updateGains();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PlaylistComponent)
};
