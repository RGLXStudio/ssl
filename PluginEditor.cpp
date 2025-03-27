#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SSLCompAudioProcessorEditor::SSLCompAudioProcessorEditor (SSLCompAudioProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p)
{
    // --- GUI Element Setup ---

    // Setup sliders using the helper function
    setupSlider(thresholdSlider, thresholdLabel, "Threshold", SSLCompAudioProcessor::PARAM_THRESHOLD, thresholdAttachment);
    setupSlider(ratioSlider, ratioLabel, "Ratio", SSLCompAudioProcessor::PARAM_RATIO, ratioAttachment, true); // Ratio is stepped
    setupSlider(attackSlider, attackLabel, "Attack", SSLCompAudioProcessor::PARAM_ATTACK, attackAttachment);
    setupSlider(releaseSlider, releaseLabel, "Release", SSLCompAudioProcessor::PARAM_RELEASE, releaseAttachment);
    setupSlider(makeupSlider, makeupLabel, "Makeup", SSLCompAudioProcessor::PARAM_MAKEUP, makeupAttachment);


    // Setup Bypass ("IN") Button
    bypassButton.setButtonText("IN");
    bypassButton.setClickingTogglesState(true); // Toggle button
    bypassButton.setColour(juce::TextButton::buttonColourId, juce::Colours::darkgrey); // Default off colour (Bypassed state)
    bypassButton.setColour(juce::TextButton::buttonOnColourId, juce::Colours::yellow); // "IN" state colour (Active state)
    bypassButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);    // Text colour when bypassed
    bypassButton.setColour(juce::TextButton::textColourOnId, juce::Colours::black);     // Text colour when "IN" (Active)

    // The bypass parameter is 'false' when active ("IN") and 'true' when bypassed.
    // ButtonAttachment correctly maps the button's toggle state (on/off) to the boolean parameter (true/false).
    // A toggled-ON button state corresponds to the parameter value 'true'.
    // So, when button is ON (yellow), parameter is 'true' (bypassed).
    // When button is OFF (grey), parameter is 'false' (active = "IN").
    // Let's adjust parameter definition default / button logic slightly if needed.
    // Our Bool parameter default is 'false' (active). ButtonAttachment maps button 'ON' state to parameter 'true'.
    // This means default state is button OFF (grey) = param false (Active). Button ON (yellow) = param true (Bypassed).
    // We want Yellow="IN"=Active. So we need to EITHER flip the parameter default/meaning OR flip the button colors.
    // Let's flip the colours to match the desired visual state: Yellow=ON=Active=ParamFalse
    // Re-setting colors based on parameter meaning (false=Active=IN)
    bypassButton.setColour(juce::TextButton::buttonOnColourId, juce::Colours::darkgrey); // ON state = Bypassed (Grey)
    bypassButton.setColour(juce::TextButton::buttonColourId, juce::Colours::yellow);   // OFF state = Active / IN (Yellow)
    bypassButton.setColour(juce::TextButton::textColourOnId, juce::Colours::white);      // Text colour when ON (Bypassed)
    bypassButton.setColour(juce::TextButton::textColourOffId, juce::Colours::black);     // Text colour when OFF (Active / IN)

    bypassAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        processorRef.parameters, SSLCompAudioProcessor::PARAM_BYPASS, bypassButton);

    addAndMakeVisible(bypassButton);

    // Add VU Meter
    addAndMakeVisible(vuMeter);

    // --- Editor Setup ---
    setSize (480, 200); // Adjusted size slightly
    startTimerHz(30); // Start timer for VU meter updates (e.g., 30 times per second)
}

SSLCompAudioProcessorEditor::~SSLCompAudioProcessorEditor()
{
    stopTimer(); // Stop the timer when the editor is destroyed
}

// Helper function to reduce code duplication
void SSLCompAudioProcessorEditor::setupSlider(juce::Slider& slider, juce::Label& label, const juce::String& text, const juce::StringRef paramID,
                                           std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>& attachment, bool isStepped)
{
    slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    // Correct arguments for setTextBoxStyle
    slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    slider.setColour(juce::Slider::thumbColourId, juce::Colours::lightgrey);
    slider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::cornflowerblue); // Example colour
    slider.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colours::darkgrey);
    addAndMakeVisible(slider);

    attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processorRef.parameters, paramID, slider);

    label.setText(text, juce::dontSendNotification);
    // Use updated Font constructor
    label.setFont(juce::Font(juce::FontOptions(12.0f)));
    label.setJustificationType(juce::Justification::centred);
    // Attach label below the slider
    label.attachToComponent(&slider, false);
    addAndMakeVisible(label);

    // Specific handling for stepped ratio slider (AudioParameterChoice)
    // No extra code needed here as SliderAttachment handles AudioParameterChoice steps correctly.
}


//==============================================================================
void SSLCompAudioProcessorEditor::paint (juce::Graphics& g)
{
    // Background
    g.fillAll (juce::Colour(0xff5a5a5a)); // SSL-ish grey background

    // Optional: Add plugin title or other static elements
    // g.setColour (juce::Colours::white);
    // g.setFont (juce::Font (juce::FontOptions(15.0f)));
    // g.drawFittedText ("SSL Style Compressor", getLocalBounds().reduced(10).removeFromTop(20), juce::Justification::centred, 1);
}

void SSLCompAudioProcessorEditor::resized()
{
    // Simple grid-like layout
    auto bounds = getLocalBounds().reduced(10); // Add some margin

    int sliderWidth = 70;
    int sliderHeight = 70;
    int labelHeight = 20; // Height reserved below slider for label
    int buttonSize = 50;
    int vuWidth = 25;
    // Make VU meter height roughly match slider + space for label
    int vuHeight = sliderHeight + labelHeight + 10; // Make slightly taller than slider+label space
    int spacing = 15; // Spacing between elements

    // --- Setup FlexBox for Sliders ---
    juce::FlexBox sliderFlexBox;
    sliderFlexBox.flexDirection = juce::FlexBox::Direction::row;
    // Use correct enum and value for FlexBox justification
    sliderFlexBox.justifyContent = juce::FlexBox::Justification::flexStart;
    sliderFlexBox.alignItems = juce::FlexBox::AlignItems::flexStart; // Align items top

    // Add sliders to the FlexBox
    // Margin: top margin accounts for label height (since label is attached below)
    juce::FlexItem::Margin sliderMargin(labelHeight, spacing, 0, 0); // top, left, bottom, right
    juce::FlexItem::Margin firstSliderMargin(labelHeight, 0, 0, 0); // No left margin for the first slider

    sliderFlexBox.items.add(juce::FlexItem(thresholdSlider).withWidth(sliderWidth).withHeight(sliderHeight).withMargin(firstSliderMargin));
    sliderFlexBox.items.add(juce::FlexItem(ratioSlider).withWidth(sliderWidth).withHeight(sliderHeight).withMargin(sliderMargin));
    sliderFlexBox.items.add(juce::FlexItem(attackSlider).withWidth(sliderWidth).withHeight(sliderHeight).withMargin(sliderMargin));
    sliderFlexBox.items.add(juce::FlexItem(releaseSlider).withWidth(sliderWidth).withHeight(sliderHeight).withMargin(sliderMargin));
    sliderFlexBox.items.add(juce::FlexItem(makeupSlider).withWidth(sliderWidth).withHeight(sliderHeight).withMargin(sliderMargin));

    // --- Calculate Areas ---
    int numSliders = 5;
    int totalSliderAreaWidth = (numSliders * sliderWidth) + ((numSliders -1) * spacing); // Width needed for sliders

    // Area for the sliders on the left
    auto sliderArea = bounds.removeFromLeft(totalSliderAreaWidth);

    // Remaining area for VU/Button on the right
    auto rightArea = bounds;
    // Add some padding between slider group and right group
    rightArea.removeFromLeft(spacing);

    // --- Perform Slider Layout ---
    // Perform layout within the designated sliderArea
    sliderFlexBox.performLayout(sliderArea.toFloat());

    // --- Layout VU meter and Bypass button in the rightArea ---
    // Vertically center the VU and Button group within the available rightArea height
    int maxHeight = std::max(vuHeight, buttonSize); // Use the taller of the two for centering calc
    // Center the combined VU + Button area vertically
    auto vuAndButtonArea = rightArea.withSizeKeepingCentre(rightArea.getWidth(), maxHeight);

    // Position VU Meter (align top within its centered vertical block)
    auto vuBounds = vuAndButtonArea.removeFromLeft(vuWidth).removeFromTop(vuHeight);
    vuMeter.setBounds(vuBounds);

    // Add space between VU and button
    vuAndButtonArea.removeFromLeft(spacing);

    // Position Bypass Button (centered vertically within its block)
    // Make it square and center it within the allocated space
    auto buttonBounds = vuAndButtonArea.removeFromLeft(buttonSize);
    bypassButton.setBounds(buttonBounds.withSizeKeepingCentre(buttonSize, buttonSize));

    // Note: Labels are automatically positioned by attachToComponent relative to the slider bounds
    // which were set by performLayout. No need for manual label positioning here.
}


//==============================================================================
void SSLCompAudioProcessorEditor::timerCallback()
{
    // Get the gain reduction from the processor and update the VU meter
    float gainReduction = processorRef.getCurrentGainReductionDb();
    vuMeter.setLevelDb(gainReduction);

    // Optional: Could force repaint of button if appearance depends on factors other than toggle state
    // bypassButton.repaint();
}