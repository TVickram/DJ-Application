#include <JuceHeader.h>
#include "PlaylistComponent.h"

//==============================================================================
// CustomButton implementation

// Custom drawing for the button
void CustomButton::paintButton(juce::Graphics& g, bool isMouseOverButton, bool isButtonDown)
{
    auto boundsF = getLocalBounds().toFloat();

    // Button colour change for when it is pressed
    if (isButtonDown)
        g.setColour(juce::Colours::red.withAlpha(0.5f));
    else
        g.setColour(juce::Colours::black);
    g.fillRect(boundsF);

    // Button border colour
    g.setColour(juce::Colours::red);
    g.drawRect(getLocalBounds(), 2);

    // Text area at bottom of button
    auto textArea = getLocalBounds().removeFromBottom(20);
    g.setColour(juce::Colours::white);
    g.setFont(juce::Font(15.0f));
    g.drawText(getButtonText(), textArea, juce::Justification::centred, true);
}

//==============================================================================
// TrackButtonsComponent methods

// Constructor: initializes buttons for a given row
PlaylistComponent::TrackButtonsComponent::TrackButtonsComponent(int row, PlaylistComponent* parentComp)
    : rowId(row), parent(parentComp)
{
    // Set button texts
    leftButton.setButtonText("Left");
    rightButton.setButtonText("Right");

    // Add buttons as child components
    addAndMakeVisible(leftButton);
    addAndMakeVisible(rightButton);

    // Register this component as listener for buttons
    leftButton.addListener(this);
    rightButton.addListener(this);
}

// Lays out buttons side by side
void PlaylistComponent::TrackButtonsComponent::resized()
{
    auto bounds = getLocalBounds();
    // Place leftButton on the left half and rightButton on the other half
    leftButton.setBounds(bounds.removeFromLeft(bounds.getWidth() / 2).reduced(2));
    rightButton.setBounds(bounds.reduced(2));
}

void PlaylistComponent::TrackButtonsComponent::setRowId(int row)
{
    rowId = row;
}

// Handle clicks on the left/right buttons
void PlaylistComponent::TrackButtonsComponent::buttonClicked(juce::Button* button)
{
    if (parent != nullptr)
    {
        if (button == &leftButton)
            parent->assignTrackToDeck(rowId, true);   // Assign track to left deck.
        else if (button == &rightButton)
            parent->assignTrackToDeck(rowId, false);  // Assign track to right deck.
    }
}

//==============================================================================
// PlaylistComponent methods

// Constructor: sets up the playlist, initializes track data and configures UI elements
PlaylistComponent::PlaylistComponent(DJAudioPlayer* d1, DJAudioPlayer* d2,
    DeckGUI* leftGUI, DeckGUI* rightGUI,
    DJAudioPlayer* drumPlayerIn)
    : volSlider1("Volume L"),
    speedSlider1("Speed L"),
    posSlider1("Vocal Mix L"),
    volSlider2("Volume R"),
    speedSlider2("Speed R"),
    posSlider2("Vocal Mix R"),
    deck1(d1),
    deck2(d2),
    leftDeckGUI(leftGUI),
    rightDeckGUI(rightGUI),
    drumPlayer(drumPlayerIn)
{
    // Determine the assets directory
    juce::File sourceDir(String(__FILE__));
    sourceDir = sourceDir.getParentDirectory();
    juce::File assetsDir = sourceDir.getChildFile("assets");

    // Set up track file paths for two songs
    juce::File song1 = assetsDir.getChildFile("Song1.mp3");
    juce::File song2 = assetsDir.getChildFile("Song2.mp3");

    // Check if song1 exists and stores its title and file
    if (song1.existsAsFile())
    {
        trackTitles.push_back("Die with a smile");
        trackFiles.push_back(song1);
    }
    //if song1 doesnt exits goes to default
    else
    {
        trackTitles.push_back("Track 1");
        trackFiles.push_back(juce::File());
    }

    // Similarly check and store song2
    if (song2.existsAsFile())
    {
        trackTitles.push_back("Not like us");
        trackFiles.push_back(song2);
    }
    //Similarly goes to default if doesnt exits
    else
    {
        trackTitles.push_back("Track 2");
        trackFiles.push_back(juce::File());
    }

    // Configure and display header label
    headerLabel.setJustificationType(juce::Justification::centred);
    headerLabel.setFont(juce::Font(20.0f, juce::Font::bold));
    addAndMakeVisible(headerLabel);

    // Configure load button and register its listener
    addAndMakeVisible(loadButton);
    loadButton.addListener(this);

    // Set up bottom buttons and register listeners for them
    addAndMakeVisible(bottomButton1); bottomButton1.addListener(this);
    addAndMakeVisible(bottomButton2); bottomButton2.addListener(this);
    addAndMakeVisible(bottomButton3); bottomButton3.addListener(this);
    addAndMakeVisible(bottomButton4); bottomButton4.addListener(this);
    addAndMakeVisible(bottomButton5); bottomButton5.addListener(this);
    addAndMakeVisible(bottomButton6); bottomButton6.addListener(this);

    // Setup Deck 1 sliders
    addAndMakeVisible(volSlider1);
    addAndMakeVisible(speedSlider1);
    addAndMakeVisible(posSlider1);
    volSlider1.setRange(0.0, 1.0);
    speedSlider1.setRange(0.01, 2.0);
    posSlider1.setRange(0.0, 1.0);
    volSlider1.setValue(0.5);
    speedSlider1.setValue(1.0);
    posSlider1.setValue(0.5);
    volSlider1.addListener(this);
    speedSlider1.addListener(this);
    posSlider1.addListener(this);

    // Setup Deck 2 sliders
    addAndMakeVisible(volSlider2);
    addAndMakeVisible(speedSlider2);
    addAndMakeVisible(posSlider2);
    volSlider2.setRange(0.0, 1.0);
    speedSlider2.setRange(0.01, 2.0);
    posSlider2.setRange(0.0, 1.0);
    volSlider2.setValue(0.5);
    speedSlider2.setValue(1.0);
    posSlider2.setValue(0.5);
    volSlider2.addListener(this);
    speedSlider2.addListener(this);
    posSlider2.addListener(this);

    // Custom look for rotary knob design on sliders
    volSlider1.setLookAndFeel(&customKnobLookAndFeel);
    speedSlider1.setLookAndFeel(&customKnobLookAndFeel);
    posSlider1.setLookAndFeel(&customKnobLookAndFeel);
    volSlider2.setLookAndFeel(&customKnobLookAndFeel);
    speedSlider2.setLookAndFeel(&customKnobLookAndFeel);
    posSlider2.setLookAndFeel(&customKnobLookAndFeel);

    // Configure table component for displaying the tracks
    tableComponent.getHeader().addColumn("Track title", 1, 150);
    tableComponent.getHeader().addColumn("Assign", 2, 200);
    tableComponent.setModel(this);
    addAndMakeVisible(tableComponent);
    // Hide header for cleaner look
    tableComponent.getHeader().setVisible(false);

    // For bottom buttons 
    addAndMakeVisible(bottomPlaceholder);
    bottomPlaceholder.addAndMakeVisible(bottomButton1);
    bottomPlaceholder.addAndMakeVisible(bottomButton2);
    bottomPlaceholder.addAndMakeVisible(bottomButton3);
    bottomPlaceholder.addAndMakeVisible(bottomButton4);
    bottomPlaceholder.addAndMakeVisible(bottomButton5);
    bottomPlaceholder.addAndMakeVisible(bottomButton6);
    bottomPlaceholder.setName("Placeholder");

    // Configure crossfader slider
    crossfaderSlider.setRange(0.0, 1.0);
    crossfaderSlider.setValue(0.5);
    crossfaderSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    crossfaderSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    crossfaderSlider.setLookAndFeel(&crossfaderLF);
    crossfaderSlider.addListener(this);
    addAndMakeVisible(crossfaderSlider);

    // Set up and display the crossfader label
    crossfaderLabel.setText("Crossfader", juce::dontSendNotification);
    crossfaderLabel.setJustificationType(juce::Justification::centred);
    crossfaderLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(crossfaderLabel);
}

// Destructor to avoid dangling pointers
PlaylistComponent::~PlaylistComponent()
{
    volSlider1.setLookAndFeel(nullptr);
    speedSlider1.setLookAndFeel(nullptr);
    posSlider1.setLookAndFeel(nullptr);
    volSlider2.setLookAndFeel(nullptr);
    speedSlider2.setLookAndFeel(nullptr);
    posSlider2.setLookAndFeel(nullptr);
}

// Paints background colour
void PlaylistComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::darkgrey.darker(2.0f));
    g.drawRect(getLocalBounds(), 1);
}

// Lays out all child components in PlaylistComponent
void PlaylistComponent::resized()
{
    auto area = getLocalBounds();

    // Divide the area into 3 horizontal sections
    int topSectionHeight = static_cast<int>(area.getHeight() * 0.3);
    int middleSectionHeight = 300;
    auto topSection = area.removeFromTop(topSectionHeight);
    auto middleSection = area.removeFromTop(middleSectionHeight);
    auto bottomSection = area;

    // Layout the header and load button
    int headerHeight = 30;
    auto headerArea = topSection.removeFromTop(headerHeight);
    headerLabel.setBounds(headerArea.removeFromLeft(headerArea.getWidth() * 0.5));
    loadButton.setBounds(headerArea.reduced(5));

    // The table component occupies the remainder of the top section
    tableComponent.setBounds(topSection);

    // Separates area for deck sliders and crossfader
    int crossfaderTotalHeight = 50;
    auto deckSlidersArea = middleSection.removeFromTop(middleSection.getHeight() - crossfaderTotalHeight);
    auto crossfaderArea = middleSection;

    // Split deckSlidersArea into two halves for deck 1 and 2
    auto deck1Area = deckSlidersArea.removeFromLeft(deckSlidersArea.getWidth() / 2);
    auto deck2Area = deckSlidersArea;

    // Layout Deck 1 sliders vertically
    int numSliders = 3;
    int sliderHeightDeck1 = deck1Area.getHeight() / numSliders;
    volSlider1.setBounds(deck1Area.removeFromTop(sliderHeightDeck1).reduced(10));
    speedSlider1.setBounds(deck1Area.removeFromTop(sliderHeightDeck1).reduced(10));
    posSlider1.setBounds(deck1Area.removeFromTop(sliderHeightDeck1).reduced(10));

    // Layout Deck 2 sliders vertically
    int sliderHeightDeck2 = deck2Area.getHeight() / numSliders;
    volSlider2.setBounds(deck2Area.removeFromTop(sliderHeightDeck2).reduced(10));
    speedSlider2.setBounds(deck2Area.removeFromTop(sliderHeightDeck2).reduced(10));
    posSlider2.setBounds(deck2Area.removeFromTop(sliderHeightDeck2).reduced(10));

    // Layout the crossfader slider and label
    int labelHeight = 20;
    auto crossSliderArea = crossfaderArea.removeFromTop(crossfaderArea.getHeight() - labelHeight);
    crossfaderSlider.setBounds(crossSliderArea.reduced(2));
    crossfaderLabel.setBounds(crossfaderArea.reduced(2));

    // Layout bottom section
    bottomPlaceholder.setBounds(bottomSection.reduced(10));
    auto placeholderArea = bottomPlaceholder.getLocalBounds();
    int numCols = 3;
    int numRows = 2;
    int cellWidth = placeholderArea.getWidth() / numCols;
    int cellHeight = placeholderArea.getHeight() / numRows;
    bottomButton1.setBounds(juce::Rectangle<int>(0, 0, cellWidth, cellHeight).reduced(5));
    bottomButton2.setBounds(juce::Rectangle<int>(cellWidth, 0, cellWidth, cellHeight).reduced(5));
    bottomButton3.setBounds(juce::Rectangle<int>(2 * cellWidth, 0, cellWidth, cellHeight).reduced(5));
    bottomButton4.setBounds(juce::Rectangle<int>(0, cellHeight, cellWidth, cellHeight).reduced(5));
    bottomButton5.setBounds(juce::Rectangle<int>(cellWidth, cellHeight, cellWidth, cellHeight).reduced(5));
    bottomButton6.setBounds(juce::Rectangle<int>(2 * cellWidth, cellHeight, cellWidth, cellHeight).reduced(5));
}

// Returns the number of rows in the track list for the table
int PlaylistComponent::getNumRows()
{
    return static_cast<int>(trackTitles.size());
}

// Paints the background of each table row
void PlaylistComponent::paintRowBackground(juce::Graphics& g, int rowNumber, int width, int height, bool rowIsSelected)
{
    g.fillAll(rowIsSelected ? juce::Colours::orange : juce::Colours::darkgrey);
}

// Paints the contents of a table cell; draws track titles
void PlaylistComponent::paintCell(juce::Graphics& g, int rowNumber, int columnId, int width, int height, bool rowIsSelected)
{
    if (columnId == 1)
        g.drawText(trackTitles[rowNumber], 2, 0, width - 4, height, juce::Justification::centredLeft, true);
}

// Updates the component used for the "Assign" cell
juce::Component* PlaylistComponent::refreshComponentForCell(int rowNumber, int columnId, bool isRowSelected, juce::Component* existingComponentToUpdate)
{
    if (columnId == 2)
    {
        // Creates a new TrackButtonsComponent if none exists
        if (existingComponentToUpdate == nullptr)
            existingComponentToUpdate = new TrackButtonsComponent(rowNumber, this);
        else if (auto* comp = dynamic_cast<TrackButtonsComponent*>(existingComponentToUpdate))
            comp->setRowId(rowNumber);  // Update the row ID if reusing an existing component.
    }
    return existingComponentToUpdate;
}

// Handles button click event for load and bottom button
void PlaylistComponent::buttonClicked(juce::Button* button)
{
    // If the load button is clicked, open a file chooser
    if (button == &loadButton)
    {
        auto* chooser = new juce::FileChooser("Select an audio file...",
            juce::File::getSpecialLocation(juce::File::userMusicDirectory),
            "*.wav;*.mp3;*.aiff");
        chooser->launchAsync(juce::FileBrowserComponent::openMode,
            [this, chooser](const juce::FileChooser& fc)
            {
                juce::File audioFile = fc.getResult();
                if (audioFile.existsAsFile())
                {
                    // Load selected file to left deck and add it to the track list
                    leftDeckGUI->loadFile(audioFile);
                    trackTitles.push_back(audioFile.getFileName().toStdString());
                    trackFiles.push_back(audioFile);
                    tableComponent.updateContent();
                }
                delete chooser;
            });
    }
    // For bottom buttons, load and play corresponding sounds from assets folder
    else if (button == &bottomButton1)
    {
        // Determine the file path for bottom buttons
        juce::File sourceDir(String(__FILE__));
        sourceDir = sourceDir.getParentDirectory();
        juce::File assetsDir = sourceDir.getChildFile("assets");
        juce::File soundFile = assetsDir.getChildFile("Drum 1.wav");

        DBG("Sound file path: " << soundFile.getFullPathName());
        if (soundFile.existsAsFile())
        {
            drumPlayer->loadURL(juce::URL(soundFile));
            drumPlayer->setGain(1.0);
            drumPlayer->start();
        }
        else
        {
            DBG("Sound file not found at: " << soundFile.getFullPathName());
            std::cout << "Sound file not found!" << std::endl;
        }
    }
    else if (button == &bottomButton2)
    {
        juce::File sourceDir(String(__FILE__));
        sourceDir = sourceDir.getParentDirectory();
        juce::File assetsDir = sourceDir.getChildFile("assets");
        juce::File soundFile = assetsDir.getChildFile("Vocal Sample 1.mp3");

        DBG("Sound file path: " << soundFile.getFullPathName());
        if (soundFile.existsAsFile())
        {
            drumPlayer->loadURL(juce::URL(soundFile));
            drumPlayer->setGain(1.0);
            drumPlayer->start();
        }
        else
        {
            DBG("Sound file not found at: " << soundFile.getFullPathName());
            std::cout << "Sound file not found!" << std::endl;
        }
    }
    else if (button == &bottomButton3)
    {
        juce::File sourceDir(String(__FILE__));
        sourceDir = sourceDir.getParentDirectory();
        juce::File assetsDir = sourceDir.getChildFile("assets");
        juce::File soundFile = assetsDir.getChildFile("Siren.mp3");

        DBG("Sound file path: " << soundFile.getFullPathName());
        if (soundFile.existsAsFile())
        {
            drumPlayer->loadURL(juce::URL(soundFile));
            drumPlayer->setGain(1.0);
            drumPlayer->start();
        }
        else
        {
            DBG("Sound file not found at: " << soundFile.getFullPathName());
            std::cout << "Sound file not found!" << std::endl;
        }
    }
    else if (button == &bottomButton4)
    {
        juce::File sourceDir(String(__FILE__));
        sourceDir = sourceDir.getParentDirectory();
        juce::File assetsDir = sourceDir.getChildFile("assets");
        juce::File soundFile = assetsDir.getChildFile("Drum 4.wav");

        DBG("Sound file path: " << soundFile.getFullPathName());
        if (soundFile.existsAsFile())
        {
            drumPlayer->loadURL(juce::URL(soundFile));
            drumPlayer->setGain(1.0);
            drumPlayer->start();
        }
        else
        {
            DBG("Sound file not found at: " << soundFile.getFullPathName());
            std::cout << "Sound file not found!" << std::endl;
        }
    }
    else if (button == &bottomButton5)
    {
        juce::File sourceDir(String(__FILE__));
        sourceDir = sourceDir.getParentDirectory();
        juce::File assetsDir = sourceDir.getChildFile("assets");
        juce::File soundFile = assetsDir.getChildFile("Glasses Up.mp3");

        DBG("Sound file path: " << soundFile.getFullPathName());
        if (soundFile.existsAsFile())
        {
            drumPlayer->loadURL(juce::URL(soundFile));
            drumPlayer->setGain(1.0);
            drumPlayer->start();
        }
        else
        {
            DBG("Sound file not found at: " << soundFile.getFullPathName());
            std::cout << "Sound file not found!" << std::endl;
        }
    }
    else if (button == &bottomButton6)
    {
        juce::File sourceDir(String(__FILE__));
        sourceDir = sourceDir.getParentDirectory();
        juce::File assetsDir = sourceDir.getChildFile("assets");
        juce::File soundFile = assetsDir.getChildFile("Airhorn.mp3");

        DBG("Sound file path: " << soundFile.getFullPathName());
        if (soundFile.existsAsFile())
        {
            drumPlayer->loadURL(juce::URL(soundFile));
            drumPlayer->setGain(1.0);
            drumPlayer->start();
        }
        else
        {
            DBG("Sound file not found at: " << soundFile.getFullPathName());
            std::cout << "Sound file not found!" << std::endl;
        }
    }
    else
    {
        std::cout << "Other button clicked." << std::endl;
    }
}

// Responds to slider changes
void PlaylistComponent::sliderValueChanged(juce::Slider* slider)
{
    // For volume or crossfader, update gains
    if (slider == &volSlider1 || slider == &volSlider2 || slider == &crossfaderSlider)
        updateGains();
    // For speed or vocal mix sliders, update respective deck
    else if (slider == &speedSlider1)
        deck1->setSpeed(slider->getValue());
    else if (slider == &posSlider1)
        deck1->setVocalMix(slider->getValue());
    else if (slider == &speedSlider2)
        deck2->setSpeed(slider->getValue());
    else if (slider == &posSlider2)
        deck2->setVocalMix(slider->getValue());
}

// Updates gain of each deck based on slider positions
void PlaylistComponent::updateGains()
{
    double crossVal = crossfaderSlider.getValue();
    double leftVol = volSlider1.getValue();
    double rightVol = volSlider2.getValue();
    // Deck gain decreases as the crossfader value increases for both decks
    deck1->setGain(leftVol * (1.0 - crossVal));
    deck2->setGain(rightVol * crossVal);
}

// Assigns a track from the track list to the left or right deck
void PlaylistComponent::assignTrackToDeck(int row, bool assignLeft)
{
    // Validate that the row index is within the available track list
    if (row >= 0 && row < static_cast<int>(trackFiles.size()))
    {
        juce::File file = trackFiles[row];
        if (file.existsAsFile())
        {
            if (assignLeft)
            {
                leftDeckGUI->loadFile(file);
                std::cout << "Assigned track " << trackTitles[row] << " to left deck." << std::endl;
            }
            else
            {
                rightDeckGUI->loadFile(file);
                std::cout << "Assigned track " << trackTitles[row] << " to right deck." << std::endl;
            }
        }
        else
        {
            std::cout << "No valid file for track " << trackTitles[row] << std::endl;
        }
    }
}
