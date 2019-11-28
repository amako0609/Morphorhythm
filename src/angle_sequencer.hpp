#ifndef angle_sequencer_hpp
#define angle_sequencer_hpp

#include <list>
#include <string>
#include <memory>
#include "ofxOsc.h"

namespace morphorhythm{
    
    struct MidiData {
        int pitch, velocity;
    };
    
    struct Note {
        Note(int pitch=-1, int velocity=100):midi_data{pitch, velocity},angle{0}, buffer_midi_data{nullptr}{}
        Note(const Note&)=default;
        float angle;
        MidiData midi_data;
        std::shared_ptr<MidiData> buffer_midi_data;
    };
    
    
    class AngleSequencer{
    public:
        using iterator = std::list<std::shared_ptr<Note>>::iterator;
        
        AngleSequencer();
        ~AngleSequencer();
        void addNote();
        void removeNote();
        void updateAngle(); // 1 degree up and make osc message
        void rotate(float angle); // offset angle
        void morph(const AngleSequencer& target, float duration);
        void morph(std::shared_ptr<AngleSequencer> target, float duration);
        const int getAngle() const;
        const int getMaxNoteNum() const;
        const int getNoteNum() const;
        const float getVolume() const;
        void setVolume(float volume);
        void setAngle(float angle);
        std::list<std::shared_ptr<Note>> getNotes() const;
        void setNotes(std::list<std::shared_ptr<Note>> newNotes) {notes_ = newNotes;}
        iterator begin() { return notes_.begin();}
        iterator end()   { return notes_.end(); }
        
        
    private:
        std::list<std::shared_ptr<Note>> notes_;
        static const int kMaxNoteNum = 360; // because ...
        int angle_;
        float offset_angle_;
        //        ofxOscSender sender_;
        float volume_;
        
    };
    
}

#endif /* angle_sequencer_hpp */

