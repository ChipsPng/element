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

#include "gui/nodes/NodeEditorComponent.h"
#include "engine/nodes/MidiProgramMapNode.h"

namespace element {

class AudioRouterEditor : public NodeEditorComponent,
                          public ChangeListener
{
public:
    AudioRouterEditor (const Node& node);
    ~AudioRouterEditor();

    void resized() override;
    void paint (Graphics& g) override;

    void adjustBoundsToMatrixSize (int cellSize = 0);
    String getSizeString() const;
    MatrixState& getMatrixState() { return matrix; }
    void applyMatrix();
    void setFadeLength (double length);
    void changeListenerCallback (ChangeBroadcaster*) override;

    void setAutoResize (bool yn) { autoResize = yn; }

private:
    MatrixState matrix;
    bool autoResize = false;
    class Content;
    std::unique_ptr<Content> content;
};

} // namespace element
