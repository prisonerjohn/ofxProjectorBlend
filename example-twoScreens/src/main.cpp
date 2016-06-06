#include "ofMain.h"
#include "ofApp.h"

//========================================================================
int main() {
	ofGLFWWindowSettings settings;
	// ofxProjectorBlend also works with programmable pipeline (Version 3.2+)!
	settings.setGLVersion(3, 2);
	settings.width = 1280;
	settings.height = 480;
	ofCreateWindow(settings);

	ofRunApp(new ofApp());
}
