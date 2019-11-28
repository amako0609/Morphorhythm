#include "ofApp.h"
#include "ofxPubSubOsc.h"


Score mainScore{};
std::deque<std::shared_ptr<Score>> relayScore{};

struct TimeManager {
    TimeManager():mInitTime{0.0f}, mTime(0.0f),isStart{false},mDuration{0.0f}{}
    float mInitTime;
    float mTime;
    float mDuration;
    void setDuration(float dur){
        mDuration = dur;
    }
    void start(){
        mInitTime = ofGetElapsedTimef();
        isStart = true;
    }
    void update(){
        if ( !isStart ) return;
        else{
            mTime = ofGetElapsedTimef() - mInitTime;
        }
        
        if ( mTime > mDuration+1.0){
            ofNotifyEvent(mFinishEvent);
            isStart = false;
        }
    }
    
    bool isStart;
    ofEvent<void> mFinishEvent;
};

TimeManager tManager{};

vector<PitchPicker> pickers;

using PitchNumber = int;
const std::vector<PitchNumber> kPitchList{
    36, 37, 38, 39, 40, 41
};

const std::vector<ofColor> kColorList{
    ofColor{48, 139, 195}, //blue
    ofColor{133, 168, 45}, //green
    ofColor{216, 63, 111}, //red
    ofColor{243, 153, 36}, //orange
    ofColor{217, 187, 66}, //yellow
    ofColor{116, 69, 144} //purple
};

const std::unordered_map<PitchNumber, ofColor> kPitchColorMap {
    {36, ofColor{48, 139, 195}}, //kick blue
    {37, ofColor{133, 168, 45}}, //rim green
    {38, ofColor{216, 63, 111}}, //snare red
    {39, ofColor{243, 153, 36}}, //clap orange
    {40, ofColor{217, 187, 66}}, //hihat closed
    {41, ofColor{116, 69, 144}} //hihat open
};

void ofApp::sendOscFinishMorph(){
    ofLog() << "FINISH";
    ofxOscMessage m;
    m.setAddress("/finish");
    m.addBoolArg(true);
    senderControl.sendMessage(m);
}

//--------------------------------------------------------------
void ofApp::setup(){
    ofBackground(6, 3, 3);
    ofHideCursor();
    ofSetFrameRate(60);
    ofSetCircleResolution(60);
    ofSetRectMode(OF_RECTMODE_CORNER);
    ofAddListener(tManager.mFinishEvent, this, &ofApp::sendOscFinishMorph);
    
    senderControl.setup("127.0.0.1", sendPort);
    ofxPublishOsc("127.0.0.1", sendPort, "/is_playing",is_playing);
    ofxPublishOsc("127.0.0.1", sendPort, "/bpm", bpm);
    ofxSubscribeOsc(receivePort, "/play", [&](ofxOscMessage& m){is_playing = m.getArgAsBool(0);});
    sendOscFinishMorph();
    
    auto itr = kPitchColorMap.begin();
    for(auto i = 0; i < kPitchList.size(); ++i){
        PitchPicker p {glm::vec2{i*180.0 + 105.0, 600}, 130.0, 130.0};
        pickers.push_back(p);
        ofColor c = kColorList[i];
        pickers[i].setColor(c);
        ++itr;
    }
    
    
    //morphing
    ofxSubscribeOsc(receivePort, "/json2assign", [&](ofxOscMessage& m){
        int x0 = count;
        int x1 = count;
        int x2 = count;
        int x3 = count;
        int x4 = count;
        int x5 = count;
        count++;
        mLayer0input = ofLoadJson("Layer0/"+std::to_string(x0));
        mLayer1input = ofLoadJson("Layer1/"+std::to_string(x1));
        mLayer2input = ofLoadJson("Layer2/"+std::to_string(x2));
        mLayer3input = ofLoadJson("Layer3/"+std::to_string(x3));
        mLayer4input = ofLoadJson("Layer4/"+std::to_string(x4));
        mLayer5input = ofLoadJson("Layer5/"+std::to_string(x5));
        cout << x0 << x1 << x2 << x3 << x4 << x5;
        ofFile jsonFile("Layer0/"+std::to_string(x0));
        if(!jsonFile.exists()){
            int x0 = ofRandom(60);
            int x1 = ofRandom(60);
            int x2 = ofRandom(60);
            int x3 = ofRandom(60);
            int x4 = ofRandom(60);
            int x5 = ofRandom(60);
            mLayer0input = ofLoadJson("Layer0/"+std::to_string(x0));
            mLayer1input = ofLoadJson("Layer1/"+std::to_string(x1));
            mLayer2input = ofLoadJson("Layer2/"+std::to_string(x2));
            mLayer3input = ofLoadJson("Layer3/"+std::to_string(x3));
            mLayer4input = ofLoadJson("Layer4/"+std::to_string(x4));
            mLayer5input = ofLoadJson("Layer5/"+std::to_string(x5));
        }
        
        auto c = json2score(mLayer0input, mLayer1input, mLayer2input, mLayer3input, mLayer4input, mLayer5input);
        relayScore.push_back(c);
        if (relayScore.size() > 2 ){
            relayScore.pop_front();
        }
    });
    
    
    
    senderMain.setup("127.0.0.1", sendPort);
    ofxSubscribeOsc(receivePort, "/seq",[&](ofxOscMessage& m){
        for(auto i = 0; i < mainScore.getSequencerNum(); ++i){
            mainScore.getSequencer(i) -> updateAngle();
        }
        for(auto i = 0; i < 6; ++i){
            for(const auto& n : mainScore.getSequencer(i)->getNotes()){
                if ( mainScore.getSequencer(i) ->getAngle() == static_cast<int>(n -> angle) && n -> midi_data.pitch != -1){
                    ofxOscMessage m2;
                    m2.setAddress("/mainNote");
                    m2.addFloatArg(n -> midi_data.pitch);
                    m2.addFloatArg(n -> midi_data.velocity*mainScore.getSequencer(i)->getVolume());
                    senderMain.sendMessage(m2);
                }
            }
        }
    });
    
    ofxSubscribeOsc(receivePort, "/duration", [&](ofxOscMessage& m){duration = m.getArgAsInt(0);});
    ofxSubscribeOsc(receivePort, "/morph0", [&](ofxOscMessage& m){
        if(relayScore.size() < 1){return;}else{mainScore.morph(*relayScore[0], duration); tManager.setDuration(duration); tManager.start();}
    });
    ofxSubscribeOsc(receivePort, "/morph1", [&](ofxOscMessage& m){
        if(relayScore.size() < 2){return;}else{mainScore.morph(*relayScore[1], duration); tManager.setDuration(duration); tManager.start();}
    });
    
    //mainScore settings
    mainScore.translate(glm::vec2{ 1240/2, 780/2}); //1600/2
    mainScore.setRadius(250);
    mainScore.setPitchColorMap(kPitchColorMap);
}

//--------------------------------------------------------------
void ofApp::update(){
    tManager.update();
}

//--------------------------------------------------------------
void ofApp::draw(){
    ofPushMatrix();
    ofTranslate(-205-412, 390);
    for(const auto& c : relayScore){
        ofTranslate(825, 0);
        if ( c == nullptr) return;
        c.get()->setRadius(100);
        c.get()->setPitchColorMap(kPitchColorMap);
        c -> draw();
    }
    ofPopMatrix();
    
    ofPushMatrix();
    ofPushStyle();
    ofSetColor(201, 201, 201);
    ofTranslate(mainScore.getTranslation());
    ofDrawLine(0, 0,
               mainScore.getRadius() * cos(ofDegToRad(mainScore.getSequencer(0) -> getAngle())),
               mainScore.getRadius() * sin(ofDegToRad(mainScore.getSequencer(0) -> getAngle())));
    ofPopStyle();
    ofPopMatrix();
    mainScore.draw();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    if (key == ' '){
        int x = ofRandom(randomX);
        mLayer0input = ofLoadJson("Layer0/"+std::to_string(x));
        mLayer1input = ofLoadJson("Layer1/"+std::to_string(x));
        mLayer2input = ofLoadJson("Layer2/"+std::to_string(x));
        mLayer3input = ofLoadJson("Layer3/"+std::to_string(x));
        mLayer4input = ofLoadJson("Layer4/"+std::to_string(x));
        mLayer5input = ofLoadJson("Layer5/"+std::to_string(x));
        cout << x;
        auto c = json2score(mLayer0input, mLayer1input, mLayer2input, mLayer3input, mLayer4input, mLayer5input);
        relayScore.push_back(c);
        if (relayScore.size() > 2 ){
            relayScore.pop_front();
        }
    }
    if (key ==  'a'){
        if(relayScore.size() < 1){return;}else{mainScore.morph(*relayScore[0], duration); tManager.setDuration(duration); tManager.start();}
    }
}


//--------------------------------------------------------------
std::shared_ptr<Score> ofApp::json2score(ofJson json0, ofJson json1, ofJson json2, ofJson json3, ofJson json4, ofJson json5){
    auto tmp = std::make_shared<Score>();
    
    std::list<std::shared_ptr<Note>> jsonNotes0;
    for(auto i = 0; i < json0["Layer0"].size(); ++i){
        auto json_note0 = std::make_shared<Note>();
        json_note0->midi_data.pitch = json0["Layer0"][i][1];
        json_note0->midi_data.velocity = json0["Layer0"][i][2];
        json_note0->angle = json0["Layer0"][i][0];
        jsonNotes0.emplace_back(json_note0);
    }
    tmp->getSequencer(0)->setNotes(jsonNotes0);
    
    std::list<std::shared_ptr<Note>> jsonNotes1;
    for(auto i = 0; i < json1["Layer1"].size(); ++i){
        auto json_note1 = std::make_shared<Note>();
        json_note1->midi_data.pitch = json1["Layer1"][i][1];
        json_note1->midi_data.velocity = json1["Layer1"][i][2];
        json_note1->angle = json1["Layer1"][i][0];
        jsonNotes1.push_back(json_note1);
    }
    tmp->getSequencer(1)->setNotes(jsonNotes1);
    
    std::list<std::shared_ptr<Note>> jsonNotes2;
    for(auto i = 0; i < json2["Layer2"].size(); ++i){
        auto json_note2 = std::make_shared<Note>();
        json_note2->midi_data.pitch = json2["Layer2"][i][1];
        json_note2->midi_data.velocity = json2["Layer2"][i][2];
        json_note2->angle = json2["Layer2"][i][0];
        jsonNotes2.push_back(json_note2);
    }
    tmp->getSequencer(2)->setNotes(jsonNotes2);
    
    std::list<std::shared_ptr<Note>> jsonNotes3;
    for(auto i = 0; i < json3["Layer3"].size(); ++i){
        auto json_note3 = std::make_shared<Note>();
        json_note3->midi_data.pitch = json3["Layer3"][i][1];
        json_note3->midi_data.velocity = json3["Layer3"][i][2];
        json_note3->angle = json3["Layer3"][i][0];
        jsonNotes3.push_back(json_note3);
    }
    tmp->getSequencer(3)->setNotes(jsonNotes3);
    
    std::list<std::shared_ptr<Note>> jsonNotes4;
    for(auto i = 0; i < json4["Layer4"].size(); ++i){
        auto json_note4 = std::make_shared<Note>();
        json_note4->midi_data.pitch = json4["Layer4"][i][1];
        json_note4->midi_data.velocity = json4["Layer4"][i][2];
        json_note4->angle = json4["Layer4"][i][0];
        jsonNotes4.push_back(json_note4);
    }
    tmp->getSequencer(4)->setNotes(jsonNotes4);
    
    std::list<std::shared_ptr<Note>> jsonNotes5;
    for(auto i = 0; i < json5["Layer5"].size(); ++i){
        auto json_note5 = std::make_shared<Note>();
        json_note5->midi_data.pitch = json5["Layer5"][i][1];
        json_note5->midi_data.velocity = json5["Layer5"][i][2];
        json_note5->angle = json5["Layer5"][i][0];
        jsonNotes5.push_back(json_note5);
    }
    tmp->getSequencer(5)->setNotes(jsonNotes5);
    
    return tmp;
}











//
////--------------------------------------------------------------
//void ofSubApp::setup(){
//    ofSetBackgroundColor(6, 3, 3);
//    ofSetFrameRate(60);
//    ofSetCircleResolution(60);
//    
//    senderMain.setup("127.0.0.1", sendPort);
//    ofxSubscribeOsc(receivePort, "/seq",[&](ofxOscMessage& m){
//        for(auto i = 0; i < mainScore.getSequencerNum(); ++i){
//            mainScore.getSequencer(i) -> updateAngle();
//        }
//        for(auto i = 0; i < 6; ++i){
//            for(const auto& n : mainScore.getSequencer(i)->getNotes()){
//                if ( mainScore.getSequencer(i) ->getAngle() == static_cast<int>(n -> angle) && n -> midi_data.pitch != -1){
//                    ofxOscMessage m2;
//                    m2.setAddress("/mainNote");
//                    m2.addFloatArg(n -> midi_data.pitch);
//                    m2.addFloatArg(n -> midi_data.velocity*mainScore.getSequencer(i)->getVolume());
//                    senderMain.sendMessage(m2);
//                }
//            }
//        }
//    });
//    
//    ofxSubscribeOsc(receivePort, "/duration", [&](ofxOscMessage& m){duration = m.getArgAsInt(0);});
//    ofxSubscribeOsc(receivePort, "/morph0", [&](ofxOscMessage& m){
//        if(relayScore.size() < 1){return;}else{mainScore.morph(*relayScore[0], duration); tManager.setDuration(duration); tManager.start();}
//    });
//    ofxSubscribeOsc(receivePort, "/morph1", [&](ofxOscMessage& m){
//        if(relayScore.size() < 2){return;}else{mainScore.morph(*relayScore[1], duration); tManager.setDuration(duration); tManager.start();}
//    });
//    ofxSubscribeOsc(receivePort, "/morph2", [&](ofxOscMessage& m){
//        if(relayScore.size() < 3){return;}else{mainScore.morph(*relayScore[2], duration); tManager.setDuration(duration); tManager.start();}
//    });
//    ofxSubscribeOsc(receivePort, "/morph3", [&](ofxOscMessage& m){
//        if(relayScore.size() < 4){return;}else{mainScore.morph(*relayScore[3], duration); tManager.setDuration(duration); tManager.start();}
//    });
//    ofxSubscribeOsc(receivePort, "/layer0volume", [&](ofxOscMessage& m){
//        for(const auto& n : mainScore.getSequencer(0)->getNotes()){if(n->midi_data.pitch != -1){n->midi_data.velocity = m.getArgAsInt(0);}}
//    });
//    ofxSubscribeOsc(receivePort, "/layer1volume", [&](ofxOscMessage& m){
//        for(const auto& n : mainScore.getSequencer(1)->getNotes()){if(n->midi_data.pitch != -1){n->midi_data.velocity = m.getArgAsInt(0);}}
//    });
//    ofxSubscribeOsc(receivePort, "/layer2volume", [&](ofxOscMessage& m){
//        for(const auto& n : mainScore.getSequencer(2)->getNotes()){if(n->midi_data.pitch != -1){n->midi_data.velocity = m.getArgAsInt(0);}}
//    });
//    ofxSubscribeOsc(receivePort, "/layer3volume", [&](ofxOscMessage& m){
//        for(const auto& n : mainScore.getSequencer(3)->getNotes()){if(n->midi_data.pitch != -1){n->midi_data.velocity = m.getArgAsInt(0);}}
//    });
//    
//    //mainScore settings
//    mainScore.translate(glm::vec2{ 1240/2, 780/2}); //1600/2
//    mainScore.setRadius(300);
//    mainScore.setPitchColorMap(kPitchColorMap);
//}
//
//void ofSubApp::update(){
//    tManager.update();
//}
//
//void ofSubApp::draw(){
//    
//    
//}
