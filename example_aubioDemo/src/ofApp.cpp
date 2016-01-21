#include "ofApp.h"
#include "ofEventUtils.h"

//--------------------------------------------------------------
void ofApp::setup(){
    // set the size of the window
    ofSetWindowShape(750, 250);
    //ofSetFrameRate(30);

    int nOutputs = 2;
    int nInputs = 2;
    int sampleRate = 44100;
    int bufferSize = 512;
    int hopSize = 256;
    int nBuffers = 4;

    // setup onset object
    //onset.setup();
    onset.setup("default", bufferSize, hopSize, sampleRate);
    // listen to onset event
    ofAddListener(onset.gotOnset, this, &ofApp::onsetEvent);

    // setup pitch object
    pitch.setup();
    //pitch.setup("yinfft", 8 * bufferSize, bufferSize, sampleRate);

    // setup beat object
    //beat.setup();
    beat.setup("default", bufferSize, hopSize, sampleRate);
    // listen to beat event
    ofAddListener(beat.gotBeat, this, &ofApp::beatEvent);
    ofAddListener(beat.gotTatum, this, &ofApp::tatumEvent);

    // setup mel bands object
    //bands.setup();
    bands.setup("default", bufferSize, hopSize, sampleRate);

    // setup attackClass object
    //attackClass.setup();
    attackClass.setup("default", bufferSize, hopSize, sampleRate);
    attackClass.setBands(bands);

    attackClass.setOnset(onset);
    ofAddListener(attackClass.gotOnsetClass, this, &ofApp::onsetClassEvent);

    attackClass.setBeat(beat);
    ofAddListener(attackClass.gotBeatClass, this, &ofApp::beatClassEvent);

    filterDetect.setup("default", bufferSize, hopSize, sampleRate);
    filterDetect.setBands(bands);

    ofSoundStreamSetup(nOutputs, nInputs, this);
    //ofSoundStreamSetup(nOutputs, nInputs, sampleRate, bufferSize, nBuffers);
    //ofSoundStreamListDevices();

    // setup the gui objects
    int start = 0;
    beatGui.setup("ofxAubioBeat", "settings.xml", start + 10, 10);
    beatGui.add(bpm_tatumSignature.setup( "tatum signature", 1, 1, 64));
    beatGui.add(bpm.setup( "bpm", 0, 0, 250));

    start += 250;
    onsetGui.setup("ofxAubioOnset", "settings.xml", start + 10, 10);
    onsetGui.add(onsetThreshold.setup( "threshold", 0, 0, 2));
    onsetGui.add(onsetNovelty.setup( "onset novelty", 0, 0, 10000));
    onsetGui.add(onsetThresholdedNovelty.setup( "thr. novelty", 0, -1000, 1000));
    // set default value
    onsetThreshold = onset.threshold;

    start += 250;
    pitchGui.setup("ofxAubioPitch", "settings.xml", start + 10, 10);
    pitchGui.add(midiPitch.setup( "midi pitch", 0, 0, 128));
    pitchGui.add(pitchConfidence.setup( "confidence", 0, 0, 1));

    bandsGui.setup("ofxAubioMelBands", "settings.xml", start + 10, 115);
    for (int i = 0; i < 40; i++) {
        bandPlot.addVertex( 50 + i * 650 / 40., 240 - 100 * bands.energies[i]);
    }

    filterDetectGui.setup("ofxAubioFilterDetect", "settings.xml", start + 10, 215);
    filterDetectGui.add(lowCut.setup( "lowcut", 0, 0, bands.nBands));
    filterDetectGui.add(highCut.setup( "hicut", 0, 0, bands.nBands));
}

void ofApp::exit(){
    ofSoundStreamStop();
    ofSoundStreamClose();
}

void ofApp::audioIn(float * input, int bufferSize, int nChannels){
    // compute onset detection
    onset.audioIn(input, bufferSize, nChannels);
    // compute pitch detection
    pitch.audioIn(input, bufferSize, nChannels);
    // compute beat location
    beat.audioIn(input, bufferSize, nChannels);
    // compute bands
    bands.audioIn(input, bufferSize, nChannels);

    // compute onset class
    attackClass.audioIn(input, bufferSize, nChannels);
    // compute filter detection
    filterDetect.audioIn(input, bufferSize, nChannels);
}

void audioOut(){
}

//--------------------------------------------------------------
void ofApp::update(){
    onset.setThreshold(onsetThreshold);
    beat.setTatumSignature((unsigned)bpm_tatumSignature);

    lowCut = filterDetect.max_low_cutoff;
    highCut = filterDetect.min_high_cutoff;
}

//--------------------------------------------------------------
void ofApp::draw(){
    // update beat info
    if (gotBeat) {
        ofSetColor(ofColor::green);
        ofRect(90,150,50,50);
        gotBeat = false;
    }
    if (gotTatum) {
        ofSetColor(ofColor::limeGreen);
        ofRect(140,150,25,25);
        gotTatum = false;
    }

    // update onset info
    if (gotOnset) {
        ofSetColor(ofColor::red);
        ofRect(250 + 90,150,50,50);
        gotOnset = false;
    }
    onsetNovelty = onset.novelty;
    onsetThresholdedNovelty = onset.thresholdedNovelty;

    // update pitch info
    pitchConfidence = pitch.pitchConfidence;
    if (pitch.latestPitch) midiPitch = pitch.latestPitch;
    bpm = beat.bpm;

    // draw
    pitchGui.draw();
    beatGui.draw();
    onsetGui.draw();

    ofSetColor(ofColor::orange);
    ofSetLineWidth(3.);
    bandsGui.draw();
    //bandPlot.clear();
    for (int i = 0; i < bandPlot.size(); i++) {
        bandPlot[i].y = 240 - 100 * bands.energies[i];
    }
    bandPlot.draw();

    // onset class
    ofSetColor(200, 100, 100);
    ofRect(250 + 190 + currentOnsetClass * 7, 180, 10, 30);

    // beat class
    ofSetColor(100, 200, 100);
    ofRect(190 + currentBeatClass * 7, 170, 10, 30);

    // filter detect
    filterDetectGui.draw();
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
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){

}

//----
void ofApp::onsetEvent(float & time) {
    //ofLog() << "got onset at " << time << " s";
    gotOnset = true;
}

//----
void ofApp::beatEvent(float & time) {
    //ofLog() << "got beat at " << time << " s";
    gotBeat = true;
}

//----
void ofApp::tatumEvent(int & t) {
    //ofLog() << "got tatum at " << time << " samples";
    gotTatum = true;
}

//---
void ofApp::onsetClassEvent(int & t) {
    //ofLog() << "got onset class event at " << t << " ";
    currentOnsetClass = t;
}

//---
void ofApp::beatClassEvent(int & t) {
    ofLog() << "got beat class event of class " << t;
    currentBeatClass = t;
}
