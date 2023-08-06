// Copyright 2019-2023 Kushview, LLC <info@kushview.net>
// SPDX-License-Identifier: GPL3-or-later

#pragma once

#include "gui/Buttons.h"
#include <element/ui/content.hpp>

namespace element {

class GraphPropertyPanel;
class GraphSettingsView : public ContentView,
                          public Button::Listener,
                          private Value::Listener
{
public:
    GraphSettingsView();
    ~GraphSettingsView();

    void setPropertyPanelHeaderVisible (bool);
    void setGraphButtonVisible (bool isVisible);
    void setUpdateOnActiveGraphChange (bool shouldUpdate);

    void resized() override;
    void didBecomeActive() override;
    void stabilizeContent() override;
    void buttonClicked (Button*) override;

private:
    std::unique_ptr<GraphPropertyPanel> props;
    GraphButton graphButton;
    Value activeGraphIndex;
    bool updateWhenActiveGraphChanges = false;

    void valueChanged (Value& value) override;
};

} // namespace element
