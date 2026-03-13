#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
      ofSetFrameRate(60);
    ofBackground(0);
    ofEnableSmoothing();
    ofSetCircleResolution(64);
    
    // Porta 5005 para escuta dos dados do sender.py
    receiver.setup(5005);
    
    rotationY = 0;
    brainGlow = 1.0;
    inferenceError = 0.0;
    
    targetColor = ofColor(0, 200, 255);
    currentColor = targetColor;
    
    neurons.clear();      
}

//--------------------------------------------------------------
void ofApp::update(){
     // 1. DINÂMICA DE ROTAÇÃO
    rotationY += 0.22f;
    float raioMaximoDisco = 420.0f;

    // 2. FÍSICA DE CONFINAMENTO (PADDING E COMPACTAÇÃO)
    for(size_t i = 0; i < neurons.size(); i++){
        // Força de Atração ao Centro (Padding Gravitacional)
        // Impede que os pontos se dispersem como uma massa solta
        float distDoCentro = ofVec2f(neurons[i].pos.x, neurons[i].pos.z).length();
        if(distDoCentro > 1.0f){
            float forcaPuxao = ofMap(distDoCentro, 0, raioMaximoDisco, 0.005, 0.12, true);
            neurons[i].pos.x -= (neurons[i].pos.x / distDoCentro) * forcaPuxao;
            neurons[i].pos.z -= (neurons[i].pos.z / distDoCentro) * forcaPuxao;
        }
        
        // Achata o eixo Y constantemente para manter a forma de Disco/Lâmina
        neurons[i].pos.y *= 0.92f;

        // Repulsão Local (Evita colapso visual no centro do círculo)
        for(size_t j = i + 1; j < neurons.size(); j++){
            ofVec3f dir = neurons[i].pos - neurons[j].pos;
            dir.y = 0; // Física limitada ao plano XZ
            float d = dir.length();
            if(d < 38 && d > 0){
                float forcaRepulsao = (1.0 - d/38.0) * 0.55f;
                neurons[i].pos += dir.getNormalized() * forcaRepulsao;
                neurons[j].pos -= dir.getNormalized() * forcaRepulsao;
            }
        }
    }

    // 3. RECEPTOR OSC E MAPEAMENTO RADIAL
    while(receiver.hasWaitingMessages()){
        ofxOscMessage m; 
        receiver.getNextMessage(m);

        if(m.getAddress() == "/roi/setup" || m.getAddress() == "/node/pos"){
            int id = m.getArgAsInt(0);
            if(id >= (int)neurons.size()) neurons.resize(id + 1);
            
            // Projeta os dados lineares em um Layout de Disco (Mandala)
            float anguloRadial = m.getArgAsFloat(1) * TWO_PI;
            float raioInterno = m.getArgAsFloat(3) * raioMaximoDisco;
            
            ofVec3f targetPos(
                cos(anguloRadial) * raioInterno, 
                0, // Y plano para a estética de disco
                sin(anguloRadial) * raioInterno
            );
            
            // Interpolação suave para movimento orgânico
            neurons[id].pos.x = ofLerp(neurons[id].pos.x, targetPos.x, 0.07);
            neurons[id].pos.y = ofLerp(neurons[id].pos.y, targetPos.y, 0.07);
            neurons[id].pos.z = ofLerp(neurons[id].pos.z, targetPos.z, 0.07);

            // Atribuição de Cor baseada na CAUSA RAIZ (Funcionalidade)
            if(m.getNumArgs() >= 5){
                int cause = m.getArgAsInt(4);
                if(cause == 1) neurons[id].color = ofColor(255, 215, 0); // Química: Amarelo
                else if(cause == 2) neurons[id].color = ofColor(255, 50, 50); // Elétrica: Vermelho
                else neurons[id].color = ofColor(0, 200, 255); // Estável: Ciano
            } else {
                neurons[id].color = ofColor(0, 200, 255);
            }
        }
        
        if(m.getAddress() == "/node/edges"){
            int id = m.getArgAsInt(0);
            if(id < (int)neurons.size()){
                neurons[id].neighborIDs.clear();
                for(size_t k=1; k<(size_t)m.getNumArgs(); k++){
                    neurons[id].neighborIDs.push_back(m.getArgAsInt(k));
                }
            }
        }

        if(m.getAddress() == "/brain/glow") brainGlow = ofLerp(brainGlow, m.getArgAsFloat(0), 0.1);
        
        if(m.getAddress() == "/brain/error"){
            inferenceError = m.getArgAsFloat(0);
            targetColor = (inferenceError > 0.5) ? ofColor(255, 20, 140) : ofColor(0, 200, 255);
        }
        
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

    // --- 1. ESTÉTICA: ORLA DE POINCARÉ (Base do Hiperplano) ---
    ofPushMatrix();
        ofTranslate(0, -450, 0);
        ofRotateXDeg(90);
        ofNoFill();
        for(int i = 1; i <= 12; i++){
            // Decaimento exponencial para anéis concêntricos perfeitos
            float raioAneis = 450.0f * (1.0f - exp(-i * 0.35f)); 
            ofSetColor(currentColor, 110 - (i * 8)); 
            ofDrawCircle(0, 0, raioAneis);
        }
        // Cruz de Simetria do Grid
        ofSetColor(currentColor, 35);
        ofDrawLine(-450, 0, 450, 0); 
        ofDrawLine(0, -450, 0, 450);
    ofPopMatrix();

    // --- 2. ESTÉTICA: TRONCO DE DADOS (Fluxo Vertical) ---
    for(int i=0; i<30; i++){
        float noiseX = ofSignedNoise(i, ofGetElapsedTimef()*0.05) * 440;
        float noiseZ = ofSignedNoise(i+15, ofGetElapsedTimef()*0.05) * 440;
        ofSetColor(currentColor, 22 * brainGlow);
        ofDrawLine(noiseX, -450, noiseZ, noiseX, 450, noiseZ);
    }

    // --- 3. ESTÉTICA: ARESTAS (Braingraph Colorido com Pulso) ---
    for(size_t i = 0; i < neurons.size(); i++){
        for(int nId : neurons[i].neighborIDs){
            if(nId >= 0 && (size_t)nId < neurons.size()){
                
                float d = neurons[i].pos.distance(neurons[nId].pos);
                float alphaBase = ofMap(d, 0, 350, 170, 0, true);
                
                // Cálculo do Pulso de Inferência
                float timeSincePulse = ofGetElapsedTimef() - std::max(neurons[i].lastPulseTime, neurons[nId].lastPulseTime);
                
                if(timeSincePulse < 0.6f){
                    // Pulso Ativo: Brilho Branco Intenso
                    ofSetColor(255, 255, 255, ofMap(timeSincePulse, 0, 0.6f, 255, 0));
                    ofSetLineWidth(2.2f);
                    ofDrawLine(neurons[i].pos, neurons[nId].pos);
                } else {
                    // Repouso: Linha Sutil com a Cor da Causa Raiz (Sem cinza)
                    ofSetLineWidth(1.0f);
                    ofSetColor(neurons[i].color, alphaBase * 0.45f * brainGlow); 
                    ofDrawLine(neurons[i].pos, neurons[nId].pos);
                }
            }
        }
    }

    // --- 4. ESTÉTICA: NEURÔNIOS (Massa de Dados Compacta) ---
    for(size_t i = 0; i < neurons.size(); i++){
        if(neurons[i].neighborIDs.size() > 0){
            // Núcleo Branco de Precisão
            ofSetColor(255); 
            ofDrawSphere(neurons[i].pos, 1.7f);
            // Aura Colorida Reativa
            ofSetColor(neurons[i].color, 60 * brainGlow); 
            ofDrawSphere(neurons[i].pos, 4.5f * brainGlow);
        } else {
            // Neurônios Isolados (Padding Visual)
            ofSetColor(neurons[i].color, 30); 
            ofDrawSphere(neurons[i].pos, 0.6f);
        }
    }

    cam.end();
    ofDisableDepthTest();
    
    // --- 5. HUD E TELEMETRIA ---
    ofSetColor(255);
    float flutuaHUD = sin(ofGetElapsedTimef() * 2.0f) * 10.0f;
    ofDrawBitmapStringHighlight("SP HYPERPLANE DISK: ACTIVE", 30, 40 + flutuaHUD, ofColor(currentColor, 130));
    ofDrawBitmapString("INFERENCE LOSS: " + ofToString(inferenceError, 5), 30, ofGetHeight()-30);
    
   
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
