#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SSLCompressorAudioProcessor::SSLCompressorAudioProcessor()
    : AudioProcessor(BusesProperties()
                    .withInput("Input", juce::AudioChannelSet::stereo(), true)
                    .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, "Parameters", createParameterLayout())
{
    // Initialize compressor parameters
    addParameter(threshold = new juce::AudioParameterFloat("threshold", 
                                                         "Threshold", 
                                                         -60.0f, 
                                                         0.0f, 
                                                         -20.0f));
                                                         
    addParameter(ratio = new juce::AudioParameterFloat("ratio",
                                                     "Ratio",
                                                     1.0f,
                                                     10.0f,
                                                     4.0f));
                                                     
    addParameter(attack = new juce::AudioParameterFloat("attack",
                                                      "Attack",
                                                      0.1f,
                                                      100.0f,
                                                      10.0f));
                                                      
    addParameter(release = new juce::AudioParameterFloat("release",
                                                       "Release",
                                                       10.0f,
                                                       1000.0f,
                                                       100.0f));
                                                       
    addParameter(makeupGain = new juce::AudioParameterFloat("makeup",
                                                          "Makeup Gain",
                                                          0.0f,
                                                          20.0f,
                                                          0.0f));
                                                          
    currentGainReduction = 0.0f;
    envelopeDetector = 0.0f;
}

juce::AudioProcessorValueTreeState::ParameterLayout SSLCompressorAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back(std::make_unique<juce::AudioParameterFloat>("threshold", 
                                                               "Threshold",
                                                               -60.0f,
                                                               0.0f,
                                                               -20.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>("ratio",
                                                               "Ratio",
                                                               1.0f,
                                                               10.0f,
                                                               4.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>("attack",
                                                               "Attack",
                                                               0.1f,
                                                               100.0f,
                                                               10.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>("release",
                                                               "Release",
                                                               10.0f,
                                                               1000.0f,
                                                               100.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>("makeup",
                                                               "Makeup",
                                                               0.0f,
                                                               20.0f,
                                                               0.0f));

    return { params.begin(), params.end() };
}

SSLCompressorAudioProcessor::~SSLCompressorAudioProcessor()
{
}

//==============================================================================
void SSLCompressorAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Initialize processing variables
    this->sampleRate = sampleRate;
    currentGainReduction = 0.0f;
    envelopeDetector = 0.0f;
}

void SSLCompressorAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    
    const int numChannels = buffer.getNumChannels();
    const int numSamples = buffer.getNumSamples();
    
    // Calculate time constants
    const float attackTime = *attack / 1000.0f;  // Convert to seconds
    const float releaseTime = *release / 1000.0f;
    
    const float attackCoeff = std::exp(-1.0f / (sampleRate * attackTime));
    const float releaseCoeff = std::exp(-1.0f / (sampleRate * releaseTime));
    
    // Process each sample
    for (int sample = 0; sample < numSamples; ++sample)
    {
        // Find maximum absolute value across all channels (for stereo linking)
        float inputLevel = 0.0f;
        for (int channel = 0; channel < numChannels; ++channel)
        {
            float* channelData = buffer.getWritePointer(channel);
            inputLevel = std::max(inputLevel, std::abs(channelData[sample]));
        }
        
        // Convert to dB
        float inputLevelDB = 20.0f * std::log10(inputLevel + 1e-6f);
        
        // Calculate gain reduction amount
        float gainReductionDB = 0.0f;
        if (inputLevelDB > *threshold)
        {
            gainReductionDB = (*threshold + (inputLevelDB - *threshold) / *ratio) - inputLevelDB;
        }
        
        // Smooth the gain reduction using envelope detector
        if (gainReductionDB < envelopeDetector)
            envelopeDetector = attackCoeff * envelopeDetector + (1.0f - attackCoeff) * gainReductionDB;
        else
            envelopeDetector = releaseCoeff * envelopeDetector + (1.0f - releaseCoeff) * gainReductionDB;
        
        // Convert back to linear gain
        float gainReduction = std::pow(10.0f, (envelopeDetector + *makeupGain) / 20.0f);
        
        // Apply gain reduction to all channels
        for (int channel = 0; channel < numChannels; ++channel)
        {
            float* channelData = buffer.getWritePointer(channel);
            channelData[sample] *= gainReduction;
        }
    }
}

//==============================================================================
juce::AudioProcessorEditor* SSLCompressorAudioProcessor::createEditor()
{
    return new juce::GenericAudioProcessorEditor(*this);
}

bool SSLCompressorAudioProcessor::hasEditor() const
{
    return true;
}

//==============================================================================
const juce::String SSLCompressorAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool SSLCompressorAudioProcessor::acceptsMidi() const
{
    return false;
}

bool SSLCompressorAudioProcessor::producesMidi() const
{
    return false;
}

bool SSLCompressorAudioProcessor::isMidiEffect() const
{
    return false;
}

double SSLCompressorAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

//==============================================================================
int SSLCompressorAudioProcessor::getNumPrograms()
{
    return 1;
}

int SSLCompressorAudioProcessor::getCurrentProgram()
{
    return 0;
}

void SSLCompressorAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String SSLCompressorAudioProcessor::getProgramName (int index)
{
    return {};
}

void SSLCompressorAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void SSLCompressorAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // Store parameter values
}

void SSLCompressorAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // Restore parameter values
}