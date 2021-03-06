#pragma once
#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
//#include "cinder/params/Params.h"

// Settings
#include "SDASettings.h"
// Session
#include "SDASession.h"
// Animation
#include "SDAAnimation.h"
// Texture
#include "SDATexture.h"
// Log
#include "SDALog.h"
// Spout
#include "CiSpoutOut.h"

using namespace ci;
using namespace ci::app;
using namespace std;
using namespace SophiaDigitalArt;

int									pingTexIndex, pongTexIndex;

// animation
ci::Anim<float>						mPingScale, mPongScale;
float								mDuration;
bool								mReverse;
bool								mPingPong;
bool								mPingAnimInProgress;
bool								mPongAnimInProgress;
const float							zoomStart = 0.01f;
const float							zoomEnd = 40 * 1.5f;
const float							xStart = 0.0f;
const float							yStart = 0.0f;

struct Tex {
	vec2							mPosStart;
	vec2							mPosEnd;
	ci::gl::TextureRef				mTexture;
};
vector<Tex>							mTexs;

class PeoplizationApp : public App {

public:
	PeoplizationApp();
	void mouseMove(MouseEvent event) override;
	void mouseDown(MouseEvent event) override;
	void mouseDrag(MouseEvent event) override;
	void mouseUp(MouseEvent event) override;
	void keyDown(KeyEvent event) override;
	void keyUp(KeyEvent event) override;
	void fileDrop(FileDropEvent event) override;
	void update() override;
	void draw() override;
	void cleanup() override;
	void setUIVisibility(bool visible);
private:
	// Settings
	SDASettingsRef					mSDASettings;
	// Session
	SDASessionRef					mSDASession;
	// Log
	SDALogRef						mSDALog;
	// Animation
	SDAAnimationRef					mSDAAnimation;
	
	float							mScaleMax;

	double							currentTime;
	double							delta;

	bool							mouseGlobal;

	string							mError;
	
	bool							mIsShutDown;
	Anim<float>						mRenderWindowTimer;
	void							positionRenderWindow();
	bool							mFadeInDelay;
	SpoutOut 						mSpoutOut;

	// textures
	fs::path						mTexturesFilepath;
	
	//! read a textures json file 
	void							loadTextures(const ci::DataSourceRef &source);
	fs::path						mTexturesJson;
	void							textureFromJson(const ci::JsonTree &json);

	//! shaders
	gl::GlslProgRef					mGlslBlend;
	int								iBlendmode;
	float							iPos0x;
	float							iPos0y;
	float							iPos1x;
	float							iPos1y;
	float							iZoom0;
	float							iZoom1;

	void							drawContent();
	// ui
	//params::InterfaceGlRef			mParams;
};
