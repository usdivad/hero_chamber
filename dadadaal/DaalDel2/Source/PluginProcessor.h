/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

//==============================================================================
/**
*/
class DaalDel2AudioProcessor  : public AudioProcessor,
                                public InterprocessConnection
{
public:
    //==============================================================================
    // Static variables
    static bool _hasInterprocessPipeBeenCreated;
    
    //==============================================================================
    DaalDel2AudioProcessor();
    ~DaalDel2AudioProcessor();

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
    // InterprocessConnection virtual methods
    void connectionMade() override;
    void connectionLost() override;
    void messageReceived(const MemoryBlock& message) override;
    
    //==============================================================================
    // Custom
    float lerp(float x0, float x1, float t); // t = "inPhase"
    bool createOrConnectToInterprocessPipe();
    String generateProcessName();
    float midiNoteToHz(float midiNote);
    // double fastPow(double a, double b); // Not accurate enough
    
private:
    //==============================================================================
    // Constants for delay
    const float _minDelayTime = 1.0; // Min and max time are basically MIDI values now
    const float _maxDelayTime = 127.0;
    const float _delayTimeSmoothAmount = 0.1; // Higher (0.1) for discrete (MIDI note), lower (0.001) if we're going more continuous (knob/MIDI CC)
    const float _delayTimeMultiplier = 0.0004;
    
    //==============================================================================
    // Circular buffer
    float* _circularBufferLeft;
    float* _circularBufferRight;
    int _circularBufferWriteHead;
    int _circularBufferLength;
    
    // Delay
    float _delayTimeInSamples;
    float _delayReadHead;
    float _delayTimeSmoothed;
    
    // Feedback
    float _feedbackLeft;
    float _feedbackRight;
    
    // MIDI
    // int _mostRecentMidiNote;
    

    //==============================================================================
    // Interprocess
    const int _interprocessCreatePipeTimeoutMs = -1;
    const int _interprocessConnectToPipeTimeoutMs = -1; // TODO: Figure out whether it's better to leave this as -1 (infinite tmieout) or to set it to disconnect/reconnect at e.g. 50-100ms
    const String _interprocessPipeName = "DAALDEL2_INTERPROCESS_PIPE_007";
    String _processName;
    bool _didCurrentInstanceCreateInterprocessPipe;
    
    //==============================================================================
    TrackProperties _trackProperties;
    
    //==============================================================================
    // User-controlled parameters
    AudioParameterFloat* _dryWetParam;
    AudioParameterFloat* _feedbackParam;
    AudioParameterFloat* _delayTimeParam;
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DaalDel2AudioProcessor)
};
