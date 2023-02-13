/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
*/
class PitchShifterAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    PitchShifterAudioProcessor();
    ~PitchShifterAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    //==============================================================================
    float sawtooth(float t, float width);
    float interpolateSample(std::vector<float> delayBuffer, float writePointer, float currentDelay, float delayOffset, float sample_rate);
    
    // SET PARAMS
    void set_width_param(float val) { gSweepWidth_slider = val; }
    void set_frequency_param(float val) { gFrequency_slider = val; }
    void set_gVolume_param(float val) { gVolume_slider = val; }

    //==============================================================================
    // FOR PARAMETERS !
    juce::AudioProcessorValueTreeState apvts;
    
private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PitchShifterAudioProcessor)
    
    
    std::vector<float> gDelayBuffer;
    unsigned int gWritePointer = 0;
    unsigned int gOffset = 0; // this is only in case of negative delays.. ! not needed. hence 0
    //unsigned int gReadPointer = 0;

    double gSampleRate;
    
    float ph; // current LFO phase (between 0-1)
    float inverseSampleRate;

//    float mAvg = 0.002; // 2 ms
    float gMaxWidth = 0.6;
    
    
    int  noWindows = 3;
    std::vector<float> currentDelay_vec;
    std::vector<float> interpolatedSample_vec;
    std::vector<float> windowVal_vec;

    
    float sweepWidth;
    float sweepWidth_prev;

    float frequency;
    
    float gVolume_slider;
    float gFrequency_slider;
    float gFrequency_slider_smoo;
    float gFrequency_slider_smoo_prev;

    float gSweepWidth_slider;
    float gSweepWidth_slider_smoo;
    float gSweepWidth_slider_smoo_prev;
    
    int flagSwitch = 0;
    float countSwitch = 0;
    float countSwitchMax = 256;
    float countSwitchTime = 0.2; // s

    // AUDIO PARAMS
    juce::AudioProcessorValueTreeState::ParameterLayout createParameters()
    {
        std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;
        
        params.push_back(std::make_unique<AudioParameterFloat>("FREQUENCY","Frequency",-20.0f,20.0f,0.0));
        params.push_back(std::make_unique<AudioParameterFloat>("SWEEP_WIDTH","Sweep_Width",0.0005f,0.6f,0.05f));
        params.push_back(std::make_unique<AudioParameterFloat>("VOLUME","Volume",-30.0f,20.0f,0.0f)); // in dB

        return { params.begin(), params.end()};
    }
    
    
};
