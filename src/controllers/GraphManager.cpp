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

#include "controllers/GraphManager.h"

#include "engine/nodes/AudioRouterNode.h"
#include "engine/nodes/MidiChannelSplitterNode.h"
#include "engine/nodes/MidiProgramMapNode.h"
#include "engine/nodes/PlaceholderProcessor.h"
#include "engine/nodes/SubGraphProcessor.h"

#include "session/PluginManager.h"
#include "Globals.h"
#include "Utils.h"

namespace Element {

static void showFailedInstantiationAlert (const PluginDescription& desc, const bool async = false)
{
    String header = "Plugin Instantiation Failed";
    String message = desc.name;
    message << " could not be instantiated";

    if (async)
        AlertWindow::showMessageBoxAsync (AlertWindow::WarningIcon, header, message);
    else
        AlertWindow::showMessageBox (AlertWindow::WarningIcon, header, message);
}

class NodeModelUpdater : public ReferenceCountedObject
{
public:
    NodeModelUpdater (GraphManager& m, const ValueTree& d, NodeObject* o)
        : manager (m), data (d), object (o)
    {
        portsChangedConnection = object->portsChanged.connect ( 
            std::bind (&NodeModelUpdater::onPortsChanged, this));
    }

    ~NodeModelUpdater()
    {
        portsChangedConnection.disconnect();
    }

private:
    GraphManager& manager;
    ValueTree data;
    NodeObjectPtr object;
    SignalConnection portsChangedConnection;

    void onPortsChanged()
    {
        const auto newPorts = object->getMetadata().getChildWithName (Tags::ports);
        int index = data.indexOf (data.getChildWithName (Tags::ports));
        if (index > 0 && newPorts.isValid())
        {
            data.removeChild (index, nullptr);
            data.addChild (newPorts.createCopy(), index, nullptr);
            manager.removeIllegalConnections();
        }

        manager.syncArcsModel();
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NodeModelUpdater);
};

/** This enforces correct IO nodes based on the graph processor's settings
    in virtual methods like 'acceptsMidi' and 'getTotalNumInputChannels'
    It uses the controller for all node operations so the model will
    stay in sync.
  */
struct IONodeEnforcer
{
    IONodeEnforcer (GraphManager& c)
        : controller (c)
    {
        addMissingIONodes(); 
        controller.syncArcsModel();
    }
    
    GraphManager* getController() const { return &controller; }

private:
    GraphManager& controller;
    void addMissingIONodes()
    {
        auto* root = getController();
        if (! root) return;
        auto& proc (root->getGraph());
        const Node graph (root->getGraphModel());

        const bool wantsAudioIn   = graph.hasAudioInputNode()  && proc.getTotalNumInputChannels() > 0;
        const bool wantsAudioOut  = graph.hasAudioOutputNode() && proc.getTotalNumOutputChannels() > 0;
        const bool wantsMidiIn    = graph.hasMidiInputNode()   && proc.acceptsMidi();
        const bool wantsMidiOut   = graph.hasMidiOutputNode()  && proc.producesMidi();
        
        NodeObjectPtr ioNodes [IOProcessor::numDeviceTypes];
        for (int i = 0; i < root->getNumNodes(); ++i)
        {
            NodeObjectPtr node = root->getNode (i);
            if (node->isMidiIONode() || node->isAudioIONode())
            {
                auto* ioProc = dynamic_cast<IOProcessor*> (node->getAudioProcessor());
                ioNodes [ioProc->getType()] = node;
            }
        }
        
        Array<uint32> nodesToRemove;

        for (int t = 0; t < IOProcessor::numDeviceTypes; ++t)
        {
            if (nullptr != ioNodes [t])
            {
                if (t == IOProcessor::audioInputNode    && !wantsAudioIn)   nodesToRemove.add (ioNodes[t]->nodeId);
                if (t == IOProcessor::audioOutputNode   && !wantsAudioOut)  nodesToRemove.add (ioNodes[t]->nodeId);;
                if (t == IOProcessor::midiInputNode     && !wantsMidiIn)    nodesToRemove.add (ioNodes[t]->nodeId);;
                if (t == IOProcessor::midiOutputNode    && !wantsMidiOut)   nodesToRemove.add (ioNodes[t]->nodeId);;
                continue;
            }

            if (t == IOProcessor::audioInputNode    && !wantsAudioIn)   continue;
            if (t == IOProcessor::audioOutputNode   && !wantsAudioOut)  continue;
            if (t == IOProcessor::midiInputNode     && !wantsMidiIn)    continue;
            if (t == IOProcessor::midiOutputNode    && !wantsMidiOut)   continue;

            PluginDescription desc;
            desc.pluginFormatName = "Internal";
            double rx = 0.5f, ry = 0.5f;
            switch (t)
            {
                case IOProcessor::audioInputNode:
                    desc.fileOrIdentifier = "audio.input";
                    rx = .25;
                    ry = .25;
                    break;
                case IOProcessor::audioOutputNode:
                    desc.fileOrIdentifier = "audio.output";
                    rx = .25;
                    ry = .75;
                    break;
                case IOProcessor::midiInputNode:
                    desc.fileOrIdentifier = "midi.input";
                    rx = .75;
                    ry = .25;
                    break;
                case IOProcessor::midiOutputNode:
                    desc.fileOrIdentifier = "midi.output";
                    rx = .75;
                    ry = .75;
                    break;
            }
            
            auto nodeId = root->addNode (&desc, rx, ry);
            ioNodes[t] = root->getNodeForId (nodeId);
            jassert(ioNodes[t] != nullptr);
        }

        for (const auto& nodeId : nodesToRemove)
            root->removeNode (nodeId);
    }
};

GraphManager::GraphManager (GraphProcessor& pg, PluginManager& pm)
    : pluginManager (pm), processor (pg), lastUID (0)
{ }

GraphManager::~GraphManager()
{
    // Make sure to dereference NodeObject's so we don't leak memory
    // If you get warnings by juce's leak detector about graph related
    // objects, then there's probably "object" properties lingering that
    // are referenced in the model;
    Node::sanitizeRuntimeProperties (graph, true);
    graph = arcs = nodes = ValueTree();
}

uint32 GraphManager::getNextUID() noexcept
{
    return ++lastUID;
}

int GraphManager::getNumNodes() const noexcept { return processor.getNumNodes(); }
const NodeObjectPtr GraphManager::getNode (const int index) const noexcept { return processor.getNode (index); }

const NodeObjectPtr GraphManager::getNodeForId (const uint32 uid) const noexcept
{
    return processor.getNodeForId (uid);
}

const Node GraphManager::getNodeModelForId (const uint32 nodeId) const noexcept {
    return Node (nodes.getChildWithProperty (Tags::id, static_cast<int64> (nodeId)), false);
}
    
bool GraphManager::contains (const uint32 nodeId) const {
    return processor.getNodeForId (nodeId) != nullptr;
}

NodeObject* GraphManager::createFilter (const PluginDescription* desc, double x, double y, uint32 nodeId)
{
    String errorMessage;
    auto node = std::unique_ptr<NodeObject> (
        pluginManager.createGraphNode (*desc, errorMessage));

    if (errorMessage.isNotEmpty())
    {
        DBG("[EL] error creating audio plugin: " << errorMessage);
        jassert (node == nullptr);
    }
    
    if (node == nullptr && errorMessage.isEmpty())
    {
        jassertfalse;
        errorMessage = "Could not find node";
    }
 
    return node != nullptr ? processor.addNode (node.release(), nodeId) : nullptr;
}

NodeObject* GraphManager::createPlaceholder (const Node& node)
{
    PluginDescription desc; node.getPluginDescription (desc);
    auto* ph = new PlaceholderProcessor ();
    ph->setupFor (node, processor.getSampleRate(), processor.getBlockSize());
    return processor.addNode (ph, node.getNodeId());
}

uint32 GraphManager::addNode (const Node& newNode)
{
    if (! newNode.isValid())
    {
        AlertWindow::showMessageBox (AlertWindow::WarningIcon, TRANS ("Couldn't create Node"),
                                     "Cannot instantiate node without a description");
        return KV_INVALID_NODE;
    }
    
    uint32 nodeId = KV_INVALID_NODE;
    const PluginDescription desc (pluginManager.findDescriptionFor (newNode));
    if (auto* node = createFilter (&desc, 0, 0,
        newNode.hasProperty(Tags::id) ? newNode.getNodeId() : 0))
    {
        nodeId = node->nodeId;
        ValueTree data = newNode.getValueTree().createCopy();
        data.setProperty (Tags::id, static_cast<int64> (nodeId), nullptr)
            .setProperty (Tags::object, node, nullptr)
            .setProperty (Tags::type, node->getTypeString(), nullptr)
            .setProperty (Tags::pluginIdentifierString, desc.createIdentifierString(), nullptr);
        
        data.removeProperty (Tags::relativeX, nullptr);
        data.removeProperty (Tags::relativeY, nullptr);
        data.removeProperty (Tags::windowX, nullptr);
        data.removeProperty (Tags::windowY, nullptr);
        data.removeProperty (Tags::windowVisible, nullptr);

        setupNode (data, node);

        nodes.addChild (data, -1, nullptr);
        changed();
    }
    else
    {
        nodeId = KV_INVALID_NODE;
        AlertWindow::showMessageBox (AlertWindow::WarningIcon, "Couldn't create filter",
                                     "The plugin could not be instantiated");
    }
    
    return nodeId;
}
    
uint32 GraphManager::addNode (const PluginDescription* desc, double rx, double ry, uint32 nodeId)
{
    if (! desc)
    {
        AlertWindow::showMessageBox (AlertWindow::WarningIcon,
                                     TRANS ("Couldn't create filter"),
                                     TRANS ("Cannot instantiate plugin without a description"));
        return KV_INVALID_NODE;
    }

    if (auto* node = createFilter (desc, rx, ry, nodeId))
    {
        nodeId = node->nodeId;
        ValueTree model = node->getMetadata().createCopy();
        model.setProperty (Tags::id, static_cast<int64> (nodeId), nullptr)
             .setProperty (Tags::name,   desc->name, nullptr)
             .setProperty (Tags::object, node, nullptr)
             .setProperty (Tags::updater,    new NodeModelUpdater (*this, model, node), nullptr)
             .setProperty (Tags::relativeX, rx, nullptr)
             .setProperty (Tags::relativeY, ry, nullptr)
             .setProperty (Tags::pluginIdentifierString, 
                           desc->createIdentifierString(), nullptr);
        
        Node n (model, true);

        if (auto* sub = node->processor<SubGraphProcessor>())
        {
            sub->getController().setNodeModel (n);
            IONodeEnforcer enforceIONodes (sub->getController());
        }
        
        if (auto* const proc = node->getAudioProcessor())
        {
            // try to use stereo by default on newly added plugins
            AudioProcessor::BusesLayout stereoInOut;
            stereoInOut.inputBuses.add (AudioChannelSet::stereo());
            stereoInOut.outputBuses.add (AudioChannelSet::stereo());
            AudioProcessor::BusesLayout stereoOut;
            stereoOut.outputBuses.add (AudioChannelSet::stereo());
            AudioProcessor::BusesLayout* tryStereo = nullptr;
            const auto oldLayout = proc->getBusesLayout();

            if (proc->getTotalNumInputChannels() == 1 &&
                proc->getTotalNumOutputChannels() == 1 &&
                proc->checkBusesLayoutSupported (stereoInOut))
            {
                tryStereo = &stereoInOut;
            }
            else if (proc->getTotalNumInputChannels() == 0 &&
                proc->getTotalNumOutputChannels() == 1 &&
                proc->checkBusesLayoutSupported (stereoOut))
            {
                tryStereo = &stereoOut;
            }

            if (tryStereo != nullptr && proc->checkBusesLayoutSupported (*tryStereo))
            {
                proc->suspendProcessing (true);
                proc->releaseResources();

                if (! proc->setBusesLayout (*tryStereo))
                    proc->setBusesLayout (oldLayout);

                proc->prepareToPlay (processor.getSampleRate(), processor.getBlockSize());
                proc->suspendProcessing (false);
            }
        }

        // make sure the model ports are correct with the actual processor
        n.resetPorts();

        nodes.addChild (model, -1, nullptr);
        changed();
    }
    else
    {
        nodeId = KV_INVALID_NODE;
        showFailedInstantiationAlert (*desc, true);
    }

    return nodeId;
}

void GraphManager::removeNode (const uint32 uid)
{
    if (! processor.removeNode (uid))
        return;
    for (int i = 0; i < nodes.getNumChildren(); ++i)
    {
        const Node node (nodes.getChild (i), false);
        if (node.getNodeId() == uid)
        {
            // the model was probably referencing the node ptr
            NodeObjectPtr obj = node.getGraphNode();
            if (obj)
            {
                obj->willBeRemoved();
                obj->releaseResources();
            }

            auto data = node.getValueTree();
            nodes.removeChild (data, nullptr);
            // clear all referecnce counted objects
            Node::sanitizeProperties (data, true);
            // finally delete the node + plugin instance.
            obj = nullptr;
        }
    }
    
    jassert(nodes.getNumChildren() == getNumNodes());
    processorArcsChanged();
}

void GraphManager::disconnectNode (const uint32 nodeId, const bool inputs, const bool outputs,
                                                             const bool audio, const bool midi)
{
    jassert (inputs || outputs);
    bool doneAnything = false;

    for (int i = getNumConnections(); --i >= 0;)
    {
        const auto* const c = processor.getConnection (i);
        if ((outputs && c->sourceNode == nodeId) || 
            (inputs && c->destNode == nodeId))
        {
            NodeObjectPtr src = processor.getNodeForId (c->sourceNode);
            NodeObjectPtr dst = processor.getNodeForId (c->destNode);

            if ((audio && src->getPortType(c->sourcePort) == PortType::Audio && dst->getPortType(c->destPort) == PortType::Audio) ||
                (midi && src->getPortType(c->sourcePort) == PortType::Midi && dst->getPortType(c->destPort) == PortType::Midi))
            {
                removeConnection (i);
                doneAnything = true;
            }
        }
    }

    if (doneAnything)
        processorArcsChanged();
}

void GraphManager::removeIllegalConnections()
{
    if (processor.removeIllegalConnections())
        processorArcsChanged();
}

int GraphManager::getNumConnections() const noexcept
{
    jassert(arcs.getNumChildren() == processor.getNumConnections());
    return processor.getNumConnections();
}

const GraphProcessor::Connection* GraphManager::getConnection (const int index) const noexcept
{
    return processor.getConnection (index);
}

const GraphProcessor::Connection* GraphManager::getConnectionBetween (uint32 sourceFilterUID, int sourceFilterChannel,
                                                                          uint32 destFilterUID, int destFilterChannel) const noexcept
{
    return processor.getConnectionBetween (sourceFilterUID, sourceFilterChannel,
                                           destFilterUID, destFilterChannel);
}

bool GraphManager::canConnect (uint32 sourceFilterUID, int sourceFilterChannel,
                                  uint32 destFilterUID, int destFilterChannel) const noexcept
{
    return processor.canConnect (sourceFilterUID, sourceFilterChannel,
                                 destFilterUID, destFilterChannel);
}

bool GraphManager::addConnection (uint32 sourceFilterUID, int sourceFilterChannel,
                                     uint32 destFilterUID, int destFilterChannel)
{
    const bool result = processor.addConnection (sourceFilterUID, (uint32)sourceFilterChannel,
                                                 destFilterUID, (uint32)destFilterChannel);
    if (result)
        processorArcsChanged();

    return result;
}

void GraphManager::removeConnection (const int index)
{
    processor.removeConnection (index);
    processorArcsChanged();
}

void GraphManager::removeConnection (uint32 sourceNode, uint32 sourcePort,
                                     uint32 destNode, uint32 destPort)
{
    if (processor.removeConnection (sourceNode, sourcePort, destNode, destPort))
        processorArcsChanged();
}

void GraphManager::setNodeModel (const Node& node)
{
    loaded = false;

    processor.clear();
    graph   = node.getValueTree();
    arcs    = node.getArcsValueTree();
    nodes   = node.getNodesValueTree();
    
    Array<ValueTree> failed;
    for (int i = 0; i < nodes.getNumChildren(); ++i)
    {
        Node node (nodes.getChild (i), false);
        const PluginDescription desc (pluginManager.findDescriptionFor (node));
        if (NodeObjectPtr obj = createFilter (&desc, 0.0, 0.0, node.getNodeId()))
        {
            setupNode (node.getValueTree(), obj);
            obj->setEnabled (node.isEnabled());
            node.setProperty (Tags::enabled, obj->isEnabled());
        }
        else if (NodeObjectPtr ph = createPlaceholder (node))
        {
            DBG("[EL] couldn't create node: " << node.getName() << ". Creating offline placeholder");
            node.getValueTree().setProperty (Tags::object, ph.get(), nullptr);
            node.getValueTree().setProperty (Tags::missing, true, nullptr);
        }
        else
        {
            DBG("[EL] couldn't create node: " << node.getName());
            failed.add (node.getValueTree());
        }
    }

    for (const auto& n : failed)
    {
        nodes.removeChild (n, nullptr);
        Node::sanitizeRuntimeProperties (n);
    }
    failed.clearQuick();

    // If you hit this, then failed nodes didn't get handled properly
    jassert (nodes.getNumChildren() == processor.getNumNodes());
    
    // Cheap way to refresh engine-side nodes
    processor.triggerAsyncUpdate();
    processor.handleUpdateNowIfNeeded();

    for (int i = 0; i < arcs.getNumChildren(); ++i)
    {
        ValueTree arc (arcs.getChild (i));
        const auto sourceNode = (uint32)(int) arc.getProperty (Tags::sourceNode);
        const auto destNode = (uint32)(int) arc.getProperty (Tags::destNode);
        bool worked = processor.addConnection (sourceNode, (uint32)(int) arc.getProperty (Tags::sourcePort),
                                               destNode, (uint32)(int) arc.getProperty (Tags::destPort));
        if (worked)
        {
            arc.removeProperty (Tags::missing, 0);
        }
        else
        {
            DBG("[EL] failed creating connection: ");
            const Node graphObject (graph, false);

            if (graphObject.getNodeById(sourceNode).isValid() &&
                graphObject.getNodeById(destNode).isValid())
            {
                DBG("[EL] set missing connection");
                // if the nodes are valid then preserve it
                arc.setProperty (Tags::missing, true, 0);
            }
            else
            {
                DBG("[EL] purge failed arc");
                failed.add (arc);
            }
        }
    }

    const bool discardFailedConnections = true;
    if (discardFailedConnections)
        for (const auto& n : failed)
            arcs.removeChild (n, nullptr);

    
    loaded = true;
    jassert (arcs.getNumChildren() == processor.getNumConnections());
    failed.clearQuick();

    IONodeEnforcer enforceIONodes (*this);
    processorArcsChanged();
}

void GraphManager::savePluginStates()
{
    for (int i = 0; i < nodes.getNumChildren(); ++i)
    {
        Node node (nodes.getChild (i), false);
        node.savePluginState();
    }
}

void GraphManager::clear()
{
    loaded = false;

    if (graph.isValid())
    {
        Node::sanitizeRuntimeProperties (graph);
        graph.removeChild (arcs, nullptr);
        graph.removeChild (nodes, nullptr);
        nodes.removeAllChildren (nullptr);
        arcs.removeAllChildren (nullptr);
        graph.addChild (nodes, -1, nullptr);
        graph.addChild (arcs, -1, nullptr);
    }
    
    processor.clear();
    changed();
}

void GraphManager::processorArcsChanged()
{
    ValueTree newArcs = ValueTree (Tags::arcs);
    for (int i = 0; i < processor.getNumConnections(); ++i)
        newArcs.addChild (Node::makeArc (*processor.getConnection (i)), -1, nullptr);

    for (int i = 0; i < arcs.getNumChildren(); ++i)
    {
        const ValueTree arc (arcs.getChild (i));
        if (true == (bool) arc [Tags::missing])
        {   
            ValueTree missingArc = arc.createCopy();
            if (processor.addConnection (
                (uint32)(int) missingArc[Tags::sourceNode],
                (uint32)(int) missingArc[Tags::sourcePort],
                (uint32)(int) missingArc[Tags::destNode],
                (uint32)(int) missingArc[Tags::destPort]))
            {
                missingArc.removeProperty (Tags::missing, 0);
            }

            newArcs.addChild (missingArc, -1, 0);
        }
    }

    const auto index = graph.indexOf (arcs);
    graph.removeChild (arcs, nullptr);
    graph.addChild (newArcs, index, nullptr);
    arcs = graph.getChildWithName (Tags::arcs);
    changed();
}

void GraphManager::setupNode (const ValueTree& data, NodeObjectPtr obj)
{
    jassert (obj && data.hasType (Tags::node));
    Node node (data, false);
    node.setProperty (Tags::type, obj->getTypeString())
        .setProperty (Tags::object, obj.get())
        .setProperty (Tags::updater, new NodeModelUpdater (*this, data, obj.get()));

    PortArray ins, outs;
    node.getPorts (ins, outs, PortType::Audio);
    bool resetPorts = false;
    if (auto* const proc = obj->getAudioProcessor())
    {
        // try to match ports
        if (proc->getTotalNumInputChannels() != ins.size() ||
            proc->getTotalNumOutputChannels() != outs.size())
        {
            AudioProcessor::BusesLayout layout;
            layout.inputBuses.add (AudioChannelSet::namedChannelSet (ins.size()));
            layout.outputBuses.add (AudioChannelSet::namedChannelSet (outs.size()));
            
            if (proc->checkBusesLayoutSupported (layout))
            {
                proc->suspendProcessing (true);
                proc->releaseResources();
                proc->setBusesLayoutWithoutEnabling (layout);
                proc->prepareToPlay (processor.getSampleRate(), processor.getBlockSize());
                proc->suspendProcessing (false);
            }
            
            resetPorts = true;
        }
    }
    
    if (auto* sub = obj->processor<SubGraphProcessor>())
    {
        sub->getController().setNodeModel (node);
        resetPorts = true;
    }

    node.restorePluginState();

    if (resetPorts || node.getNumPorts() != static_cast<int> (obj->getNumPorts()))
        node.resetPorts();
    
    jassert (node.getNumPorts() == static_cast<int> (obj->getNumPorts()));
}

// MARK: Root Graph Controller
void RootGraphManager::unloadGraph()
{
    getRootGraph().clear();
}

}
