/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
*/
class DaalSamAudioProcessor  : public AudioProcessor,
                               public ValueTree::Listener
{
public:
    //==============================================================================
    DaalSamAudioProcessor();
    ~DaalSamAudioProcessor();

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (AudioBuffer<float>&, MidiBuffer&) override;

    //==============================================================================
    AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const String getProgramName (int index) override;
    void changeProgramName (int index, const String& newName) override;

    //==============================================================================
    void getStateInformation (MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    //==============================================================================
    void valueTreePropertyChanged(ValueTree& treeWhosePropertyhasChanged, const Identifier& property) override;
    
    //==============================================================================
    void loadFile();
    void loadFile(const String& path);
    
    int getNumSamplerSounds() {return m_Sampler.getNumSounds(); }
    const AudioBuffer<float>& getWaveform() { return m_Waveform; }
    
    void updateADSR();
    
    ADSR::Parameters& getADSRParams() { return m_ADSRParams; }
    
    AudioProcessorValueTreeState& getValueTreeState() { return m_ValueTreeState; }

private:
    //==============================================================================
    Synthesiser m_Sampler;
    const int m_NumVoices {3};
    AudioBuffer<float> m_Waveform;
    
    AudioFormatManager m_FormatManager;
    AudioFormatReader* m_FormatReader {nullptr};
    
    ADSR::Parameters m_ADSRParams;
    
    AudioProcessorValueTreeState m_ValueTreeState;
    AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    
    std::atomic<bool> m_ShouldUpdate {false};
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DaalSamAudioProcessor)
};
