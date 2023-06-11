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

#include "LookAndFeel.h"
#include "gui/Buttons.h"
#include "gui/widgets/MidiBlinker.h"
#include "BinaryData.h"

namespace element {

LookAndFeel::LookAndFeel()
{
    // Element Colors
    setColour (Style::backgroundColorId, Colour (0xff16191a));
    setColour (Style::backgroundHighlightColorId, Colour (0xffcccccc).darker (0.6000006).withAlpha (0.6f));

    setColour (Style::widgetBackgroundColorId, Colour (0xff3b3b3b));
    setColour (Style::contentBackgroundColorId, Colour (0xff3b3b3b).darker (0.6));

    setColour (Style::textColorId, Colour (0xffcccccc));
    setColour (Style::textActiveColorId, Colour (0xffe5e5e5));
    setColour (Style::textBoldColorId, Colour (0xffe4e4e4));

    // setColour (ResizableWindow::backgroundColourId, widgetBackgroundColor.darker(.3));
    // setColour (CaretComponent::caretColourId, Colors::toggleOrange.brighter (0.20f));

    // Property Component
    setColour (PropertyComponent::labelTextColourId, LookAndFeel::textColor);
    setColour (PropertyComponent::backgroundColourId, LookAndFeel::backgroundColor.brighter (0.02));

    // // Text Editor
    // setColour (TextEditor::textColourId,            textColor);
    // setColour (TextEditor::highlightColourId,       Colors::elemental.brighter (0.31f));
    // setColour (TextEditor::highlightedTextColourId, Colours::black.brighter(0.22f));
    // setColour (TextEditor::outlineColourId,         Colours::black);
    // setColour (TextEditor::focusedOutlineColourId,  Colors::toggleBlue.darker (0.002).withAlpha (0.6f));

    setColour (Label::textWhenEditingColourId, findColour (TextEditor::textColourId).darker (0.003));

    setColour (TextPropertyComponent::outlineColourId, findColour (TextEditor::outlineColourId));
    setColour (TextPropertyComponent::backgroundColourId, findColour (TextEditor::backgroundColourId));
    setColour (TextPropertyComponent::textColourId, findColour (TextEditor::textColourId));

    setColour (ToggleButton::textColourId, textColor);

    // Boolean property comp
    setColour (BooleanPropertyComponent::backgroundColourId, findColour (TextEditor::backgroundColourId));
    setColour (BooleanPropertyComponent::outlineColourId, Colours::black);

    // Setting Button
    setColour (SettingButton::backgroundColourId, widgetBackgroundColor.brighter());
    setColour (SettingButton::backgroundOnColourId, Colors::toggleOrange);
    setColour (SettingButton::textColourId, Colours::black);
    setColour (SettingButton::textDisabledColourId, Colours::darkgrey);

    // MIDI Blinkers
    setColour (MidiBlinker::backgroundColourId, findColour (SettingButton::backgroundColourId));
    setColour (MidiBlinker::outlineColourId, LookAndFeel::widgetBackgroundColor.brighter().brighter());

    // Tree View
    setColour (TreeView::selectedItemBackgroundColourId, Colors::elemental.darker (0.6000006f));
    setColour (TreeView::backgroundColourId, LookAndFeel_KV1::backgroundColor);

    // Keymap Editor
    setColour (KeyMappingEditorComponent::textColourId, LookAndFeel::textColor);
    setColour (KeyMappingEditorComponent::backgroundColourId, findColour (TreeView::backgroundColourId));

    // Directory Contents Display
    setColour (DirectoryContentsDisplayComponent::textColourId, textColor);
    setColour (DirectoryContentsDisplayComponent::highlightColourId, Colors::elemental.darker (0.6000006f));

    // List Box
    setColour (ListBox::textColourId, textColor);

    // // Slider
    // setColour (Slider::thumbColourId,               Colours::black);
    // setColour (Slider::textBoxTextColourId,         LookAndFeel::textColor);
    // setColour (Slider::trackColourId,               Colours::black);
    // setColour (Slider::textBoxBackgroundColourId,   findColour (TextEditor::backgroundColourId));
    // setColour (Slider::textBoxHighlightColourId,    findColour (TextEditor::highlightColourId));
    // setColour (Slider::textBoxOutlineColourId,      findColour (TextEditor::outlineColourId));
    // setColour (Slider::textBoxTextColourId,         findColour (TextEditor::textColourId));

    // Hyperlink button
    setColour (HyperlinkButton::textColourId, Colors::toggleBlue);

    // DockItem
    // setColour (DockItem::selectedHighlightColourId, Colors::toggleBlue);

    // // ProgressBar
    // setColour (ProgressBar::foregroundColourId, Colors::elemental);
    // setColour (ProgressBar::backgroundColourId, findColour (
    //     DocumentWindow::backgroundColourId).darker());

    // ToggleButton
    setColour (ToggleButton::tickColourId, Colors::toggleBlue.darker());

    // rotary
    setColour (Slider::rotarySliderFillColourId, Colors::toggleBlue.darker (0.3));

    // Scrollbar
    setColour (ScrollBar::thumbColourId, Colour::greyLevel (0.25f));

    // code editor. TODO
    setColour (CodeEditorComponent::backgroundColourId, findColour (Style::widgetBackgroundColorId).darker (0.6));
    setColour (CodeEditorComponent::highlightColourId, Colour (0xff1b5381));
    setColour (CodeEditorComponent::defaultTextColourId, Colour (0xffc4c4c4));
    setColour (CodeEditorComponent::lineNumberBackgroundId, findColour (Style::widgetBackgroundColorId).darker (0.55));
    setColour (CodeEditorComponent::lineNumberTextId, Colour (0xff555555));
}
//==============================================================================
Typeface::Ptr LookAndFeel::getTypefaceForFont (const Font& font)
{
#if JUCE_LINUX
    if (font.getTypefaceName() == Font::getDefaultSansSerifFontName())
    {
        return Typeface::createSystemTypefaceFor (
            BinaryData::RobotoRegular_ttf, BinaryData::RobotoRegular_ttfSize);
    }
    else if (font.getTypefaceName() == Font::getDefaultMonospacedFontName())
    {
        Font f (font);
        if (defaultMonospaceName.isEmpty())
        {
            const StringArray possible ("Courier 10 Pitch");
            const auto names = Font::findAllTypefaceNames();
            for (const auto& name : possible)
                if (names.contains (name))
                {
                    defaultMonospaceName = name;
                    break;
                }
            if (defaultMonospaceName.isEmpty())
                defaultMonospaceName = names[0];
        }

        f.setTypefaceName (defaultMonospaceName);
        f.setTypefaceStyle ("Regular");
        return Typeface::createSystemTypefaceFor (f);
    }
#endif
    return LookAndFeel_V2::getTypefaceForFont (font);
}

//==============================================================================
// MARK: default sizes

int LookAndFeel::getDefaultScrollbarWidth() { return 12; }

//
void LookAndFeel::drawRotarySlider (Graphics& g, int x, int y, int width, int height, float sliderPos, const float rotaryStartAngle, const float rotaryEndAngle, Slider& slider)
{
    const float radius = jmin (width / 2, height / 2) - 2.0f;
    const float centreX = x + width * 0.5f;
    const float centreY = y + height * 0.5f;
    const float rx = centreX - radius;
    const float ry = centreY - radius;
    const float rw = radius * 2.0f;
    const float angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
    const bool isMouseOver = slider.isMouseOverOrDragging() && slider.isEnabled();

    if (radius > 12.0f)
    {
        if (slider.isEnabled())
            g.setColour (slider.findColour (Slider::rotarySliderFillColourId).withAlpha (isMouseOver ? 1.0f : 0.7f));
        else
            g.setColour (Colour (0x80808080));

        const float thickness = 0.7f;

        {
            Path filledArc;
            filledArc.addPieSegment (rx, ry, rw, rw, rotaryStartAngle, angle, thickness);
            g.fillPath (filledArc);
        }

        {
            const float innerRadius = radius * 0.2f;
            Path p;
            p.addTriangle (-innerRadius, 0.0f, 0.0f, -radius * thickness * 1.1f, innerRadius, 0.0f);

            p.addEllipse (-innerRadius, -innerRadius, innerRadius * 2.0f, innerRadius * 2.0f);

            g.fillPath (p, AffineTransform::rotation (angle).translated (centreX, centreY));
        }

        if (slider.isEnabled())
            g.setColour (slider.findColour (Slider::rotarySliderOutlineColourId));
        else
            g.setColour (Colour (0x80808080));

        Path outlineArc;
        outlineArc.addPieSegment (rx, ry, rw, rw, rotaryStartAngle, rotaryEndAngle, thickness);
        outlineArc.closeSubPath();

        g.strokePath (outlineArc, PathStrokeType (slider.isEnabled() ? (isMouseOver ? 2.0f : 1.2f) : 0.3f));
    }
    else
    {
        if (slider.isEnabled())
            g.setColour (slider.findColour (Slider::rotarySliderFillColourId).withAlpha (isMouseOver ? 1.0f : 0.7f));
        else
            g.setColour (Colour (0x80808080));

        Path p;
        p.addEllipse (-0.4f * rw, -0.4f * rw, rw * 0.8f, rw * 0.8f);
        PathStrokeType (rw * 0.1f).createStrokedPath (p, p);

        p.addLineSegment (Line<float> (0.0f, 0.0f, 0.0f, -radius), rw * 0.2f);

        g.fillPath (p, AffineTransform::rotation (angle).translated (centreX, centreY));
    }
}

//==============================================================================
// MARK: AlertWindow
#if 0
AlertWindow* LookAndFeel::createAlertWindow (const String& title, const String& message,
                                                const String& button1, const String& button2, const String& button3,
                                                AlertWindow::AlertIconType iconType,
                                                int numButtons, Component* associatedComponent)
{
    auto boundsOffset = 4;

    auto* aw = LookAndFeel_V2::createAlertWindow (title, message, button1, button2, button3,
                                                  iconType, numButtons, associatedComponent);
    auto bounds = aw->getBounds();
    bounds = bounds.withSizeKeepingCentre (bounds.getWidth() + boundsOffset, bounds.getHeight() + boundsOffset);
    aw->setBounds (bounds);

    for (auto* child : aw->getChildren())
        if (auto* button = dynamic_cast<TextButton*> (child))
            button->setBounds (button->getBounds() + Point<int> (25, 40));

    return aw;
}

void LookAndFeel::drawAlertBox (Graphics& g, AlertWindow& alert,
                                   const Rectangle<int>& textArea, TextLayout& textLayout)
{
    auto cornerSize = 4.0f;

    g.setColour (alert.findColour (AlertWindow::outlineColourId));
    g.drawRoundedRectangle (alert.getLocalBounds().toFloat(), cornerSize, 2.0f);

    auto bounds = alert.getLocalBounds().reduced (1);
    g.reduceClipRegion (bounds);

    g.setColour (alert.findColour (AlertWindow::backgroundColourId));
    g.fillRoundedRectangle (bounds.toFloat(), cornerSize);

    auto iconSpaceUsed = 0;

    auto iconWidth = 80;
    auto iconSize = jmin (iconWidth + 50, bounds.getHeight() + 20);

    if (alert.containsAnyExtraComponents() || alert.getNumButtons() > 2)
        iconSize = jmin (iconSize, textArea.getHeight() + 50);

    Rectangle<int> iconRect (iconSize / -10, iconSize / -10,
                             iconSize, iconSize);

    if (alert.getAlertType() != AlertWindow::NoIcon)
    {
        Path icon;
        char character;
        uint32 colour;

        if (alert.getAlertType() == AlertWindow::WarningIcon)
        {
            character = '!';

            icon.addTriangle (iconRect.getX() + iconRect.getWidth() * 0.5f, (float) iconRect.getY(),
                              static_cast<float> (iconRect.getRight()), static_cast<float> (iconRect.getBottom()),
                              static_cast<float> (iconRect.getX()), static_cast<float> (iconRect.getBottom()));

            icon = icon.createPathWithRoundedCorners (5.0f);
            colour = 0x66ff2a00;
        }
        else
        {
            colour = Colour (0xff00b0b9).withAlpha (0.4f).getARGB();
            character = alert.getAlertType() == AlertWindow::InfoIcon ? 'i' : '?';

            icon.addEllipse (iconRect.toFloat());
        }

        GlyphArrangement ga;
        ga.addFittedText ({ iconRect.getHeight() * 0.9f, Font::bold },
                          String::charToString ((juce_wchar) (uint8) character),
                          static_cast<float> (iconRect.getX()), static_cast<float> (iconRect.getY()),
                          static_cast<float> (iconRect.getWidth()), static_cast<float> (iconRect.getHeight()),
                          Justification::centred, false);
        ga.createPath (icon);

        icon.setUsingNonZeroWinding (false);
        g.setColour (Colour (colour));
        g.fillPath (icon);

        iconSpaceUsed = iconWidth;
    }

    g.setColour (alert.findColour (AlertWindow::textColourId));

    Rectangle<int> alertBounds (bounds.getX() + iconSpaceUsed, 30,
                                bounds.getWidth(), bounds.getHeight() - getAlertWindowButtonHeight() - 20);

    textLayout.draw (g, alertBounds.toFloat());
}

int LookAndFeel::getAlertWindowButtonHeight()    { return 24; }
Font LookAndFeel::getAlertWindowTitleFont()      { return { 16.0f, Font::bold }; }
Font LookAndFeel::getAlertWindowMessageFont()    { return { 15.0f }; }
Font LookAndFeel::getAlertWindowFont()           { return { 13.0f }; }
#endif

// MARK: Concertina Panel

void LookAndFeel::drawConcertinaPanelHeader (Graphics& g, const Rectangle<int>& area, bool isMouseOver, bool isMouseDown, ConcertinaPanel& panel, Component& comp)
{
    g.setColour (Colour (0xff323232));
    Rectangle<int> r (area.withSizeKeepingCentre (area.getWidth(), area.getHeight() - 2));
    g.fillRect (r);
}

Font LookAndFeel::getComboBoxFont (ComboBox& box)
{
    return Font (jmin (12.0f, box.getHeight() * 0.85f));
}

// Label
Font LookAndFeel::getLabelFont (Label& label)
{
    if (nullptr != dynamic_cast<PropertyComponent*> (label.getParentComponent()))
        label.setFont (Font (13.f));
    return label.getFont();
}

//==============================================================================
void LookAndFeel::drawProgressBar (Graphics& g, ProgressBar& progressBar, int width, int height, double progress, const String& textToShow)
{
    LookAndFeel_KV1::drawProgressBar (g, progressBar, width, height, progress, textToShow);
}

// MARK toggle button

Path LookAndFeel::getTickShape (float height)
{
    static const unsigned char pathData[] = { 110, 109, 32, 210, 202, 64, 126, 183, 148, 64, 108, 39, 244, 247, 64, 245, 76, 124, 64, 108, 178, 131, 27, 65, 246, 76, 252, 64, 108, 175, 242, 4, 65, 246, 76, 252, 64, 108, 236, 5, 68, 65, 0, 0, 160, 180, 108, 240, 150, 90, 65, 21, 136, 52, 63, 108, 48, 59, 16, 65, 0, 0, 32, 65, 108, 32, 210, 202, 64, 126, 183, 148, 64, 99, 101, 0, 0 };

    Path path;
    path.loadPathFromData (pathData, sizeof (pathData));
    path.scaleToFit (0, 0, height * 2.0f, height, true);

    return path;
}

Path LookAndFeel::getCrossShape (float height)
{
    static const unsigned char pathData[] = { 110, 109, 51, 51, 255, 66, 0, 0, 0, 0, 108, 205, 204, 13, 67, 51, 51, 99, 65, 108, 0, 0, 170, 66, 205, 204, 141, 66, 108, 51, 179, 13, 67, 52, 51, 255, 66, 108, 0, 0, 255, 66, 205, 204, 13, 67, 108, 205, 204, 141, 66, 0, 0, 170, 66, 108, 52, 51, 99, 65, 51, 179, 13, 67, 108, 0, 0, 0, 0, 51, 51, 255, 66, 108, 205, 204, 98, 66, 204, 204, 141, 66, 108, 0, 0, 0, 0, 51, 51, 99, 65, 108, 51, 51, 99, 65, 0, 0, 0, 0, 108, 205, 204, 141, 66, 205, 204, 98, 66, 108, 51, 51, 255, 66, 0, 0, 0, 0, 99, 101, 0, 0 };

    Path path;
    path.loadPathFromData (pathData, sizeof (pathData));
    path.scaleToFit (0, 0, height * 2.0f, height, true);

    return path;
}

void LookAndFeel::drawToggleButton (Graphics& g, ToggleButton& button, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown)
{
    auto fontSize = jmin (13.0f, button.getHeight() * 0.75f);
    auto tickWidth = fontSize * 1.1f;

    drawTickBox (g, button, 4.0f, (button.getHeight() - tickWidth) * 0.5f, tickWidth, tickWidth, button.getToggleState(), button.isEnabled(), shouldDrawButtonAsHighlighted, shouldDrawButtonAsDown);

    g.setColour (button.findColour (ToggleButton::textColourId));
    g.setFont (fontSize);

    if (! button.isEnabled())
        g.setOpacity (0.5f);

    g.drawFittedText (button.getButtonText(),
                      button.getLocalBounds().withTrimmedLeft (roundToInt (tickWidth) + 10).withTrimmedRight (2),
                      Justification::centredLeft,
                      10);
}

void LookAndFeel::drawTickBox (Graphics& g, Component& component, float x, float y, float w, float h, const bool ticked, const bool isEnabled, const bool shouldDrawButtonAsHighlighted, const bool shouldDrawButtonAsDown)
{
    ignoreUnused (isEnabled, shouldDrawButtonAsHighlighted, shouldDrawButtonAsDown);

    Rectangle<float> tickBounds (x, y, w, h);

    g.setColour (component.findColour (ToggleButton::tickDisabledColourId));
    g.drawRoundedRectangle (tickBounds, 4.0f, 1.0f);

    if (ticked)
    {
        g.setColour (component.findColour (ToggleButton::tickColourId));
        auto tick = getTickShape (0.75f);
        g.fillPath (tick, tick.getTransformToScaleToFit (tickBounds.reduced (4, 5).toFloat(), false));
    }
}

void LookAndFeel::changeToggleButtonWidthToFitText (ToggleButton& button)
{
    auto fontSize = jmin (15.0f, button.getHeight() * 0.75f);
    auto tickWidth = fontSize * 1.1f;

    Font font (fontSize);

    button.setSize (font.getStringWidth (button.getButtonText()) + roundToInt (tickWidth) + 14, button.getHeight());
}

// MARK: Property Panel

void LookAndFeel::drawPropertyPanelSectionHeader (Graphics& g, const String& name, bool isOpen, int width, int height)
{
    String text = isOpen ? " - " : " + ";
    text << name;
    g.setColour (isOpen ? LookAndFeel::textBoldColor : LookAndFeel::textColor);
    g.drawText (text, 0, 0, width, height, Justification::centredLeft);
}

void LookAndFeel::drawPropertyComponentBackground (Graphics& g, int width, int height, PropertyComponent& pc)
{
    const auto r = getPropertyComponentContentPosition (pc);
    g.setColour (widgetBackgroundColor.darker (0.0015));
    g.fillRect (0, 0, r.getX(), height - 1);

    g.setColour (pc.findColour (PropertyComponent::backgroundColourId));
    g.fillRect (r.getX(), 0, width - r.getX(), height - 1);
}

static int getPropertyComponentIndent (PropertyComponent& component)
{
    return jmin (10, component.getWidth() / 10);
}

void LookAndFeel::drawPropertyComponentLabel (Graphics& g, int width, int height, PropertyComponent& component)
{
    ignoreUnused (width);

    const auto indent = getPropertyComponentIndent (component);

    g.setColour (component.findColour (PropertyComponent::labelTextColourId)
                     .withMultipliedAlpha (component.isEnabled() ? 1.0f : 0.6f));

    // g.setFont (jmin (height, 24) * 0.65f);
    g.setFont (12.5f);

    auto r = getPropertyComponentContentPosition (component);

    g.drawFittedText (component.getName(),
                      indent,
                      r.getY(),
                      r.getX() - 5,
                      r.getHeight(),
                      Justification::centredLeft,
                      2);
}

Rectangle<int> LookAndFeel::getPropertyComponentContentPosition (PropertyComponent& component)
{
    const auto textW = jmin (130, component.getWidth() / 2);
    return { textW, 0, component.getWidth() - textW, component.getHeight() - 1 };
}

// MARK: Treeview
void LookAndFeel::drawTreeviewPlusMinusBox (Graphics& g, const Rectangle<float>& area, Colour backgroundColour, bool isOpen, bool isMouseOver)
{
    LookAndFeel_KV1::drawTreeviewPlusMinusBox (g, area, backgroundColour, isOpen, isMouseOver);
}

} // namespace element
