#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    ofSetFrameRate(60);
    ofBackground(0);
    ofEnableSmoothing();
    ofSetCircleResolution(64);
    
    // Porta 5005 conforme configurado no seu sender.py
    receiver.setup(5005);
    
    rotationY = 0;
    brainGlow = 1.0;
    
    // Cores iniciais do sistema
    targetColor = ofColor(0, 200, 255);
    currentColor = targetColor;
    
    neurons.clear();
}

//--------------------------------------------------------------
void ofApp::update(){
   // 1. DINÂMICA DE ROTAÇÃO E VOLUME
    rotationY += 0.2f;
    ofVec3f mainScale(360, 320, 360); // Volume da Membrana Neural

    // 2. FÍSICA BIO-INSPIRADA (AUTO-ORGANIZAÇÃO)
    for(size_t i = 0; i < neurons.size(); i++){
        // Repulsão entre neurônios para evitar colapsos visuais
        for(size_t j = i + 1; j < neurons.size(); j++){
            ofVec3f dir = neurons[i].pos - neurons[j].pos;
            float dist = dir.length();
            if(dist < 55 && dist > 0){
                float f = (1.0 - dist/55.0) * 0.4f;
                neurons[i].pos += dir.getNormalized() * f;
                neurons[j].pos -= dir.getNormalized() * f;
            }
        }

        // Restrição Elipsoide (Mantém o cérebro coeso)
        float check = pow(neurons[i].pos.x/mainScale.x, 2) + 
                      pow(neurons[i].pos.y/mainScale.y, 2) + 
                      pow(neurons[i].pos.z/mainScale.z, 2);
        
        if(check > 1.0) {
            neurons[i].pos *= 0.97; // Puxão centrípeto
        }
    }

    // 3. RECEPTOR OSC (CONEXÃO COM A LITERATURA DE VETORES)
    while(receiver.hasWaitingMessages()){
        ofxOscMessage m; 
        receiver.getNextMessage(m);

        // Recebe Configuração e Posição (Setup/Pos)
        if(m.getAddress() == "/roi/setup" || m.getAddress() == "/node/pos"){
            int id = m.getArgAsInt(0);
            if(id >= (int)neurons.size()) neurons.resize(id + 1);
            
            // Posição alvo vinda do UMAP (Normalizada para o volume do OF)
            ofVec3f targetPos(m.getArgAsFloat(1) * 350.0f, 
                              m.getArgAsFloat(2) * 300.0f, 
                              m.getArgAsFloat(3) * 350.0f);
            
            // Interpolação Viscosa para movimento orgânico
            neurons[id].pos.x = ofLerp(neurons[id].pos.x, targetPos.x, 0.05);
            neurons[id].pos.y = ofLerp(neurons[id].pos.y, targetPos.y, 0.05);
            neurons[id].pos.z = ofLerp(neurons[id].pos.z, targetPos.z, 0.05);

            // Atribuição de Cor baseada na CAUSA RAIZ (Química/Elétrica)
            if(m.getNumArgs() >= 5){
                int cause = m.getArgAsInt(4);
                if(cause == 1) neurons[id].color = ofColor(255, 215, 0); // Química: Amarelo
                else if(cause == 2) neurons[id].color = ofColor(255, 50, 50); // Elétrica: Vermelho
                else neurons[id].color = ofColor(0, 200, 255); // Estável: Ciano
            } else {
                neurons[id].color = ofColor(0, 200, 255);
            }
        }
        
        // Recebe Conexões (Arestas do Plexus)
        if(m.getAddress() == "/node/edges"){
            int id = m.getArgAsInt(0);
            if(id < (int)neurons.size()){
                neurons[id].neighborIDs.clear();
                for(size_t k=1; k<(size_t)m.getNumArgs(); k++){
                    neurons[id].neighborIDs.push_back(m.getArgAsInt(k));
                }
            }
        }

        // Pulso e Erro
        if(m.getAddress() == "/brain/glow") brainGlow = ofLerp(brainGlow, m.getArgAsFloat(0), 0.1);
        if(m.getAddress() == "/brain/error"){
            targetColor = (m.getArgAsFloat(0) > 0.5) ? ofColor(255, 10, 150) : ofColor(0, 200, 255);
        }
        
        // Pulso de Inferência (O Fóton)
        if(m.getAddress() == "/inference/pulse"){
            int pId = m.getArgAsInt(0);
            if(pId < (int)neurons.size()) neurons[pId].lastPulseTime = ofGetElapsedTimef();
        }
    }
    
    currentColor.lerp(targetColor, 0.05);    
}

//--------------------------------------------------------------
void ofApp::draw(){
   ofEnableDepthTest();
    ofEnableBlendMode(OF_BLENDMODE_ADD);
    
    cam.begin();
    ofRotateYDeg(rotationY); 

    // 1. GRADE DE BASE (POINCARÉ)
    ofPushMatrix();
        ofTranslate(0, -350, 0);
        ofRotateXDeg(90);
        ofSetColor(currentColor, 60);
        ofDrawGridPlane(100, 20, false); 
    ofPopMatrix();

    // 2. ARESTAS PLEXUS (FIBRA ÓTICA)
    float localPulse = (sin(ofGetElapsedTimef() * 3.0) * 0.35) + 0.65; 
    float finalPulse = localPulse * (brainGlow * 0.5 + 0.5);

    for(size_t i = 0; i < neurons.size(); i++){
        for(int nId : neurons[i].neighborIDs){
            if(nId >= 0 && (size_t)nId < neurons.size()){
                float dist = glm::distance((glm::vec3)neurons[i].pos, (glm::vec3)neurons[nId].pos);
                float alpha = ofMap(dist, 0, 250, 180, 0, true);
                
                // Brilho de Inferência (Se o fóton estiver passando)
                float timeSincePulse = std::min(ofGetElapsedTimef() - neurons[i].lastPulseTime, 
                                                ofGetElapsedTimef() - neurons[nId].lastPulseTime);
                
                if(timeSincePulse < 0.6f){
                    ofSetColor(255, 255, 255, ofMap(timeSincePulse, 0, 0.6f, 255, 0));
                    ofSetLineWidth(2.5);
                } else {
                    ofSetColor(neurons[i].color, alpha * finalPulse); 
                    ofSetLineWidth(1.0);
                }
                ofDrawLine(neurons[i].pos, neurons[nId].pos);
            }
        }
    }

    // 3. NEURÔNIOS REATIVOS
    for(auto &n : neurons){
        // Núcleo com cor individual da Causa Raiz
        ofSetColor(n.color, 255);
        ofDrawSphere(n.pos, 1.8);
        
        // Aura Glow
        ofSetColor(n.color, 45 * brainGlow);
        ofDrawSphere(n.pos, 4.5 * brainGlow); 
    }

    // 4. TRONCO DE DADOS VERTICAL
    ofSetColor(currentColor, 40);
    for(int i=0; i<40; i++){
        float x = ofSignedNoise(i, ofGetElapsedTimef()*0.1) * 200;
        float z = ofSignedNoise(i+10, ofGetElapsedTimef()*0.1) * 200;
        ofDrawLine(x, -350, z, x, 350, z);
    }

    cam.end();
    ofDisableDepthTest();
    
    // HUD
    ofSetColor(255);
    ofDrawBitmapString("BIO-SINAPTIC LINK: ACTIVE", 30, 30);
    ofDrawBitmapString("VETOR X: AGUARDANDO INFERENCIA", 30, 50);
}

//--------------------------------------------------------------
void ofApp::exit(){

}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseScrolled(int x, int y, float scrollX, float scrollY){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
