#ifndef morphorhythm_hpp
#define morphorhythm_hpp

#include "angle_sequencer.hpp"
#include "score.hpp"

namespace morphorhythm {
    class PitchPicker{
    public:
        PitchPicker(const glm::vec2& pos, float width, float height)
        : pos_{pos}, width_{width}, height_{height}
        {}
        ~PitchPicker(){}
        
        bool isPressed(int x, int y);
        void setColor(ofColor color);
        const glm::vec2 getpos();
        void draw() const;
        
    private:
        glm::vec2 pos_;
        float width_,height_;
        ofColor color_;
    };
    
    
    
    
}
#endif /* morphorhythm_hpp */
