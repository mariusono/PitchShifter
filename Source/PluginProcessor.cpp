/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"


//==============================================================================

float PitchShifterAudioProcessor::sawtooth(float t, float width)
{
    float twoPi = 2*float_Pi;
    float rt = fmodf(t,twoPi) / twoPi;
    float y = t;
    
    float c1 = 0;
    float c2 = 0;
    if ((width > 0) && (width < 1))
    {
        c1 = 2/width;
        c2 = 2/(1 - width);
    }
    else
    {
        c1 = 2.0f;
        c2 = c1;
    }
    
    if (rt>0)
    {
        if (rt>width)
        {
            y = (-(t<0) - rt + 1 - 0.5*(1-width))*c2;
        }
        else
        {
            y = ( (t<0) + rt - 0.5*width)*c1;
        }
    }
    else if (rt<0)
    {
        if (rt<width-1)
        {
            y = ( (t<0) + rt - 0.5*width)*c1;
        }
        else
        {
            y = (-(t<0) - rt + 1 - 0.5*(1-width))*c2;
        }
    }
    else if (width > 0)
    {
        y = (rt - 0.5*width)*c1;
    }
    else
    {
        y = 1;
    }
    
//    y = rt;
    
    return y;
}

float PitchShifterAudioProcessor::interpolateSample(std::vector<float> delayBuffer, float writePointer, float currentDelay, float delayOffset, float sample_rate)
{

    float readPointer = fmodf((float)writePointer - (float)(currentDelay * sample_rate) + (float)delayBuffer.size() - delayOffset, (float)delayBuffer.size());

    // Cubic interpolation
    int index = (int)floorf(readPointer);
    float alpha = readPointer - index;

    int index_m1 = (index-1) % delayBuffer.size();
    int index_p1 = (index+1) % delayBuffer.size();
    int index_p2 = (index+2) % delayBuffer.size();

    float interpolatedSample = ( alpha*(alpha-1)*(alpha-2)*delayBuffer[index_m1]/(-6)
                             + (alpha-1)*(alpha+1)*(alpha-2)*delayBuffer[index]/2
                             + alpha*(alpha+1)*(alpha-2)*delayBuffer[index_p1]/(-2)
                             + alpha*(alpha+1)*(alpha-1)*delayBuffer[index_p2]/(6) );
    
//    float interpolatedSample = 0;
    return interpolatedSample;
}

//==============================================================================
PitchShifterAudioProcessor::PitchShifterAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ), apvts(*this, nullptr, "Parameters", createParameters())
#endif
{
}

PitchShifterAudioProcessor::~PitchShifterAudioProcessor()
{
}

//==============================================================================
const juce::String PitchShifterAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool PitchShifterAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool PitchShifterAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool PitchShifterAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double PitchShifterAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int PitchShifterAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int PitchShifterAudioProcessor::getCurrentProgram()
{
    return 0;
}

void PitchShifterAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String PitchShifterAudioProcessor::getProgramName (int index)
{
    return {};
}

void PitchShifterAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void PitchShifterAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
//    gDelayBuffer.resize(ceil(gMaxWidth*2 * sampleRate));
    gDelayBuffer.resize(ceil(10 * sampleRate));
    gOffset = 0;
    
    gSampleRate = sampleRate;
    
    inverseSampleRate = 1/sampleRate;
    
//    noWindows = 10;
    
    currentDelay_vec.resize(noWindows,0);
    interpolatedSample_vec.resize(noWindows,0);
    windowVal_vec.resize(noWindows,0);

    
    sweepWidth = 0.05;
    sweepWidth_prev = 0.05;

    frequency = 0;
    
    gFrequency_slider = 0;
    gSweepWidth_slider = 0.05;
    gVolume_slider = 1.0;
    
    
    gFrequency_slider_smoo = 0;
    gFrequency_slider_smoo_prev = 0;
    
    gSweepWidth_slider_smoo = 0.05;
    gSweepWidth_slider_smoo_prev = 0.05;
    
    ph = 0; // phase increment
    
    flagSwitch = 0;
    countSwitchMax = floorf(countSwitchTime*(float)sampleRate);
}

void PitchShifterAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool PitchShifterAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
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

void PitchShifterAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    float* const outputL = buffer.getWritePointer(0);
    float* const outputR = buffer.getWritePointer(1);

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
    {
        buffer.clear (i, 0, buffer.getNumSamples());
    }

    for (int i = 0; i < buffer.getNumSamples(); ++i)
    {
        for (int channel = 0; channel < 2; ++channel)
        {
            if (channel == 0)
            {
                auto* input = buffer.getWritePointer (channel);
                
                // Current input sample
                float in = input[i];
                
                float interpolatedSample_result = 0;
                float interpolatedSample_prev = 0;
                float interpolatedSample_comb  = 0;
                
                gFrequency_slider_smoo = (1-0.95)*gFrequency_slider + 0.95*gFrequency_slider_smoo_prev;
                gFrequency_slider_smoo_prev = gFrequency_slider_smoo;
                
                frequency = gFrequency_slider_smoo;
                
                gSweepWidth_slider_smoo = (1-0.95)*gSweepWidth_slider + 0.95*gSweepWidth_slider_smoo_prev;
                gSweepWidth_slider_smoo_prev = gSweepWidth_slider_smoo;
                                
                if (flagSwitch == 0)
                {
//                    if (gSweepWidth_slider != sweepWidth)
                    if (gSweepWidth_slider_smoo != sweepWidth)
                    {
                        flagSwitch = 1;
                        sweepWidth_prev = sweepWidth;
//                        sweepWidth = gSweepWidth_slider;
                        sweepWidth = gSweepWidth_slider_smoo;
                    }
                }
                
// First version just minimizes the initial delay. Perhaps try to offset the sin wave, i.e. change phase so that is starts at a value where it is 0 !

                for (int iWin = 0; iWin<noWindows; iWin++)
                {
                    float currentDelay = sweepWidth * (0.5 + 0.5 * sawtooth(2.0 * M_PI * ph + iWin * 2*M_PI/(noWindows),1.0f)); // ph is f/fs

                    float readPointer = fmodf((float)gWritePointer - (float)(currentDelay * gSampleRate) + (float)gDelayBuffer.size() - gOffset, (float)gDelayBuffer.size());

                    // Cubic interpolation
                    int index = (int)floorf(readPointer);
                    float alpha = readPointer - index;

                    int index_m1 = (index-1) % gDelayBuffer.size();
                    int index_p1 = (index+1) % gDelayBuffer.size();
                    int index_p2 = (index+2) % gDelayBuffer.size();

                    float interpolatedSample_val = ( alpha*(alpha-1)*(alpha-2)*gDelayBuffer[index_m1]/(-6)
                                             + (alpha-1)*(alpha+1)*(alpha-2)*gDelayBuffer[index]/2
                                             + alpha*(alpha+1)*(alpha-2)*gDelayBuffer[index_p1]/(-2)
                                             + alpha*(alpha+1)*(alpha-1)*gDelayBuffer[index_p2]/(6) );
                    
  
//                    interpolatedSample_vec[iWin] = interpolateSample(gDelayBuffer, (float) gWritePointer,  currentDelay_vec[iWin], (float) gOffset, (float) gSampleRate);
                    
                    float windowVal = cos(sawtooth(2.0*M_PI*ph + iWin * 2*M_PI/(noWindows),1.0f)*M_PI/2);

                    interpolatedSample_result = interpolatedSample_result + interpolatedSample_val*windowVal;
                }
                
                
                
                for (int iWin = 0; iWin<noWindows; iWin++)
                {
                    float currentDelay = sweepWidth_prev * (0.5 + 0.5 * sawtooth(2.0 * M_PI * ph + iWin * 2*M_PI/(noWindows),1.0f)); // ph is f/fs
                    
//                    float gReadPointer = fmodf((float)gWritePointer - (float)(currentDelay * getSampleRate()) + (float)gDelayBuffer.size() - delayOffset, (float)gDelayBuffer.size());

                    
                    float readPointer = fmodf((float)gWritePointer - (float)(currentDelay * gSampleRate) + (float)gDelayBuffer.size() - gOffset, (float)gDelayBuffer.size());

                    // Cubic interpolation
                    int index = (int)floorf(readPointer);
                    float alpha = readPointer - index;

                    int index_m1 = (index-1) % gDelayBuffer.size();
                    int index_p1 = (index+1) % gDelayBuffer.size();
                    int index_p2 = (index+2) % gDelayBuffer.size();

                    float interpolatedSample_val = ( alpha*(alpha-1)*(alpha-2)*gDelayBuffer[index_m1]/(-6)
                                             + (alpha-1)*(alpha+1)*(alpha-2)*gDelayBuffer[index]/2
                                             + alpha*(alpha+1)*(alpha-2)*gDelayBuffer[index_p1]/(-2)
                                             + alpha*(alpha+1)*(alpha-1)*gDelayBuffer[index_p2]/(6) );
                    
  
//                    interpolatedSample_vec[iWin] = interpolateSample(gDelayBuffer, (float) gWritePointer,  currentDelay_vec[iWin], (float) gOffset, (float) gSampleRate);
                    
                    float windowVal = cos(sawtooth(2.0*M_PI*ph + iWin * 2*M_PI/(noWindows),1.0f)*M_PI/2);

                    interpolatedSample_prev = interpolatedSample_prev + interpolatedSample_val*windowVal;
                }
                
        

                if (flagSwitch==1)
                {

                    Logger::getCurrentLogger()->outputDebugString("flagSwitch is " + String(flagSwitch) + ".");

                    double fac1 = sqrt((1 - countSwitch/countSwitchMax));
                    double fac2 = sqrt((countSwitch/countSwitchMax));

                    interpolatedSample_comb = interpolatedSample_prev * fac1 + interpolatedSample_result*fac2;

                    countSwitch = countSwitch + 1;
                    if (countSwitch == countSwitchMax)
                    {
                        countSwitch = 0;
                        flagSwitch = 0;
                        sweepWidth_prev = sweepWidth; // update sweepWidth_prev

//                        Logger::getCurrentLogger()->outputDebugString("flagSwitch is " + String(flagSwitch) + ".");
                    }
                }
                else if (flagSwitch==0)
                {
                    interpolatedSample_comb = interpolatedSample_result;
                }


                gDelayBuffer[gWritePointer] = in;
                    
                gWritePointer++;
                if(gWritePointer>=gDelayBuffer.size())
                {
                    gWritePointer = 0;
                }
        
                // Update the LFO phase, keeping it in the range 0-1
                ph += frequency*inverseSampleRate;
                if(ph >= 1.0)
                {
                    ph -= 1.0;
                }
                
//                outputL[i] = interpolatedSample_comb * gVolume_slider;
                outputL[i] = interpolatedSample_comb * gVolume_slider;
//                outputL[i] = interpolatedSample;
                outputR[i] = outputL[i];

            }
        }
    }
    
}

//==============================================================================
bool PitchShifterAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* PitchShifterAudioProcessor::createEditor()
{
    return new PitchShifterAudioProcessorEditor (*this);
}

//==============================================================================
void PitchShifterAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void PitchShifterAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PitchShifterAudioProcessor();
}
