/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
PitchShifterAudioProcessorEditor::PitchShifterAudioProcessorEditor (PitchShifterAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (400, 300);
    
    addAndMakeVisible(frequency_Slider);
    frequency_Slider.setTextValueSuffix(" [Hz]");
    frequency_Slider.addListener(this);
    frequency_Slider.setRange(-20.0,20.0);
    frequency_Slider.setValue(10.0);
    addAndMakeVisible(frequency_Label);
    frequency_Label.setText("Frequency", juce::dontSendNotification);
    frequency_Label.attachToComponent(&frequency_Slider, true);
    
    frequency_SliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts,"FREQUENCY",frequency_Slider);
    
    audioProcessor.set_frequency_param(frequency_Slider.getValue());

    
    addAndMakeVisible(sweepWidth_Slider);
    sweepWidth_Slider.setTextValueSuffix(" [-]");
    sweepWidth_Slider.addListener(this);
    sweepWidth_Slider.setRange(0.0005,0.6);
    sweepWidth_Slider.setValue(0.05);
    addAndMakeVisible(sweepWidth_Label);
    sweepWidth_Label.setText("Sweep Width", juce::dontSendNotification);
    sweepWidth_Label.attachToComponent(&sweepWidth_Slider, true);

    sweepWidth_SliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts,"SWEEP_WIDTH",sweepWidth_Slider);
    
    audioProcessor.set_width_param(sweepWidth_Slider.getValue());
    
    addAndMakeVisible(vol_Slider);
    vol_Slider.setTextValueSuffix(" [-]");
    vol_Slider.addListener(this);
    vol_Slider.setRange(-30.0,20.0);
    vol_Slider.setValue(0.0);
    addAndMakeVisible(vol_Label);
    vol_Label.setText("Volume", juce::dontSendNotification);
    vol_Label.attachToComponent(&vol_Slider, true);
    
    vol_SliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts,"VOLUME",vol_Slider);
    
    audioProcessor.set_gVolume_param(vol_Slider.getValue());
    
    int ana = 3;

}

PitchShifterAudioProcessorEditor::~PitchShifterAudioProcessorEditor()
{
}

//==============================================================================
void PitchShifterAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

//    g.setColour (juce::Colours::white);
//    g.setFont (15.0f);
//    g.drawFittedText ("Hello World!", getLocalBounds(), juce::Justification::centred, 1);
}

void PitchShifterAudioProcessorEditor::resized()
{
    auto sliderLeft = 120;

    frequency_Slider.setBounds(sliderLeft, 20, getWidth() - sliderLeft - 10, 20);
    sweepWidth_Slider.setBounds(sliderLeft, 20+40, getWidth() - sliderLeft - 10, 20);
    vol_Slider.setBounds(sliderLeft, 20+40+40, getWidth() - sliderLeft - 10, 20);
    
}


void PitchShifterAudioProcessorEditor::sliderValueChanged(Slider* slider)
{
    if (slider == &frequency_Slider)
    {
        audioProcessor.set_frequency_param(frequency_Slider.getValue());
    }
    else if (slider == &sweepWidth_Slider)
    {
        audioProcessor.set_width_param(sweepWidth_Slider.getValue());
    }
    else if (slider == &vol_Slider)
    {
        float volValue = pow(10,(vol_Slider.getValue()/20));
        audioProcessor.set_gVolume_param(volValue);
//        audioProcessor.set_gVolume_param(vol_Slider.getValue());
    }
}

