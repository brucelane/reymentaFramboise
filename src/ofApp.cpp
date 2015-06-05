#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
	ofSetVerticalSync(true);
    // setup a server with default options on port 9092
    // - pass in true after port to set up with SSL
    //bSetup = server.setup( 9093 );

    ofxLibwebsockets::ServerOptions options = ofxLibwebsockets::defaultServerOptions();
    options.port = 9092;
	options.bUseSSL = false; // you'll have to manually accept this self-signed cert if 'true'!
    bSetup = server.setup( options );

    // this adds your app as a listener for the server
    server.addListener(this);

    ofBackground(0);
    ofSetFrameRate(60);
    ofSetWindowPosition(10, 100);
	// midi
	// print input ports to console
	midiIn.listPorts(); // via instance
	//ofxMidiIn::listPorts(); // via static as well
    if (midiIn.getNumPorts()) {
	    for (int i=0; i<midiIn.getNumPorts(); i++) {
            string pname = midiIn.getPortName(i);
            if (ofStringTimesInString(pname, "Launchpad")) {
                printf("Launchpad on %d\n", i);
                midiInLaunchpad.openPort(i);
                midiInLaunchpad.addListener(this);
            }
            if (ofStringTimesInString(pname, "nano")) {
                printf("nanoKontrol on %d\n", i);
                midiInNanoKontrol.openPort(i);
                midiInNanoKontrol.addListener(this);
            }
        }
	}
	// open port by number (you may need to change this)
	midiIn.openPort(3);

	//midiIn
	//midiIn.openVirtualPort("ofxMidiIn Input"); // open a virtual port

	// don't ignore sysex, timing, & active sense messages,
	// these are ignored by default
	midiIn.ignoreTypes(false, false, false);

	// add ofApp as a listener
	midiIn.addListener(this);

	// print received messages to the console
	midiIn.setVerbose(true);
}

//--------------------------------------------------------------
void ofApp::update(){
}

//--------------------------------------------------------------
void ofApp::draw(){
    if ( bSetup ){
        ofDrawBitmapString("WebSocket server setup at "+ofToString( server.getPort() ) + ( server.usingSSL() ? " with SSL" : " without SSL"), 20, 260);

        ofSetColor(150);
        ofDrawBitmapString("Click anywhere to open up client example", 20, 280);
    } else {
        ofDrawBitmapString("WebSocket setup failed :(", 20,260);
    }

    int x = 20;
    int y = 320;

    ofSetColor(0,150,0);
    ofDrawBitmapString("Console", x, 300);

    ofSetColor(255);
    for (int i = messages.size() -1; i >= 0; i-- ){
        ofDrawBitmapString( messages[i], x, y );
        y += 20;
    }
    if (messages.size() > NUM_MESSAGES) messages.erase( messages.begin() );

    ofSetColor(150,0,0);
    ofDrawBitmapString("Type a message, hit [RETURN] to send:", x, ofGetHeight()-60);
    ofSetColor(255);
    ofDrawBitmapString(toSend, x, ofGetHeight() - 40);
	// midi
	int name = 0;
	int value = 0;
	switch (midiMessage.status)
	{
	    case MIDI_CONTROL_CHANGE:
            name = midiMessage.control;
            value = midiMessage.value;
            break;
	    case MIDI_NOTE_ON:
            name = midiMessage.pitch;
            value = midiMessage.velocity;
            break;
	    case MIDI_NOTE_OFF:
            name = midiMessage.pitch;
            value = midiMessage.velocity;
            break;
	}
	text.str(""); // clear
	// lmap<float>(value, 0.0, 127.0, 0.0, 1.0) (from Cinder)
	float normalizedValue = value / 127;
	text << "{\"params\" :[{\"name\" : " << name << ",\"value\" : " << normalizedValue << "}]}";
	server.send( text.str() );
	text.str(""); // clear
	// draw the last recieved message contents to the screen
	text << "Received: " << ofxMidiMessage::getStatusString(midiMessage.status);
	ofDrawBitmapString(text.str(), 20, 20);
	text.str(""); // clear

	text << "channel: " << midiMessage.channel;
	ofDrawBitmapString(text.str(), 20, 34);
	text.str(""); // clear

	text << "pitch: " << midiMessage.pitch;
	ofDrawBitmapString(text.str(), 20, 48);
	text.str(""); // clear
	ofRect(20, 58, ofMap(midiMessage.pitch, 0, 127, 0, ofGetWidth()-40), 20);

	text << "velocity: " << midiMessage.velocity;
	ofDrawBitmapString(text.str(), 20, 96);
	text.str(""); // clear
	ofRect(20, 105, ofMap(midiMessage.velocity, 0, 127, 0, ofGetWidth()-40), 20);

	text << "control: " << midiMessage.control;
	ofDrawBitmapString(text.str(), 20, 144);
	text.str(""); // clear
	ofRect(20, 154, ofMap(midiMessage.control, 0, 127, 0, ofGetWidth()-40), 20);

	text << "value: " << midiMessage.value;
	ofDrawBitmapString(text.str(), 20, 192);
	text.str(""); // clear
	if(midiMessage.status == MIDI_PITCH_BEND) {
		ofRect(20, 202, ofMap(midiMessage.value, 0, MIDI_MAX_BEND, 0, ofGetWidth()-40), 20);
	}
	else {
		ofRect(20, 202, ofMap(midiMessage.value, 0, 127, 0, ofGetWidth()-40), 20);
	}

	text << "delta: " << midiMessage.deltatime;
	ofDrawBitmapString(text.str(), 20, 240);
	text.str(""); // clear
}

//--------------------------------------------------------------
void ofApp::onConnect( ofxLibwebsockets::Event& args ){
    cout<<"on connected"<<endl;
}

//--------------------------------------------------------------
void ofApp::onOpen( ofxLibwebsockets::Event& args ){
    cout<<"new connection open"<<endl;
    messages.push_back("New connection from " + args.conn.getClientIP() + ", " + args.conn.getClientName() );
}

//--------------------------------------------------------------
void ofApp::onClose( ofxLibwebsockets::Event& args ){
    cout<<"on close"<<endl;
    messages.push_back("Connection closed");
}

//--------------------------------------------------------------
void ofApp::onIdle( ofxLibwebsockets::Event& args ){
    cout<<"on idle"<<endl;
}

//--------------------------------------------------------------
void ofApp::onMessage( ofxLibwebsockets::Event& args ){
    cout<<"got message "<<args.message<<endl;

    // trace out string messages or JSON messages!
    if ( !args.json.isNull() ){
        messages.push_back("New message: " + args.json.toStyledString() + " from " + args.conn.getClientName() );
    } else {
        messages.push_back("New message: " + args.message + " from " + args.conn.getClientName() );
    }

    // echo server = send message right back!
    args.conn.send( args.message );
}

//--------------------------------------------------------------
void ofApp::onBroadcast( ofxLibwebsockets::Event& args ){
    cout<<"got broadcast "<<args.message<<endl;
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    // do some typing!
    if ( key != OF_KEY_RETURN ){
        if ( key == OF_KEY_BACKSPACE ){
            if ( toSend.length() > 0 ){
                toSend.erase(toSend.end()-1);
            }
        } else {
            toSend += key;
        }
    } else {
        // send to all clients
        server.send( toSend );
        messages.push_back("Sent: '" + toSend + "' to "+ ofToString(server.getConnections().size())+" websockets" );
        toSend = "";
    }
    if ( key == OF_KEY_ESC ){
            exit();
    }
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
    string url = "http";
    if ( server.usingSSL() ){
        url += "s";
    }
    url += "://localhost:" + ofToString( server.getPort() );
    ofLaunchBrowser(url);
}
void ofApp::newMidiMessage(ofxMidiMessage& msg) {

	// make a copy of the latest message
	midiMessage = msg;
}
void ofApp::exit() {

	// clean up
	midiIn.closePort();
	midiIn.removeListener(this);
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
