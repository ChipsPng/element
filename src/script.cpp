
#include <element/gzip.hpp>
#include <element/script.hpp>
#include <element/tags.hpp>

#include "scripting/bindings.hpp"
#include "sol/sol.hpp"

namespace element {
using namespace juce;

static ScriptInfo parseScriptComments (const String& buffer)
{
    static const StringArray tags = { "@author", "@script", "@description", "@kind" };

    ScriptInfo desc;
    desc.type = "";
    const auto lines = StringArray::fromLines (buffer);
    int index = 0;
    bool inBlock = false;
    bool finished = false;
    for (index = 0; index < lines.size(); ++index)
    {
        const auto line = lines[index].trim();

        if (! inBlock)
            inBlock = line.startsWith ("--[[");

        if (inBlock || line.startsWith ("--"))
        {
            for (const auto& tag : tags)
            {
                if (line.contains (tag))
                {
                    const auto value = line.fromFirstOccurrenceOf (tag, false, false).trimStart().upToFirstOccurrenceOf ("--]]", false, false).trimEnd();

                    // DBG (tag.replace("@","") << " = " << value);

                    if (tag == "@kind" && desc.type.isEmpty())
                    {
                        desc.type = value.fromLastOccurrenceOf (".", false, false);
                    }
                    else if (tag == "@script" && desc.name.isEmpty())
                    {
                        desc.name = value;
                    }
                    else if (tag == "@author" && desc.author.isEmpty())
                    {
                        desc.author = value;
                    }
                    else if (tag == "@description" && desc.description.isEmpty())
                    {
                        desc.description = value;
                    }
                }
            }

            if (inBlock)
            {
                inBlock = ! line.contains ("--]]");
                finished = ! inBlock;
            }
        }
        else if (! inBlock && ! line.startsWith ("--"))
        {
            finished = true;
        }

        if (finished)
        {
            // DBG("finihed at line index: " << index);
            // DBG("LINE: " << lines[index]);
            break;
        }
    }

    return desc;
}

ScriptInfo ScriptInfo::read (lua_State* L, const String& buffer)
{
    sol::state_view view (L);
    ScriptInfo desc;
    try
    {
        sol::table script;
        auto result = view.script (buffer.toRawUTF8());
        if (result.get_type() == sol::type::table)
            script = result;

        if (script.valid())
        {
            desc.name = script["name"].get_or<std::string> ("");
            desc.type = script["type"].get_or<std::string> ("");
            desc.author = script["author"].get_or<std::string> ("");
            desc.description = script["description"].get_or<std::string> ("");
        }
    } catch (const std::exception&)
    {
        desc = {};
    }

    return desc;
}

ScriptInfo ScriptInfo::read (const String& buffer)
{
    ScriptInfo desc;
    sol::state lua;
    element::Lua::initializeState (lua);
    return read (lua, buffer);
}

ScriptInfo ScriptInfo::read (File file)
{
    return read (file.loadFileAsString());
}

ScriptInfo ScriptInfo::parse (const String& buffer)
{
    auto desc = parseScriptComments (buffer);
    desc.code = "";
    return desc;
}

ScriptInfo ScriptInfo::parse (File file)
{
    ScriptInfo desc;

    if (file.existsAsFile())
    {
        desc = parseScriptComments (file.loadFileAsString());
        desc.code = URL (file).toString (false);
    }

    return desc;
}

//==============================================================================
Script::Script() : Model (types::Script) { setMissing(); }
Script::Script (const juce::ValueTree& data) : Model (data) {}

Script::Script (const juce::String& source)
    : Model (types::Script)
{
    setMissing();
    setCode (source);
}

Script::Script (const Script& o) { Script::operator= (o); }
Script& Script::operator= (const Script& o)
{
    Model::operator= (o);
    return *this;
}

void Script::setName (const String& newName)
{
    setProperty (Tags::name, newName);
}
String Script::name() const noexcept { return getProperty (Tags::name); }

juce::String Script::code() const noexcept { return gzip::decode (getProperty (tags::code).toString()); }
void Script::setCode (const String& newCode) { setProperty (tags::code, gzip::encode (newCode)); }
bool Script::valid() const noexcept { return hasType (types::Script) && hasProperty (tags::code) && hasProperty (tags::name); }

void Script::setMissing()
{
    stabilizePropertyString (tags::name, types::Script.toString());
    stabilizePropertyString (tags::code, "");
}

Script Script::anonymous()
{
    static String body (R"(--- New Anonymous Script.
--
-- This is an anonymous script. Do as you please.
--
-- @script      amp
-- @kind        Anonymous
-- @license     GPL v3
-- @author      Michael Fisher
)");

    Script script (body);
    script.setName ("Anonymous Script");
    return script;
}

Script Script::view()
{
    static String body (R"(--- New View Script.
--
-- This is a content View script.
--
-- @script      amp
-- @kind        View
-- @license     GPL v3
-- @author      Michael Fisher
)");

    Script script (body);
    script.setName ("View Script");
    return script;
}

Script Script::make (const juce::String& name, const juce::Identifier& kind)
{
    juce::String buffer;
    // clang-format off
    buffer << 
        "--- A new " << kind.toString() << " script." << juce::newLine <<
        "-- " << juce::newLine <<
        "-- Script description." << juce::newLine <<
        "-- " << juce::newLine <<
        "-- @kind " << kind.toString() << juce::newLine <<
        "-- @license GPL3-or-later"  << juce::newLine <<
        "-- @author Your Name" << juce::newLine <<
        juce::newLine;
    // clang-format on
    Script script (buffer);
    script.setName (name);
    return script;
}
} // namespace element
