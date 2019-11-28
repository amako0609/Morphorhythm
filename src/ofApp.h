#pragma once

#include "ofMain.h"
#include "ofxGui.h"
#include "morphorhythm.hpp"
using namespace morphorhythm;

constexpr int receivePort = 7777;
constexpr int sendPort = 1212;


class ofApp : public ofBaseApp{
public:
    void setup();
    void update();
    void draw();
    void keyPressed(int key);
    void sendOscFinishMorph();
    
    std::shared_ptr<Score> makeScore(const Score& score);
    std::shared_ptr<Score> json2score(ofJson json0, ofJson json1, ofJson json2, ofJson json3, ofJson json4, ofJson json5);
    
private:
    int bpm = 120;
    bool is_playing = false;
    int selected_pitch = -1;
    int morphing_speed = 10;
    int toggle = 1;
    int flag = 0;
    int trycount = 20;
    int randomX = 19;
    int count = 60;
    
    ofxOscSender senderControl;
    
    float duration = 15.0;
    
    ofxOscSender senderMain;
    
    
    ofJson mLayer0input = ofLoadJson("Layer0/0");
    ofJson mLayer1input = ofLoadJson("Layer1/0");
    ofJson mLayer2input = ofLoadJson("Layer2/0");
    ofJson mLayer3input = ofLoadJson("Layer3/0");
    ofJson mLayer4input = ofLoadJson("Layer4/0");
    ofJson mLayer5input = ofLoadJson("Layer5/0");
};

//class ofSubApp : public ofBaseApp{
//    
//public:
//    void setup();
//    void update();
//    void draw();
//    
//private:
//    float duration = 10.0;
//    
//    ofxOscSender senderMain;
//};
