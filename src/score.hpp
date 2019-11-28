#ifndef score_hpp
#define score_hpp

#include "angle_sequencer.hpp"
#include <array>
#include <unordered_map>
#include "glm/glm.hpp"
#include "ofColor.h"

namespace morphorhythm{
    enum class NoteType { kKick, kSnare };
    
    class Score {
    public:
        Score();
        ~Score();
        void translate(const glm::vec2& p);
        void morph(const Score& target, float duration);
        void draw() const;
        
        // setter
        void setRadius(float radius);
        void setPitch(int x, int y, int pitch);
        void setPitchColorMap(const std::unordered_map<int, ofColor>& m);
        void eraseLayer(const std::shared_ptr<AngleSequencer>& s);
        
        // Getter
        const float      getRadius() const;
        const int        getSequencerNum() const;
        const glm::vec2& getTranslation() const;
        std::shared_ptr<AngleSequencer> getSequencer(int index) const;
        
        
        
    private:
        static const int kSequencerNum = 6;
        std::array<std::shared_ptr<AngleSequencer>, kSequencerNum> sequencers_;
        glm::vec2 translate_;
        float radius_;
        float note_size_ratio_;
        std::unordered_map<int, ofColor> pitch_color_map_;
        
    };
}

#endif /* score_hpp */
