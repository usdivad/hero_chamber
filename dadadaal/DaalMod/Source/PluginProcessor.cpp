/*
 ==============================================================================
 
 This file was auto-generated!
 
 It contains the basic framework code for a JUCE plugin processor.
 
 ==============================================================================
 */

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
DaalModAudioProcessor::DaalModAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
: AudioProcessor (BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
                  .withInput  ("Input",  AudioChannelSet::stereo(), true)
#endif
                  .withOutput ("Output", AudioChannelSet::stereo(), true)
#endif
                  )
#endif
{
    // ========
    // Default values
    // Circular buffers are set to null pointers because we don't know sample rate yet, and thus don't know how to instantiate the audio data. These will be set in prepareToPlay()
    _circularBufferLeft = nullptr;
    _circularBufferRight = nullptr;
    _circularBufferWriteHead = 0;
    _circularBufferLength = 0;
    
    _feedbackLeft = 0;
    _feedbackRight = 0;
    
    _lfoPhase = 0;
    
    // ========
    // Parameters
    addParameter(_dryWetParam = new AudioParameterFloat("dryWet", "Dry/Wet", 0, 1, 0.5));
    addParameter(_depthParam = new AudioParameterFloat("depth", "Depth", 0, 1, 0.5));
    addParameter(_rateParam = new AudioParameterFloat("rate", "Rate", 0.1, 20, 10));
    addParameter(_phaseOffsetParam = new AudioParameterFloat("phaseOffset", "Phase Offset", 0, 1, 0));
    addParameter(_feedbackParam = new AudioParameterFloat("feedback", "Feedback", 0, 0.98, 0.5));
    // addParameter(_delayTimeParam = new AudioParameterFloat("delayTime", "Delay Time", 0.01, MAX_DELAY_TIME_IN_SECONDS, 0.5));
    addParameter(_typeParam = new AudioParameterInt("type", "Type", 0, 1, 0));
}

DaalModAudioProcessor::~DaalModAudioProcessor()
{
    // Free up memory for circular buffers if necessary
    if (_circularBufferLeft != nullptr) {
        delete [] _circularBufferLeft;
        _circularBufferLeft = nullptr;
    }
    if (_circularBufferRight != nullptr) {
        delete [] _circularBufferRight;
        _circularBufferRight = nullptr;
    }
}

//==============================================================================
const String DaalModAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool DaalModAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool DaalModAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool DaalModAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double DaalModAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int DaalModAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
    // so this should be at least 1, even if you're not really implementing programs.
}

int DaalModAudioProcessor::getCurrentProgram()
{
    return 0;
}

void DaalModAudioProcessor::setCurrentProgram (int index)
{
}

const String DaalModAudioProcessor::getProgramName (int index)
{
    return {};
}

void DaalModAudioProcessor::changeProgramName (int index, const String& newName)
{
}

//==============================================================================
void DaalModAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    
    // LFO
    _lfoPhase = 0;
    
    // Calculate circular buffer length based on sample rate
    _circularBufferLength = (int)(sampleRate * MAX_DELAY_TIME_IN_SECONDS);
    
    // Initialize circular buffers based on sample rate and delay time
    // The nullptr setting allows for changing sample rates in the middle of a session
    if (_circularBufferLeft != nullptr) { // Left
        delete [] _circularBufferLeft;
        _circularBufferLeft = nullptr;
    }
    if (_circularBufferLeft == nullptr) {
        _circularBufferLeft = new float[_circularBufferLength];
    }
    zeromem(_circularBufferLeft, _circularBufferLength * sizeof(float)); // Clear junk data
    
    if (_circularBufferRight != nullptr) { // Right
        delete [] _circularBufferRight;
        _circularBufferRight = nullptr;
    }
    if (_circularBufferRight == nullptr) {
        _circularBufferRight = new float[_circularBufferLength];
    }
    zeromem(_circularBufferLeft, _circularBufferLength * sizeof(float));
    
    // Initialize write head
    _circularBufferWriteHead = 0;
}

void DaalModAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool DaalModAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
    ignoreUnused (layouts);
    return true;
#else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != AudioChannelSet::stereo())
        return false;
    
    // This checks if the input layout matches the output layout
#if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
#endif
    
    return true;
#endif
}
#endif

void DaalModAudioProcessor::processBlock (AudioBuffer<float>& buffer, MidiBuffer& midiMessages)
{
    ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    
    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
    
    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.
    //     for (int channel = 0; channel < totalNumInputChannels; ++channel)
    //     {
    //         auto* channelData = buffer.getWritePointer (channel);
    //
    //         // ..do something to the data...
    //     }
    
    // Get write pointers for left and right channels
    float* leftChannel = buffer.getWritePointer(0);
    float* rightChannel = buffer.getWritePointer(1);
    
    // Debug printing
    // DBG("Dry/Wet: " <<  _dryWetParam->get());
    // DBG("Depth: " <<  _depthParam->get());
    // DBG("Rate: " <<  _rateParam->get());
    // DBG("Phase Offset: " <<  _phaseOffsetParam->get());
    // DBG("Feedback: " <<  _feedbackParam->get());
    // DBG("Type: " <<  _typeParam->get());
    
    // Process audio by iterating through all the samples in the buffer
    for (int i=0; i<buffer.getNumSamples(); i++) {
        // ========
        // Write samples to circular buffer (and also add feedback)
        _circularBufferLeft[_circularBufferWriteHead] = leftChannel[i] + _feedbackLeft;
        _circularBufferRight[_circularBufferWriteHead] = rightChannel[i] + _feedbackRight;
        
        // ========
        // Setup LFO for left channel
        
        // Generate LFO output
        float lfoOutLeft = sin(2 * M_PI * _lfoPhase); // Formula for LFO
        
        
        
        // ========
        // Setup LFO for right channel
        
        // Calculate the phase first
        float lfoPhaseRight = _lfoPhase + _phaseOffsetParam->get(); // Set based on phase and offset param
        if (lfoPhaseRight > 1) { // Keep range between 0-1
            lfoPhaseRight -= 1;
        }
        
        // And then do the actual LFO output value generation
        float lfoOutRight = sin(2 * M_PI * lfoPhaseRight);
        
        
        // ========
        // Update LFO phase (moving it forward)
        _lfoPhase += _rateParam->get() / getSampleRate(); // Increment phase based on rate
        if (_lfoPhase > 1) { // Keep range between 0-1
            _lfoPhase -= 1;
        }
        
        // ========
        // Apply depth param to LFO output values
        lfoOutLeft *= _depthParam->get();
        lfoOutRight *= _depthParam->get();
        
        
        // ========
        // Map LFO output value to min and max delay time values
        // depending on the type (chorus/flanger)
        
        float lfoOutMappedLeft = 0;
        float lfoOutMappedRight = 0;
        
        if (_typeParam->get() == 0) { // Chorus
            lfoOutMappedLeft = jmap(lfoOutLeft, -1.0f, 1.0f, CHORUS_LFO_OUT_MIN_IN_SECONDS, CHORUS_LFO_OUT_MAX_IN_SECONDS); // Map left
            lfoOutMappedRight = jmap(lfoOutRight, -1.0f, 1.0f, CHORUS_LFO_OUT_MIN_IN_SECONDS, CHORUS_LFO_OUT_MAX_IN_SECONDS); // Map right
        }
        else { // Flanger
            lfoOutMappedLeft = jmap(lfoOutLeft, -1.0f, 1.0f, FLANGER_LFO_OUT_MIN_IN_SECONDS, FLANGER_LFO_OUT_MAX_IN_SECONDS); // Map left
            lfoOutMappedRight = jmap(lfoOutRight, -1.0f, 1.0f, FLANGER_LFO_OUT_MIN_IN_SECONDS, FLANGER_LFO_OUT_MAX_IN_SECONDS); // Map right
        }
        
        
        // ========
        // Calculate delay time in samples
        // Update delay time in case sample rate has changed
        // We also now need to update since we're using the LFO
        // (Also, we use LFO directly, instead of a smoothed delay var, since it's already smooth)
        float delayTimeInSamplesLeft = getSampleRate() * lfoOutMappedLeft;
        float delayTimeInSamplesRight = getSampleRate() * lfoOutMappedRight;


        // ========
        // Read and generate delay for both left and right channels
        
        // Read from delayed position in buffer (calculate read head position)
        float delayReadHeadLeft = _circularBufferWriteHead - delayTimeInSamplesLeft;
        if (delayReadHeadLeft < 0) {
            delayReadHeadLeft += _circularBufferLength;
        }
        float delayReadHeadRight = _circularBufferWriteHead - delayTimeInSamplesRight;
        if (delayReadHeadRight < 0) {
            delayReadHeadRight += _circularBufferLength;
        }
        
        // Lerp!
        int delayReadHeadIntX0Left = (int) delayReadHeadLeft; // x0
        float delayReadHeadRemainderX0Left = delayReadHeadLeft - delayReadHeadIntX0Left; // t, i.e. inPhase
        int delayReadHeadIntX1Left = delayReadHeadIntX0Left + 1; // x1
        if (delayReadHeadIntX1Left >= _circularBufferLength) {
            delayReadHeadIntX1Left -= _circularBufferLength;
        }
        
        int delayReadHeadIntX0Right = (int) delayReadHeadRight; // x0
        float delayReadHeadRemainderX0Right = delayReadHeadRight - delayReadHeadIntX0Right; // t, i.e. inPhase
        int delayReadHeadIntX1Right = delayReadHeadIntX0Right + 1; // x1
        if (delayReadHeadIntX1Right >= _circularBufferLength) {
            delayReadHeadIntX1Right -= _circularBufferLength;
        }
        
        // Generate current output delay sample for applying feedback
        float delaySampleLeft = lerp(_circularBufferLeft[(int)delayReadHeadIntX0Left],
                                     _circularBufferLeft[(int)delayReadHeadIntX1Left],
                                     delayReadHeadRemainderX0Left);
        float delaySampleRight = lerp(_circularBufferRight[(int)delayReadHeadIntX0Right],
                                      _circularBufferRight[(int)delayReadHeadIntX1Right],
                                      delayReadHeadRemainderX0Right);
        
        // Apply feedback (for next iteration)
        _feedbackLeft = delaySampleLeft * _feedbackParam->get();
        _feedbackRight = delaySampleRight * _feedbackParam->get();
        
        // Increment write head
        // _circularBufferWriteHead = (_circularBufferWriteHead + 1) % _circularBufferLength;
        _circularBufferWriteHead++;
        if (_circularBufferWriteHead >= _circularBufferLength) {
            _circularBufferWriteHead = 0;
        }
        
        // Sum the dry and wet (delayed) samples
        float dryAmount = 1 - _dryWetParam->get();
        float wetAmount = _dryWetParam->get();
        
        buffer.setSample(0, i, (buffer.getSample(0, i) * dryAmount) +
                         (delaySampleLeft * wetAmount));
        buffer.setSample(1, i, (buffer.getSample(1, i) * dryAmount) +
                         (delaySampleLeft * wetAmount));
    }
}

//==============================================================================
bool DaalModAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* DaalModAudioProcessor::createEditor()
{
    return new DaalModAudioProcessorEditor (*this);
}

//==============================================================================
void DaalModAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    
    // Create XML
    std::unique_ptr<XmlElement> xml(new XmlElement("DaalMod"));
    
    // Populate with params
    xml->setAttribute("DryWet", _dryWetParam->get());
    xml->setAttribute("Depth", _depthParam->get());
    xml->setAttribute("Rate", _rateParam->get());
    xml->setAttribute("PhaseOffset", _phaseOffsetParam->get());
    xml->setAttribute("Feedback", _feedbackParam->get());
    xml->setAttribute("Type", _typeParam->get());
    
    // Copy the XML data to destination blob
    copyXmlToBinary(*xml, destData);
    
}

void DaalModAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    
    // Create XML from state data
    std::unique_ptr<XmlElement> xml(getXmlFromBinary(data, sizeInBytes));
    
    // Set params based on state
    if (xml.get() != nullptr && xml->hasTagName("DaalMod")) {
        *_dryWetParam = xml->getDoubleAttribute("DryWet");
        *_depthParam = xml->getDoubleAttribute("Depth");
        *_rateParam = xml->getDoubleAttribute("Rate");
        *_phaseOffsetParam = xml->getDoubleAttribute("PhaseOffset");
        *_feedbackParam = xml->getDoubleAttribute("Feedback");
        *_typeParam = xml->getIntAttribute("Type");
    }
}

//==============================================================================
float DaalModAudioProcessor::lerp(float x0, float x1, float t)
{
    return (1 - t) * x0 + t * x1;
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new DaalModAudioProcessor();
}
