#pragma once

#include "ofMain.h"
#include "ofxOsc.h"

// Estrutura do Neurônio Bio-Inspirado
struct Neuron {
    int id;
    ofVec3f pos;
    vector<int> neighborIDs;
    ofColor color;
    float lastPulseTime; // Buffer para o rastro de luz da inferência
    float severity;
};

class ofApp : public ofBaseApp{

	public:
		void setup() override;
		void update() override;
		void draw() override;
		void exit() override;

		void keyPressed(int key) override;
		void keyReleased(int key) override;
		void mouseMoved(int x, int y ) override;
		void mouseDragged(int x, int y, int button) override;
		void mousePressed(int x, int y, int button) override;
		void mouseReleased(int x, int y, int button) override;
		void mouseScrolled(int x, int y, float scrollX, float scrollY) override;
		void mouseEntered(int x, int y) override;
		void mouseExited(int x, int y) override;
		void windowResized(int w, int h) override;
		void dragEvent(ofDragInfo dragInfo) override;
		void gotMessage(ofMessage msg) override;
		
		ofxOscReceiver receiver;
        vector<Neuron> neurons;
        ofEasyCam cam;
        
        // Estado do Hiperplano
        float rotationY;
        float brainGlow;
        ofColor currentColor;
        ofColor targetColor;

        // Visualização
        ofTrueTypeFont font;
};
