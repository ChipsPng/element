#include "Tests.h"
#include "engine/nodes/ProgramChangeMapNode.h"

namespace Element {

class ProgramChangeMapTest : public UnitTestBase
{
public:
    ProgramChangeMapTest() : UnitTestBase ("Program Change Map", "nodes", "programChangeMap") {}
    void initialise() override { }
    void shutdown() override { }
    void runTest() override
    {
        beginTest ("port count");
        GraphProcessor graph;
        GraphNodePtr node = graph.addNode (new ProgramChangeMapNode());
        expect (node->getNumPorts (PortType::Midi, true) == 1);
        expect (node->getNumPorts (PortType::Midi, false) == 1);
        auto* pgc = dynamic_cast<ProgramChangeMapNode*> (node.get());
        pgc->clear();
        ProgramChangeMapNode::ProgramEntry entry;

        beginTest ("3 maps to 4");
        pgc->addProgramEntry ("Program 1", 3, 4);
        entry = pgc->getProgramEntry (0);
        expect (entry.name == "Program 1" && entry.in == 3 && entry.out == 4);

        beginTest ("1 to 1 map");
        pgc->addProgramEntry ("Program 2", 5);
        entry = pgc->getProgramEntry (1);
        expect (entry.in == 5 && entry.out == 5);

        beginTest ("does not duplicate");
        pgc->addProgramEntry ("Program Edit", 3, 6);
        entry = pgc->getProgramEntry (0);
        expect (entry.in = 3 && entry.out == 6);
        expect (pgc->getNumProgramEntries() == 2);

        beginTest ("renders mappings");
        OwnedArray<MidiBuffer> buffers;
        Array<int> channels;
        buffers.add (new MidiBuffer());
        channels.add (0);

        MidiPipe pipe (buffers, channels);
        AudioSampleBuffer audio;
        audio.setSize (2, 1024, false, true, false);

        auto* midi = pipe.getWriteBuffer (0);
        midi->addEvent (MidiMessage::programChange (1, 3), 100);
        midi->addEvent (MidiMessage::programChange (1, 5), 200);
        midi->addEvent (MidiMessage::noteOn (1, 12, static_cast<uint8> (50)), 300);
        midi->addEvent (MidiMessage::noteOff (1, 12), 300);
        pgc->render (audio, pipe);
        
        MidiBuffer::Iterator iter (*midi);
        MidiMessage msg; int frame = 0, index = 0;
        while (iter.getNextEvent (msg, frame))
        {
            switch (index)
            {
                case 0: expect (msg.isProgramChange() && msg.getProgramChangeNumber() == 6); break;
                case 1: expect (msg.isProgramChange() && msg.getProgramChangeNumber() == 5); break;
                case 2: expect (msg.isNoteOn()); break;
                case 3: expect (msg.isNoteOff()); break;
            }

            ++index;
        }

        pgc = nullptr;
        node = nullptr;
        graph.clear();
    }
};

static ProgramChangeMapTest sProgramChangeMapTest;

}
