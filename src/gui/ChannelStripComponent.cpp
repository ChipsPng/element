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

#include "gui/ChannelStripComponent.h"

namespace element {

ChannelStripComponent::VolumeLabel::VolumeLabel() {}
ChannelStripComponent::VolumeLabel::~VolumeLabel() {}

void ChannelStripComponent::VolumeLabel::settingLabelDoubleClicked()
{
    if (auto* const strip = findParentComponentOfClass<ChannelStripComponent>())
        strip->volumeLabelDoubleClicked();
}

ChannelStripComponent::ChannelStripComponent()
    : meter (2, false)
{
    addAndMakeVisible (fader);
    fader.setSliderStyle (Slider::LinearVertical);
    fader.setTextBoxStyle (Slider::NoTextBox, true, 1, 1);
    fader.setRange (-60.0, 6.0, 0.001);
    fader.setValue (0.f, dontSendNotification);
    fader.setSkewFactor (2);
    fader.addListener (this);

    addAndMakeVisible (meter, 100);
    addAndMakeVisible (scale, 101);

    addAndMakeVisible (name);
    name.setFont (name.getFont().withHeight (14));
    name.setJustificationType (Justification::centred);
    name.setText ("Name", dontSendNotification);

    addAndMakeVisible (mute);
    mute.setColour (SettingButton::backgroundOnColourId, Colors::toggleBlue);
    mute.setButtonText ("M");
    mute.addListener (this);

    addAndMakeVisible (mute2);
    mute2.setYesNoText ("M", "M");
    mute2.setButtonText ("M");
    mute2.setColour (SettingButton::backgroundOnColourId, Colors::toggleRed);
    mute2.setColour (SettingButton::textColourId, Colours::black);
    mute2.addListener (this);

    addAndMakeVisible (volume);
    volume.setNumDecimalPlaces (1);
    volume.setMinMax (fader.getMinimum(), fader.getMaximum());
    volume.setValue (fader.getValue());
    volume.setTextWhenMinimum ("-inf");
    volume.getValueObject().addListener (this);

    stabilizeContent();
}

ChannelStripComponent::~ChannelStripComponent() noexcept
{
    fader.removeListener (this);
    volume.getValueObject().removeListener (this);
}

void ChannelStripComponent::setMinMaxDecibels (double minDb, double maxDb)
{
    jassert (maxDb > minDb);
    fader.setRange (minDb, maxDb, 0.001);
    volume.setMinMax (fader.getMinimum(), fader.getMaximum());
    volume.setValue (fader.getValue());
}

void ChannelStripComponent::addButton (Component* btn)
{
    jassert (! extraButtons.contains (btn));
    if (! extraButtons.contains (btn))
        addAndMakeVisible (extraButtons.add (btn));
    resized();
}

void ChannelStripComponent::resized()
{
    auto r1 = getLocalBounds().reduced (2);
    auto r2 = r1.removeFromRight (r1.getWidth() / 2);

    r1.removeFromTop (4);
    volume.setBounds (r1.removeFromTop (18).withSizeKeepingCentre (30, 18));
    r1.removeFromBottom (4);

    for (auto* const button : extraButtons)
    {
        button->setBounds (r1.removeFromBottom (18).withSizeKeepingCentre (26, 18));
        r1.removeFromBottom (1);
    }

    mute.setBounds (r1.removeFromBottom (18).withSizeKeepingCentre (26, 18));

    if (mute2.isVisible())
    {
        r1.removeFromBottom (1);
        mute2.setBounds (r1.removeFromBottom (18).withSizeKeepingCentre (26, 18));
    }

    const int quarter = r2.getWidth() / 2;
    fader.setBounds (r2.removeFromRight (quarter));
    auto r3 = r2.removeFromRight (quarter);
    r3.removeFromTop (4);
    r3.removeFromBottom (4);
    meter.setBounds (r3);
    scale.setBounds (meter.getBoundsInParent());
}

void ChannelStripComponent::buttonClicked (Button* b)
{
    if (b == &mute)
    {
        mute.setToggleState (! mute.getToggleState(), false);
        powerChanged();
    }
    else if (b == &mute2)
    {
        mute2.setToggleState (! mute2.getToggleState(), false);
        muteChanged();
    }
}

void ChannelStripComponent::sliderValueChanged (Slider* slider)
{
    volumeChanged (slider->getValue());
    stabilizeContent();
}

void ChannelStripComponent::valueChanged (Value& value)
{
    fader.setValue ((double) value.getValue(), sendNotificationAsync);
}

void ChannelStripComponent::paint (Graphics&) {}

void ChannelStripComponent::stabilizeContent()
{
    volume.getValueObject().removeListener (this);
    volume.setValue (fader.getValue());
    volume.getValueObject().addListener (this);
}

} // namespace element
