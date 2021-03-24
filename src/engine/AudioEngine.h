/*
    This file is part of Element
    Copyright (C) 2019  Kushview, LLC.  All rights reserved.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#pragma once

#include "ElementApp.h"
#include "engine/Engine.h"
#include "engine/GraphProcessor.h"
#include "engine/MidiIOMonitor.h"
#include "engine/Transport.h"
#include "session/DeviceManager.h"
#include "session/Session.h"
#include "RunMode.h"

namespace Element {

class Globals;
class ClipFactory;
class EngineControl;
class Settings;

typedef GraphProcessor::AudioGraphIOProcessor IOProcessor;

class RootGraph : public GraphProcessor
{
public:
    RootGraph();
    ~RootGraph() { }
    
    enum RenderMode
    {
        SingleGraph     = 0,
        Parallel        = (1 << 0)
    };

    inline void setLocked (const var&)
    {
        const bool isNowLocked = false;
        ScopedLock sl (getCallbackLock());
        locked = isNowLocked;
    }

    inline static bool renderModeValid (const int mode) {
        return mode == SingleGraph || mode == Parallel;
    }

    inline static String getSlugForRenderMode (const RenderMode mode)
    {
        switch (mode)
        {
            case SingleGraph: return "single"; break;
            case Parallel:    return "parallel"; break;
        }
        return String();
    }

    void setValueTree (const ValueTree& tree);
    void setPlayConfigFor (AudioIODevice* device);
    void setPlayConfigFor (const DeviceManager::AudioDeviceSetup& setup);
    void setPlayConfigFor (DeviceManager&);
    
    inline RenderMode getRenderMode() const { return renderMode; }
    inline String getRenderModeSlug() const { return getSlugForRenderMode (renderMode); }
    inline bool isSingle() const { return getRenderMode() == SingleGraph; }
    
    inline void setRenderMode (const RenderMode mode)
    {
        // TODO: don't use a lock
        if (! locked && renderMode == static_cast<int> (mode))
            return;
        ScopedLock sl (getCallbackLock());
        renderMode = locked ? SingleGraph : mode;
    }

    inline void setMidiProgram (const int program)
    {
        if (program == midiProgram)
            return;
        ScopedLock sl (getCallbackLock());
        midiProgram = program;
    }
    
    const String getName() const override;
    const String getInputChannelName (int channelIndex) const override;
    const String getOutputChannelName (int channelIndex) const override;
    const String getAudioInputDeviceName() const        { return audioInName; }
    const String getAudioOutputDeviceName() const       { return audioOutName; }
    
    /** the index in the audio engine.  if less than 0 then the graph
        is not attached */
    int getEngineIndex()    const { return engineIndex; }

private:
    friend class AudioEngine;
    friend struct RootGraphRender;

    GraphNodePtr ioNodes [IOProcessor::numDeviceTypes];
    String graphName = "Device";
    String audioInName, audioOutName;
    StringArray audioInputNames;
    StringArray audioOutputNames;
    int midiChannel = 0;
    int midiProgram = -1;
    int engineIndex = -1;
    RenderMode renderMode = Parallel;
    
    bool locked = true;

    void updateChannelNames (AudioIODevice* device);
};

class AudioEngine : public Engine
{
public:
    Signal<void()> sampleLatencyChanged;
    AudioEngine (Globals&, RunMode mode = RunMode::STANDALONE);
    virtual ~AudioEngine() noexcept;

    //==========================================================================
    RunMode getRunMode() const { return runMode; }

    //==========================================================================
    void activate();
    void deactivate();
    
    /** Adds a message to the MIDI input.  This can be used by Controllers and UI
        components that send MIDI in a non-realtime critical situation. DO NOT call
        this from the audio thread 
     
        @param msg                      The MidiMessage to send
        @param handleOnDeviceQueue      When true will treat it as if received by a
                                        MidiInputDevice callback (don't use except for debugging)
     */
    void addMidiMessage (const MidiMessage msg, bool handleOnDeviceQueue = false);
    
    void applySettings (Settings&);
    
    bool isUsingExternalClock() const;
    
    void setSession (SessionPtr);
    void refreshSession();
    
    bool addGraph (RootGraph* graph);
    bool removeGraph (RootGraph* graph);
    
    void setCurrentGraph (const int index) { setActiveGraph (index); }
    void setActiveGraph (const int index);
    int getActiveGraph() const;
    
    RootGraph* getGraph (const int index);
    
    void setPlaying (const bool shouldBePlaying);
    void setRecording (const bool shouldBeRecording);
    void seekToAudioFrame (const int64 frame);
    void setMeter (int beatsPerBar, int beatDivisor);
    
    void togglePlayPause();
    
    MidiKeyboardState& getKeyboardState();
    Transport::MonitorPtr getTransportMonitor() const;
    AudioIODeviceCallback& getAudioIODeviceCallback() override;
    MidiInputCallback& getMidiInputCallback() override;
    
    /** For use by external systems only! e.g. the AU/VST version of Element and
        possibly things like rendering in the future
     */
    void prepareExternalPlayback (const double sampleRate, const int blockSize,
                                  const int numIns, const int numOuts);
    void processExternalBuffers (AudioBuffer<float>& buffer, MidiBuffer& midi);
    void processExternalPlayhead (AudioPlayHead* playhead, const int nframes);
    void releaseExternalResources();
    void updateExternalLatencySamples();
    int getExternalLatencySamples() const;

    Globals& getWorld() const;
    MidiIOMonitorPtr getMidiIOMonitor() const;

private:
    class Private;
    ScopedPointer<Private> priv;
    Globals& world;
    RunMode runMode;
};

typedef ReferenceCountedObjectPtr<AudioEngine> AudioEnginePtr;

}
