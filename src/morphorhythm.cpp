#include "morphorhythm.hpp"
#include "ofMain.h"

bool morphorhythm::PitchPicker::isPressed(int x, int y){
    auto click_pos = glm::vec2{x,y};
    if ( click_pos.x >= pos_.x && click_pos.x <= pos_.x + width_ && click_pos.y >= pos_.y){
        return true;
    }else{
        return false;
    }
}

void morphorhythm::PitchPicker::setColor(ofColor color){
    color_ = color;
}

const glm::vec2 morphorhythm::PitchPicker::getpos(){
    return pos_;
}

void morphorhythm::PitchPicker::draw() const{
    ofPushStyle();
    ofSetColor(color_);
    ofDrawRectangle(pos_, width_, height_);
    ofPopStyle();
}

