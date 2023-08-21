// Copyright 2023 Kushview, LLC <info@kushview.net>
// SPDX-License-Identifier: GPL3-or-later

#include "ui/nodechannelstrip.hpp"
#include "ui/guicommon.hpp"
#include "ui/nodeioconfiguration.hpp"
#include "nodes/volume.hpp"
#include "nodes/volumeeditor.hpp"

namespace element {

class VolumeNodeEditor::ChannelStrip : public NodeChannelStripComponent,
                                       public AudioProcessorParameter::Listener
{
public:
    ChannelStrip (GuiService& g)
        : NodeChannelStripComponent (g, false)
    {
        setVolumeMinMax (-30, 12, 0.5);

        ioButton = new SettingButton();
        ioButton->setPath (getIcons().fasCog);
        ioButton->onClick = [this]() {
            auto node = getNode();
            ProcessorPtr obj = node.getObject();
            auto* proc = (obj) ? obj->getAudioProcessor() : 0;
            if (! proc)
                return;

            if (ioButton->getToggleState())
            {
                ioButton->setToggleState (false, dontSendNotification);
                ioBox.clear();
            }
            else
            {
                auto* component = new NodeAudioBusesComponent (node, proc, ViewHelpers::findContentComponent (this));
                auto& box = CallOutBox::launchAsynchronously (
                    std::unique_ptr<Component> (component),
                    ioButton->getScreenBounds(),
                    0);
                ioBox.setNonOwned (&box);
            }
        };

        getChannelStrip().addButton (ioButton);

        onVolumeChanged = [this] (double value) {
            float fvalue = static_cast<float> (value);
            if (param != nullptr)
            {
                param->beginChangeGesture();
                *param = fvalue;
                param->endChangeGesture();
            }
        };
    }

    ~ChannelStrip()
    {
        if (ioButton)
        {
            ioButton->onClick = nullptr;
            ioButton = nullptr;
        }

        if (param)
            param->removeListener (this);
        param = nullptr;
        onVolumeChanged = nullptr;
    }

    void parameterValueChanged (int parameterIndex, float newValue) override
    {
        ignoreUnused (parameterIndex, newValue);
        // updateChannelStrip();
    }

    void parameterGestureChanged (int parameterIndex, bool gestureIsStarting) override
    {
        ignoreUnused (parameterIndex, gestureIsStarting);
    }

    void updateParameter()
    {
        if (param)
        {
            param->removeListener (this);
            param = nullptr;
        }

        if (auto object = getNode().getObject())
            if (auto* proc = dynamic_cast<VolumeProcessor*> (object->getAudioProcessor()))
                param = dynamic_cast<AudioParameterFloat*> (proc->getParameters()[0]);

        stabilizeContent();

        if (param)
            param->addListener (this);
    }

protected:
    float getCurrentVolume() override
    {
        return param != nullptr ? *param : 0.f;
    }

private:
    AudioParameterFloat* param = nullptr;
    SettingButton* ioButton = nullptr;
    OptionalScopedPointer<CallOutBox> ioBox;
};

VolumeNodeEditor::VolumeNodeEditor (const Node& node, GuiService& gui)
    : NodeEditor (node)
{
    setOpaque (true);
    strip.reset (new ChannelStrip (gui));
    addAndMakeVisible (strip.get());
    strip->setComboBoxesVisible (false, false);

    setSize (128, 262);

    strip->setNode (node);
    strip->updateParameter();
}

VolumeNodeEditor::~VolumeNodeEditor()
{
    strip.reset();
}

void VolumeNodeEditor::paint (Graphics& g)
{
    g.fillAll (Colours::black);
}

void VolumeNodeEditor::resized()
{
    if (strip)
        strip->setBounds (getLocalBounds());
}

} // namespace element
