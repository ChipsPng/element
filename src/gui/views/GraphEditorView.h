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

#include "gui/properties/NodePropertyPanel.h"
#include "gui/views/GraphDisplayView.h"
#include "gui/GraphEditorComponent.h"
#include <element/signals.hpp>

namespace element {

class GraphEditorView : public GraphDisplayView,
                        public ChangeListener
{
public:
    GraphEditorView();
    ~GraphEditorView();

    void didBecomeActive() override;
    void stabilizeContent() override;
    void willBeRemoved() override;

    ValueTree settings() const;
    void saveSettings();
    void restoreSettings();

    void paint (Graphics& g) override;
    bool keyPressed (const KeyPress& key) override;
    void changeListenerCallback (ChangeBroadcaster*) override;

protected:
    void graphDisplayResized (const Rectangle<int>& area) override;
    void graphNodeWillChange() override;
    void graphNodeChanged (const Node& g, const Node&) override;

private:
    Node node;
    GraphEditorComponent graph;
    Viewport view;
    NodePropertyPanel nodeProps;
    int nodePropsWidth = 220;

    class NodePropsToggle : public Label
    {
    public:
        NodePropsToggle() = default;
        ~NodePropsToggle() override = default;
        std::function<void()> onClick;

    protected:
        void mouseUp (const MouseEvent& ev) override
        {
            if (onClick)
                onClick();
        }
    };

    NodePropsToggle nodePropsToggle;

    SignalConnection nodeSelectedConnection,
        nodeRemovedConnection;
    void onNodeSelected();
    void onNodeRemoved (const Node&);
    void updateSizeInternal (const bool force = true);
};

} // namespace element
