#include "angle_sequencer.hpp"
#include <vector>
#include "ofxFunctionalEasing.h"

using namespace morphorhythm;

AngleSequencer::AngleSequencer()
:notes_{}, angle_{0}, offset_angle_{0.0f}, volume_{1.0} {
}


AngleSequencer::~AngleSequencer(){
    
}
void AngleSequencer::addNote(){
    if ( notes_.size() <= kMaxNoteNum){
        notes_.push_back(std::make_shared<Note>());
    }else{
        return;
    }
    
    auto angle_interval = 360. / notes_.size();
    float angle = 0.0 + offset_angle_;
    for(const auto& n : notes_){
        n -> angle = angle;
        angle+=angle_interval;
    }
}



void AngleSequencer::removeNote(){
    if (notes_.size() > 0 ) {
        notes_.pop_back();
    }else{
        return;
    }
    
    auto angle_interval = 360. / notes_.size();
    float angle = 0.0 + offset_angle_;
    for(const auto& n : notes_){
        n -> angle = angle;
        angle+=angle_interval;
    }
}


void AngleSequencer::updateAngle(){
    ++angle_;
    if (angle_ >= 360) angle_ = 0;
}

void AngleSequencer::morph(const AngleSequencer& target, float duration){
    /* ターゲットのAngleSequecnerにモーフィング */
    
    // note数が同じ場合
    auto target_notes = target.getNotes();
    if (notes_.size() == target.getNotes().size()){
        auto itr = target_notes.begin();
        for(const auto& n : notes_){
            auto angle_from = n      -> angle;
            auto angle_to   = (*itr) -> angle;
            if ( abs(angle_to - angle_from) > abs(angle_to - (angle_from+360))){
                angle_from += 360;
            }
            if ( n -> midi_data.pitch != (*itr)->midi_data.pitch){
                auto velocity_from = n -> midi_data.velocity;
                auto velocity_to   = 0;
                auto angle_half_to   = angle_to /2.;
                ofxFunctionalEasing([=](float progress){
                    n -> angle = ofLerp(angle_from, angle_half_to, progress);
                    n -> midi_data.velocity = ofLerp(velocity_from, velocity_to, progress);
                }, duration/2., 0.0, "", [itr,duration,n,angle_to,angle_half_to](const std::string){
                    auto velocity_from = 0;
                    auto velocity_to  = (*itr)->midi_data.velocity;
                    n -> midi_data.pitch = (*itr)->midi_data.pitch;
                    ofxFunctionalEasing([=](float progress){
                        n -> angle = ofLerp(angle_half_to, angle_to, progress);
                        n -> midi_data.velocity = ofLerp(velocity_from, velocity_to, progress);
                    }, duration/2., 0.0);
                });
            }else{
                auto velocity_from = n      -> midi_data.velocity;
                auto velocity_to   = (*itr) -> midi_data.velocity;
                ofxFunctionalEasing([=](float progress){
                    n -> angle = ofLerp(angle_from, angle_to, progress);
                    auto tmp_angle = ofLerp(angle_from, angle_to, progress);
                    if (tmp_angle >= 360){
                        n ->angle = tmp_angle - 360;
                    }else{
                        n -> angle = tmp_angle;
                    }
                    n -> midi_data.velocity = ofLerp(velocity_from, velocity_to, progress);
                }, duration, 0.0);
            }
            ++itr;
        }
    }else{
        // noteの数がターゲットより多い場合
        if ( notes_.size() > target_notes.size()){
            auto itr = target_notes.begin();
            for(auto b = notes_.begin(); b != notes_.end(); ++b){
                if (itr == target_notes.end()){
                    auto velocity_from =  (*b)-> midi_data.velocity;
                    auto velocity_to   = 0;
                    ofxFunctionalEasing([=](float progress){
                        (*b) ->midi_data.velocity = ofLerp(velocity_from, velocity_to, progress);
                    }, duration, 0.0,"",[=](const std::string){
                        notes_.erase(b);
                    });
                }else{
                    auto angle_from = (*b) -> angle;
                    auto angle_to   = (*itr) -> angle;
                    if ( abs(angle_to - angle_from) > abs(angle_to - (angle_from+360))){
                        angle_from += 360;
                    }
                    
                    
                    auto velocity_from = (*b) -> midi_data.velocity;
                    auto velocity_to   = (*itr) -> midi_data.velocity;
                    ofxFunctionalEasing([=](float progress){
                        auto tmp_angle = ofLerp(angle_from, angle_to, progress);
                        if (tmp_angle >= 360){
                            (*b) -> angle = tmp_angle - 360;
                        }else{
                            (*b) -> angle = tmp_angle;
                        }
                        (*b) ->midi_data.velocity = ofLerp(velocity_from, velocity_to, progress);
                    }, duration, 0.0);
                    ++itr;
                }
            }
        }
        
        // noteの数がターゲットより少ない場合
        if ( notes_.size() < target_notes.size()){
            auto itr = notes_.begin();
            for(auto b = target_notes.begin(); b != target_notes.end(); ++b){
                if (itr != notes_.end()){
                    auto angle_from  = (*itr) -> angle;
                    auto angle_to = (*b) -> angle;
                    if ( abs(angle_to - angle_from) > abs(angle_to - (angle_from+360))){
                        angle_from += 360;
                    }
                    auto velocity_from   = (*itr) -> midi_data.velocity;
                    auto velocity_to     = (*b) -> midi_data.velocity;
                    ofxFunctionalEasing([=](float progress){
                        auto tmp_angle = ofLerp(angle_from, angle_to, progress);
                        if (tmp_angle >= 360){
                            (*itr) -> angle = tmp_angle - 360;
                        }else{
                            (*itr) -> angle = tmp_angle;
                        }
                        (*itr) -> midi_data.velocity = ofLerp(velocity_from, velocity_to, progress);
                    }, duration, 0.0);
                    ++itr;
                }else{
                    auto velocity_from =  0;
                    auto velocity_to   =  (*b)->midi_data.velocity;
                    notes_.push_back(std::make_shared<Note>((*b)->midi_data.pitch, velocity_from));
                    auto n = notes_.back();
                    n->angle = (*b) ->angle;
                    ofxFunctionalEasing([=](float progress){
                        n ->midi_data.velocity = ofLerp(velocity_from, velocity_to, progress);
                    }, duration, 0.0);
                    itr = notes_.end();
                }
            }
        }
    }
}


void AngleSequencer::morph(std::shared_ptr<AngleSequencer> target, float duration){
    
    auto target_notes = target -> getNotes();
    // note数が同じ場合
    if (notes_.size() == target -> getNotes().size()){
        auto itr = target_notes.begin();
        //angle
        for(const auto& n : notes_){
            auto angle_from = n      -> angle;
            auto angle_to   = (*itr) -> angle;
            if ( abs(angle_to - angle_from) > abs(angle_to - (angle_from+360))){
                angle_from += 360;
            }
            ofxFunctionalEasing([=](float progress){
                n -> angle = ofLerp(angle_from, angle_to, progress);
                auto tmp_angle = ofLerp(angle_from, angle_to, progress);
                if (tmp_angle >= 360){
                    n ->angle = tmp_angle - 360;
                }else{
                    n -> angle = tmp_angle;
                }
            }, duration, 0.0);
            
            if ( n -> midi_data.pitch != (*itr)->midi_data.pitch){
                auto target_pitch    = (*itr)->midi_data.pitch;
                auto target_velocity = (*itr)->midi_data.velocity;
                auto velocity_from = n -> midi_data.velocity;
                auto velocity_to   = 0;
                ofxFunctionalEasing([=](float progress){
                    n -> midi_data.velocity = ofLerp(velocity_from, velocity_to, progress);
                }, duration/2., 0.0, "", [=](const std::string){
                    //ofLog() << "fwfew";
                    auto velocity_from = 0;
                    auto velocity_to     = target_velocity;
                    n -> midi_data.pitch = target_pitch;
                    ofxFunctionalEasing([=](float progress){
                        n -> midi_data.velocity = ofLerp(velocity_from, velocity_to, progress);
                    }, duration/2., 0.0);
                });
            }else{
                auto velocity_from = n      -> midi_data.velocity;
                auto velocity_to   = (*itr) -> midi_data.velocity;
                ofxFunctionalEasing([=](float progress){
                    n -> midi_data.velocity = ofLerp(velocity_from, velocity_to, progress);
                }, duration, 0.0);
            }
            ++itr;
        }
    }else{
        
        // noteの数がターゲットより多い場合
        if ( notes_.size() > target_notes.size()){
            auto itr = target_notes.begin();
            for(auto b = notes_.begin(); b != notes_.end(); ++b){
                if (itr == target_notes.end()){
                    auto velocity_from =  (*b) -> midi_data.velocity;
                    auto velocity_to   = 0;
                    ofxFunctionalEasing([=](float progress){
                        (*b) ->midi_data.velocity = ofLerp(velocity_from, velocity_to, progress);
                    }, duration, 0.0,"",[=](const std::string){
                        notes_.erase(b);
                    });
                }else{
                    auto angle_from = (*b) -> angle;
                    auto angle_to   = (*itr) -> angle;
                    if ( abs(angle_to - angle_from) > abs(angle_to - (angle_from+360))){
                        angle_from += 360;
                    }
                    
                    ofxFunctionalEasing([=](float progress){
                        (*b) -> angle = ofLerp(angle_from, angle_to, progress);
                        auto tmp_angle = ofLerp(angle_from, angle_to, progress);
                        if (tmp_angle >= 360){
                            (*b) ->angle = tmp_angle - 360;
                        }else{
                            (*b) -> angle = tmp_angle;
                        }
                    }, duration, 0.0);
                    
                    
                    if((*b) -> midi_data.pitch != (*itr) -> midi_data.pitch){
                        auto target_pitch    = (*itr)->midi_data.pitch;
                        auto target_velocity = (*itr)->midi_data.velocity;
                        auto velocity_from = (*b) -> midi_data.velocity;
                        auto velocity_to   = 0;
                        ofxFunctionalEasing([=](float progress){
                            (*b) ->midi_data.velocity = ofLerp(velocity_from, velocity_to, progress);
                        }, duration/2.0, 0.0,"", [=](const std::string){
                            auto velocity_from = 0;
                            auto velocity_to     = target_velocity;
                            (*b) -> midi_data.pitch = target_pitch;
                            ofxFunctionalEasing([=](float progress){
                                (*b) -> midi_data.velocity = ofLerp(velocity_from, velocity_to, progress);
                            }, duration/2., 0.0);
                        });
                    }else{
                        auto velocity_from = (*b)   -> midi_data.velocity;
                        auto velocity_to   = (*itr) -> midi_data.velocity;
                        ofxFunctionalEasing([=](float progress){
                            (*b)-> midi_data.velocity = ofLerp(velocity_from, velocity_to, progress);
                        }, duration, 0.0);
                    }
                    ++itr;
                }
            }
        }
        
        // noteの数がターゲットより少ない場合
        if ( notes_.size() < target_notes.size()){
            auto itr = notes_.begin();
            for(auto b = target_notes.begin(); b != target_notes.end(); ++b){
                if (itr != notes_.end()){
                    auto angle_from  = (*itr) -> angle;
                    auto angle_to = (*b) -> angle;
                    if ( abs(angle_to - angle_from) > abs(angle_to - (angle_from+360))){
                        angle_from += 360;
                    }
                    
                    //angle
                    ofxFunctionalEasing([=](float progress){
                        (*itr) -> angle = ofLerp(angle_from, angle_to, progress);
                        auto tmp_angle = ofLerp(angle_from, angle_to, progress);
                        if (tmp_angle >= 360){
                            (*itr) ->angle = tmp_angle - 360;
                        }else{
                            (*itr) -> angle = tmp_angle;
                        }
                    }, duration, 0.0);
                    
                    if((*itr)->midi_data.pitch != (*b)->midi_data.pitch){
                        auto target_pitch    = (*b)->midi_data.pitch;
                        auto target_velocity = (*b)->midi_data.velocity;
                        auto velocity_from = (*itr) -> midi_data.velocity;
                        auto velocity_to   = 0;
                        ofxFunctionalEasing([=](float progress){
                            (*itr) ->midi_data.velocity = ofLerp(velocity_from, velocity_to, progress);
                        }, duration/2.0, 0.0,"", [=](const std::string){
                            auto velocity_from = 0;
                            auto velocity_to     = target_velocity;
                            (*itr) -> midi_data.pitch = target_pitch;
                            ofxFunctionalEasing([=](float progress){
                                (*itr) -> midi_data.velocity = ofLerp(velocity_from, velocity_to, progress);
                            }, duration/2., 0.0);
                        });
                    }else{
                        auto velocity_from = (*itr)   -> midi_data.velocity;
                        auto velocity_to   = (*b) -> midi_data.velocity;
                        ofxFunctionalEasing([=](float progress){
                            (*itr)-> midi_data.velocity = ofLerp(velocity_from, velocity_to, progress);
                        }, duration, 0.0);
                    }
                    ++itr;
                }else{
                    //tukuru
                    auto velocity_from =  0;
                    auto velocity_to   =  (*b)->midi_data.velocity;
                    notes_.push_back(std::make_shared<Note>((*b)->midi_data.pitch, velocity_from));
                    auto n = notes_.back();
                    n->angle = (*b) ->angle;
                    ofxFunctionalEasing([=](float progress){
                        n ->midi_data.velocity = ofLerp(velocity_from, velocity_to, progress);
                    }, duration, 0.0,"",[](const std::string){
                        
                    });
                    itr = notes_.end();
                }
            }
        }
    }
}


void AngleSequencer::setVolume(float volume){
    volume_ = volume;
}

void AngleSequencer::setAngle(float angle){
    angle_ = angle;
}

void AngleSequencer::rotate(float angle){
    
    for(const auto& n : notes_){
        auto tmp = n -> angle - offset_angle_;
        n -> angle = tmp + angle ;
        if ( n -> angle >= 360) n -> angle -= 360;
    }
    offset_angle_ = angle;
}

//void morph(AngleSequencer* target);



const int AngleSequencer::getAngle() const {
    return angle_;
}

const int AngleSequencer::getMaxNoteNum() const {
    return kMaxNoteNum;
}

const int AngleSequencer::getNoteNum() const {
    return notes_.size();
}
const float AngleSequencer::getVolume() const {
    return volume_;
}

std::list<std::shared_ptr<Note>> AngleSequencer::getNotes() const{
    return notes_;
}


