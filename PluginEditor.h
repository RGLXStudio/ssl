#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
// Custom knob style LookAndFeel class
class SSLLookAndFeel : public juce::LookAndFeel_V4
{
public:
    SSLLookAndFeel();
    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                         float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                         juce::Slider& slider) override;
private:
    juce::Image knobImage;
};

//==============================================================================
class SSLCompressorAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    SSLCompressorAudioProcessorEditor (SSLCompressorAudioProcessor&);
    ~SSLCompressorAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    SSLCompressorAudioProcessor& audioProcessor;
    SSLLookAndFeel sslLookAndFeel;

    // Knobs
    juce::Slider thresholdKnob;
    juce::Slider ratioKnob;
    juce::Slider attackKnob;
    juce::Slider releaseKnob;
    juce::Slider makeupKnob;

    // Labels
    juce::Label thresholdLabel;
    juce::Label ratioLabel;
    juce::Label attackLabel;
    juce::Label releaseLabel;
    juce::Label makeupLabel;
    juce::Label titleLabel;

    // Attachments for parameter control
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> thresholdAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> ratioAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attackAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> releaseAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> makeupAttachment;

    // Gain reduction meter
    float gainReduction;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SSLCompressorAudioProcessorEditor)
};