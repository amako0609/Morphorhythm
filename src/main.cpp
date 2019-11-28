#include "ofMain.h"
#include "ofApp.h"

//========================================================================
int main( ){
    ofGLFWWindowSettings settings;
    //    settings.setSize(1280, 800);
    settings.setSize(1280, 800);
    settings.setPosition(glm::vec2{0,0});
    settings.decorated = false;
    auto main_window = ofCreateWindow(settings);
    auto main_app    = std::make_shared<ofApp>();
    
    //    settings.setSize(1600, 900);
//    //    settings.setPosition(glm::vec2{1280,0});
//    settings.setSize(1600, 900);
//    settings.setPosition(glm::vec2{1280,0});
//    auto sub_window  = ofCreateWindow(settings);
//    auto sub_app     = std::make_shared<ofSubApp>();
    
    ofRunApp(main_window, main_app);
//    ofRunApp(sub_window, sub_app);
    ofRunMainLoop();
}


