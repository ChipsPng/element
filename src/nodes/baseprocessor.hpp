// Copyright 2023 Kushview, LLC <info@kushview.net>
// SPDX-License-Identifier: GPL3-or-later

#pragma once

#include <element/juce/core.hpp>
#include "nodes/nodetypes.hpp"
#include "ElementApp.h" // FIXME

namespace element {

/** Base class for writing new nodes as a juce::AudioProcessor. */
class BaseProcessor : public juce::AudioPluginInstance
{
public:
    using HostedParameter = juce::HostedAudioProcessorParameter;

    BaseProcessor() : juce::AudioPluginInstance() {}
    BaseProcessor (const BusesProperties& ioLayouts)
        : juce::AudioPluginInstance (ioLayouts) {}
    virtual ~BaseProcessor() {}

protected:
    /** This is for backward compatibility with juce 6. Don't use in new processors */
    inline void addLegacyParameter (HostedParameter* param)
    {
        addHostedParameter (std::unique_ptr<HostedParameter> (param));
    }
#if 0
    // Audio Processor Template
    virtual const String getName() const = 0;
    virtual StringArray getAlternateDisplayNames() const;
    virtual void prepareToPlay (double sampleRate, int maximumExpectedSamplesPerBlock) = 0;
    virtual void releaseResources() = 0;
    
    virtual void processBlock (AudioBuffer<float>& buffer, MidiBuffer& midiMessages) = 0;
    virtual void processBlock (AudioBuffer<double>& buffer, idiBuffer& midiMessages);
    
    virtual void processBlockBypassed (AudioBuffer<float>& buffer, MidiBuffer& midiMessages);
    virtual void processBlockBypassed (AudioBuffer<double>& buffer, MidiBuffer& midiMessages);
    
    virtual bool canAddBus (bool isInput) const                     { ignoreUnused (isInput); return false; }
    virtual bool canRemoveBus (bool isInput) const                  { ignoreUnused (isInput); return false; }
    virtual bool supportsDoublePrecisionProcessing() const;
    
    virtual double getTailLengthSeconds() const = 0;
    virtual bool acceptsMidi() const = 0;
    virtual bool producesMidi() const = 0;
    virtual bool supportsMPE() const                            { return false; }
    virtual bool isMidiEffect() const                           { return false; }
    virtual void reset();
    virtual void setNonRealtime (bool isNonRealtime) noexcept;
    
    virtual AudioProcessorEditor* createEditor() = 0;
    virtual bool hasEditor() const = 0;
    
    virtual int getNumPrograms()        { return 1; };
    virtual int getCurrentProgram()     { return 1; };
    virtual void setCurrentProgram (int index) { ignoreUnused (index); };
    virtual const String getProgramName (int index) { ignoreUnused (index); }
    virtual void changeProgramName (int index, const String& newName) { ignoreUnused (index, newName); }
    virtual void getStateInformation (juce::MemoryBlock& destData) = 0;
    virtual void getCurrentProgramStateInformation (juce::MemoryBlock& destData);
    
    virtual void setStateInformation (const void* data, int sizeInBytes) = 0;
    virtual void setCurrentProgramStateInformation (const void* data, int sizeInBytes);
    
    virtual void numChannelsChanged();
    virtual void numBusesChanged();
    virtual void processorLayoutsChanged();
    
    virtual void addListener (AudioProcessorListener* newListener);
    virtual void removeListener (AudioProcessorListener* listenerToRemove);
    virtual void setPlayHead (AudioPlayHead* newPlayHead);
    
    virtual void updateTrackProperties (const TrackProperties& properties);
    
    virtual void fillInPluginDescription (PluginDescription& description);
    
protected:
    virtual bool isBusesLayoutSupported (const BusesLayout&) const          { return true; }
    virtual bool canApplyBusesLayout (const BusesLayout& layouts) const     { return isBusesLayoutSupported (layouts); }
    virtual bool canApplyBusCountChange (bool isInput, bool isAddingBuses, BusProperties& outNewBusProperties);
#endif

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BaseProcessor)
};

} // namespace element
