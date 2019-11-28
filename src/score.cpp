#include "score.hpp"
#include "ofMain.h"

using namespace morphorhythm;

const std::unordered_map<int, ofColor> kDefaultPitchColorMap {
    {36, ofColor(48, 139, 195)},
    {38, ofColor(216, 63, 111)}
};

Score::Score():radius_{300.f}, translate_{0.0, 0.0}, note_size_ratio_{0.1}, pitch_color_map_{kDefaultPitchColorMap}
{
    for(auto i = 0; i < kSequencerNum; ++i){
        sequencers_[i] = std::make_shared<AngleSequencer>();
    }
}

Score::~Score(){}

void Score::setRadius(float radius){
    radius_ = radius;
    
    note_size_ratio_ = ofMap(radius_, 0, 500, 0.0, 0.2);
}

void Score::translate(const glm::vec2& p){
    translate_ = p;
}

void Score::setPitchColorMap(const std::unordered_map<int, ofColor>& m){
    pitch_color_map_ = m;
}



void Score::setPitch(int x, int y, int pitch){
    auto click_pos = glm::vec2(x, y);
    auto interval = radius_ / kSequencerNum;
    auto r = interval;
    for(const auto& seq : sequencers_){
        for(const auto& n : seq->getNotes()){
            auto note_pos = glm::vec2{ r*cos(ofDegToRad(n->angle)), r*sin(ofDegToRad(n->angle))} + translate_;
            if (ofDist(click_pos.x, click_pos.y, note_pos.x, note_pos.y) < note_size_ratio_ * n -> midi_data.velocity){
                n -> midi_data.pitch = pitch;
            }
        }
        r += interval;
    }
}

const float Score::getRadius() const{
    return radius_;
}


std::shared_ptr<AngleSequencer> Score::getSequencer(int index) const {
    if ( index > kSequencerNum-1){
        return nullptr;
    }else{
        return sequencers_[index];
    }
}

const int Score::getSequencerNum() const{
    return kSequencerNum;
}

const glm::vec2& Score::getTranslation() const{
    return translate_;
}

void Score::morph(const Score& target, float duration){
    for(auto i = 0; i < getSequencerNum(); ++i){
        getSequencer(i) -> morph(target.getSequencer(i), duration);
    }
}


void Score::draw()const{
    
    ofPushMatrix();
    ofTranslate(translate_);
    
    auto interval = radius_ / kSequencerNum;
    auto r = interval;
    
    for(const auto& seq : sequencers_){
        auto has_valid_note = std::any_of(seq->begin(), seq->end(), [](const std::shared_ptr<Note> x){
            return x->midi_data.pitch > 0;
        });
        if(has_valid_note){
            ofNoFill();
            ofSetColor(201, 201, 201);
            ofDrawCircle(0, 0, r);
        }else{
            ofNoFill();
            ofSetColor(6, 3, 3);
            ofDrawCircle(0, 0, r);
        }
            ofFill();
            for(const auto& n : seq->getNotes()){
                auto x = r * cos(ofDegToRad(n -> angle));
                auto y = r * sin(ofDegToRad(n -> angle));;
                auto itr = pitch_color_map_.find(n->midi_data.pitch);
                if (itr != pitch_color_map_.end()){
                    ofSetColor(itr->second);
                }else{
                    ofSetColor(201, 201, 201);
                }
            
                ofDrawCircle(x, y, note_size_ratio_*n->midi_data.velocity);
            }
        r += interval;
    }
    ofPopMatrix();
}
