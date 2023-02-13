/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class PitchShifterAudioProcessorEditor  : public juce::AudioProcessorEditor,
public juce::Slider::Listener
{
public:
    PitchShifterAudioProcessorEditor (PitchShifterAudioProcessor&);
    ~PitchShifterAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    void sliderValueChanged (Slider* slider) override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    PitchShifterAudioProcessor& audioProcessor;
    
    
    Slider frequency_Slider;
    Label frequency_Label;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> frequency_SliderAttachment;

    Slider sweepWidth_Slider;
    Label sweepWidth_Label;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> sweepWidth_SliderAttachment;

    Slider vol_Slider;
    Label vol_Label;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> vol_SliderAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PitchShifterAudioProcessorEditor)
};
