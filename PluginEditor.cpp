#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SSLLookAndFeel::SSLLookAndFeel()
{
    // Here you would load a custom knob image if you have one
    // For now we'll create a basic SSL-style knob look
}

void SSLLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                                     float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                                     juce::Slider& slider)
{
    // Basic colors
    const auto outline = juce::Colour(0xFF374554);
    const auto fill = juce::Colour(0xFF6C788C);
    const auto pointer = juce::Colour(0xFFEEEEEE);

    auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat().reduced(2);
    auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) / 2.0f;
    auto toAngle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
    auto lineW = radius * 0.2f;
    auto arcRadius = radius - lineW * 0.5f;

    // Draw knob body
    g.setColour(outline);
    g.fillEllipse(bounds);
    
    // Draw indicator arc
    juce::Path backgroundArc;
    backgroundArc.addCentredArc(bounds.getCentreX(),
                               bounds.getCentreY(),
                               arcRadius,
                               arcRadius,
                               0.0f,
                               rotaryStartAngle,
                               rotaryEndAngle,
                               true);

    g.setColour(fill);
    g.strokePath(backgroundArc, juce::PathStrokeType(lineW, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // Draw pointer
    juce::Path p;
    auto pointerLength = radius * 0.8f;
    auto pointerThickness = lineW * 0.8f;
    
    p.addRectangle(-pointerThickness * 0.5f, -radius + lineW, pointerThickness, pointerLength);
    p.applyTransform(juce::AffineTransform::rotation(toAngle).translated(bounds.getCentreX(), bounds.getCentreY()));
    g.setColour(pointer);
    g.fillPath(p);
}

//==============================================================================
SSLCompressorAudioProcessorEditor::SSLCompressorAudioProcessorEditor (SSLCompressorAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Set SSL look and feel
    setLookAndFeel(&sslLookAndFeel);
    
    // Make the window resizable and set size
    setResizable(true, true);
    setSize (600, 300);

    // Configure knobs
    auto setupKnob = [this](juce::Slider& knob, const juce::String& suffix) {
        knob.setSliderStyle(juce::Slider::RotaryVerticalDrag);
        knob.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
        knob.setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);
        knob.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
        knob.setLookAndFeel(&sslLookAndFeel);
        knob.setTextValueSuffix(suffix);
        addAndMakeVisible(knob);
    };

    // Configure labels
    auto setupLabel = [this](juce::Label& label, const juce::String& text) {
        label.setText(text, juce::dontSendNotification);
        label.setFont(juce::Font(14.0f, juce::Font::bold));
        label.setJustificationType(juce::Justification::centred);
        label.setColour(juce::Label::textColourId, juce::Colours::white);
        addAndMakeVisible(label);
    };

    // Setup all knobs
    setupKnob(thresholdKnob, " dB");
    setupKnob(ratioKnob, ":1");
    setupKnob(attackKnob, " ms");
    setupKnob(releaseKnob, " ms");
    setupKnob(makeupKnob, " dB");

    // Setup all labels
    setupLabel(thresholdLabel, "THRESH");
    setupLabel(ratioLabel, "RATIO");
    setupLabel(attackLabel, "ATTACK");
    setupLabel(releaseLabel, "RELEASE");
    setupLabel(makeupLabel, "MAKEUP");
    
    // Setup title label
    titleLabel.setText("SSL STYLE COMPRESSOR", juce::dontSendNotification);
    titleLabel.setFont(juce::Font(18.0f, juce::Font::bold));
    titleLabel.setJustificationType(juce::Justification::centred);
    titleLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(titleLabel);

    // Make editor non-resizable below minimum size
    setResizeLimits(400, 200, 800, 400);
}

SSLCompressorAudioProcessorEditor::~SSLCompressorAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

//==============================================================================
void SSLCompressorAudioProcessorEditor::paint (juce::Graphics& g)
{
    // Fill background
    g.fillAll(juce::Colour(0xFF262626));

    // Draw border
    g.setColour(juce::Colour(0xFF374554));
    g.drawRect(getLocalBounds(), 2);

    // Draw gain reduction meter
    auto meterBounds = getLocalBounds().removeFromTop(40).reduced(10, 5);
    g.setColour(juce::Colours::darkgrey);
    g.fillRect(meterBounds);
    
    float meterWidth = (gainReduction / 20.0f) * meterBounds.getWidth(); // Assuming max GR of 20dB
    g.setColour(juce::Colours::orange);
    g.fillRect(meterBounds.removeFromLeft((int)meterWidth));
}

void SSLCompressorAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();
    
    // Title area
    titleLabel.setBounds(bounds.removeFromTop(40));
    
    // Meter area
    bounds.removeFromTop(40); // Space for GR meter
    
    // Control area
    bounds.reduce(20, 20);
    int knobSize = juce::jmin(bounds.getWidth() / 5, bounds.getHeight() / 2);
    
    auto setupControl = [knobSize](juce::Slider& knob, juce::Label& label, juce::Rectangle<int>& area) {
        auto controlBounds = area.removeFromLeft(knobSize).reduced(10);
        knob.setBounds(controlBounds);
        label.setBounds(controlBounds.removeFromBottom(20));
    };

    setupControl(thresholdKnob, thresholdLabel, bounds);
    setupControl(ratioKnob, ratioLabel, bounds);
    setupControl(attackKnob, attackLabel, bounds);
    setupControl(releaseKnob, releaseLabel, bounds);
    setupControl(makeupKnob, makeupLabel, bounds);
}