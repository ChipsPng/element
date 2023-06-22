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
#include <element/ui/menumodels.hpp>
#include <element/services/guiservice.hpp>
#include "services/sessionservice.hpp"
#include <element/ui/content.hpp>
#include "gui/MainMenu.h"
#include "gui/MainWindow.h"
#include "session/commandmanager.hpp"
#include "commands.hpp"
#include <element/context.hpp>
#include <element/settings.hpp>
#include "utils.hpp"

namespace element {

MainWindow::MainWindow (Context& g)
    : DocumentWindow (Util::appName(), Colours::darkgrey, DocumentWindow::allButtons, false),
      world (g)
{
    auto& gui = *g.services().find<GuiService>();

    auto _mainMenu = new MainMenu (*this, gui.commands());
    mainMenu.reset (_mainMenu);
    _mainMenu->setupMenu();

    nameChanged();

    g.session()->addChangeListener (this);
    addKeyListener (gui.commands().getKeyMappings());
    setUsingNativeTitleBar (true);
    setResizable (true, false);
}

MainWindow::~MainWindow()
{
    setMenuBar (nullptr);
    world.session()->removeChangeListener (this);
    mainMenu.reset();
}

void MainWindow::changeListenerCallback (ChangeBroadcaster*)
{
    refreshName();
}

void MainWindow::refreshName()
{
    nameChanged();
}

void MainWindow::setMainMenuModel (std::unique_ptr<MainMenuBarModel> model)
{
    if (mainMenu)
    {
        setMenuBar (nullptr);
        mainMenu.reset();
    }

    if (model)
    {
        setMenuBar (model.get());
#if JUCE_MAC
        MenuBarModel::setMacMainMenu (model.get(), model->getMacAppMenu(), "");
        setMenuBar (nullptr);
#endif
        refreshMenu();
    }

    mainMenu.reset (model.release());
}

void MainWindow::nameChanged()
{
    if (windowTitleFunction != nullptr)
    {
        setName (windowTitleFunction());
        return;
    }

    nameChangedSession();
}

void MainWindow::nameChangedSession()
{
    String title = Util::appName();

    auto session = world.session();
    SessionService* controller = nullptr;
    if (nullptr != dynamic_cast<ContentComponent*> (getContentComponent()))
        controller = services().find<SessionService>();

    if (nullptr == session || nullptr == controller)
    {
        setName (title);
        return;
    }

    auto sessionName = session->getName().trim();
    auto graphName = session->getCurrentGraph().getName().trim();
    if (sessionName.isEmpty())
    {
        const auto file = controller->getSessionFile();
        if (file.existsAsFile())
            sessionName = file.getFileNameWithoutExtension();
    }

    if (sessionName.isEmpty())
        sessionName = "Untitled Session";
    if (graphName.isEmpty())
        graphName = "Untitled Graph";

    title << " - " << sessionName << ": " << graphName;
    setName (title);
}

void MainWindow::closeButtonPressed()
{
    JUCEApplication* app (JUCEApplication::getInstance());
    app->systemRequestedQuit();
}

void MainWindow::minimiseButtonPressed()
{
    auto& gui = *world.services().find<GuiService>();
    if (world.settings().isSystrayEnabled())
        gui.commands().invokeDirectly (Commands::toggleUserInterface, true);
    else
        DocumentWindow::minimiseButtonPressed();
}

void MainWindow::activeWindowStatusChanged()
{
    if (nullptr == getContentComponent())
        return;
    auto& gui = *services().find<GuiService>();
    gui.checkForegroundStatus();
}

void MainWindow::refreshMenu()
{
    if (mainMenu)
        mainMenu->menuItemsChanged();
}

Services& MainWindow::services()
{
    jassert (nullptr != dynamic_cast<ContentComponent*> (getContentComponent()));
    return (dynamic_cast<ContentComponent*> (getContentComponent()))
        ->services();
}

} // namespace element
