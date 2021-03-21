/*
    This file is part of Element
    Copyright (C) 2019  Kushview, LLC.  All rights reserved.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "Version.h"

#ifndef TEST_CURRENT_VERSION
 #define TEST_CURRENT_VERSION 0
#endif

namespace Element {

Version::Version() { }
Version::~Version() { }

StringArray Version::segments (const String& versionString)
{
    StringArray segments;
    segments.addTokens (versionString, ",.", "");
    segments.trim();
    segments.removeEmptyStrings();
    return segments;
}

int Version::asHexInteger (const String& versionString)
{
    const StringArray segs (segments (versionString));
    
    int value = (segs[0].getIntValue() << 16)
              + (segs[1].getIntValue() << 8)
              + segs[2].getIntValue();
    
    if (segs.size() >= 4)
        value = (value << 8) + segs[3].getIntValue();
    
    return value;
}

CurrentVersion::CurrentVersion()
    : Thread("elVersionCheck"),
      version (ProjectInfo::versionString),
      hasChecked (false) { }

CurrentVersion::~CurrentVersion()
{
    cancel();
    signalThreadShouldExit();
    notify();
    waitForThreadToExit (timeout + 1);
    DBG ("CurrentVersion::~CurrentVersion()");
}

void CurrentVersion::checkAfterDelay (const int milliseconds, const bool showUpToDate)
{
    auto* cv = new CurrentVersion();
    cv->timeout = milliseconds;
    cv->hasChecked = false;
    cv->shouldShowUpToDateMessage = showUpToDate;
    cv->startThread();
}

bool CurrentVersion::isNewerVersionAvailable()
{
    if (hasChecked)
        return result;
    
   #if TEST_CURRENT_VERSION
    const URL url ("http://kushview.dev/?edd_action=get_version&item_id=15");
   #else
    const URL url ("https://kushview.net/?edd_action=get_version&item_id=20");
   #endif
   
    if (threadShouldExit() || cancelled.get())
        return false;
    
    bool result = false;
    auto stream = url.createInputStream (false, nullptr, nullptr, {}, 300, nullptr, nullptr, 5, {});

    WebInputStream::Listener dummyListener;
    if (stream && ((WebInputStream*)stream.get())->connect (&dummyListener))
    {
        var data;
        const Result res (JSON::parse (stream->readEntireStreamAsString(), data));
        if (!res.failed() && data.isObject())
        {
            permalink   = "https://kushview.net/element/download/"; // data["homepage"].toString();
            version     = data["stable_version"].toString();
            result      = Version::asHexInteger(version) > ProjectInfo::versionNumber;
        }
    }

    return result;
}

void CurrentVersion::run()
{
    hasChecked = false;
    result = isNewerVersionAvailable();
    hasChecked = true;
    startTimer (cancelled.get() ? 4 : timeout);
}

void CurrentVersion::timerCallback()
{
    stopTimer();

    if (! cancelled.get())
    {
        if (result)
        {
            if (AlertWindow::showOkCancelBox (AlertWindow::NoIcon, "New Version",
                                            "A new version is available: " + version, "Download"))
            {
                URL(permalink).launchInDefaultBrowser();
            }
        }
        else if (shouldShowUpToDateMessage)
        {
            String msg = "Element v";
            msg << ProjectInfo::versionString << " is currently the newest version available.";
            AlertWindow::showMessageBox(AlertWindow::InfoIcon, "You're up-to-date.", msg);
        }
    }

    delete this;
}

}
