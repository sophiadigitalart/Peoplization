#include "PeoplizationApp.h"
// alternance g d intérieurs
// enlever tete carrées
// voir tulle avec seb + vp derriere
PeoplizationApp::PeoplizationApp()
	: mSpoutOut("Peoplization", app::getWindowSize())
{
	// Settings
	mSDASettings = SDASettings::create("Peoplization");
	// Session
	mSDASession = SDASession::create(mSDASettings);
	// Animation
	mSDAAnimation = SDAAnimation::create(mSDASettings);

	setUIVisibility(mSDASettings->mCursorVisible);
	mSDASession->getWindowsResolution();

	mouseGlobal = false;
	mFadeInDelay = true;
	// windows
	mIsShutDown = false;
#ifdef _DEBUG	
	mSDASettings->mRenderX = mSDASettings->mRenderY = 100;
	mSDASettings->mRenderWidth = 1280;
	mSDASettings->mRenderHeight = 720;
#else
#endif  // _DEBUG

	mRenderWindowTimer = 0.0f;
	timeline().apply(&mRenderWindowTimer, 1.0f, 2.0f).finishFn([&] { positionRenderWindow(); });

	// initialize 
	mDuration = 2.0f;
	pingTexIndex = 0;
	pongTexIndex = 1;
	mPingPong = mPingAnimInProgress = mReverse = false;
	mPongAnimInProgress = true;
	currentTime = 2.0f;
	mScaleMax = 1.5f;
	delta = 0.01f;
	mPingScale = mPongScale = iZoom0 = iZoom1 = zoomStart;
	iPos0x = iPos1x = xStart;
	iPos0y = iPos1y = yStart;

	mTexturesJson = getAssetPath("") / mSDASettings->mAssetsPath / "texturesjpg.json";
	if (fs::exists(mTexturesJson)) {
		loadTextures(loadFile(mTexturesJson));
	}
	else {
		quit();
	}

	iBlendmode = 8;
	mGlslBlend = gl::GlslProg::create(gl::GlslProg::Format().vertex(loadAsset("passthrough.vs")).fragment(loadAsset("mixtextures.glsl")));
}

void PeoplizationApp::loadTextures(const ci::DataSourceRef &source) {

	JsonTree json(source);
	// try to load the specified json file
	if (json.hasChild("textures")) {
		JsonTree u(json.getChild("textures"));
		// iterate textures
		for (size_t i = 0; i < u.getNumChildren(); i++) {
			JsonTree child(u.getChild(i));
			if (child.hasChild("texture")) {
				JsonTree w(child.getChild("texture"));
				textureFromJson(child);
			}
		}
	}
}
void PeoplizationApp::textureFromJson(const ci::JsonTree &json) {
	string jName;
	int jCtrlIndex;
	float jValue, jPosX, jPosY;
	if (json.hasChild("texture")) {
		JsonTree u(json.getChild("texture"));
		jName = (u.hasChild("name")) ? u.getValueForKey<string>("name") : "unknown";
		jCtrlIndex = (u.hasChild("index")) ? u.getValueForKey<int>("index") : 249;
		jValue = (u.hasChild("value")) ? u.getValueForKey<float>("value") : 0.01f;
		fs::path fullPath = getAssetPath("") / "seq" / jName;
		if (fs::exists(fullPath)) {
			Tex mTex;
			mTex.mTexture = ci::gl::Texture::create(ci::loadImage(fullPath));
			mTexs.push_back(mTex);
		}
	}
}


void PeoplizationApp::positionRenderWindow() {
	mSDASettings->mRenderPosXY = ivec2(mSDASettings->mRenderX, mSDASettings->mRenderY);
	setWindowPos(mSDASettings->mRenderX, mSDASettings->mRenderY);
	setWindowSize(mSDASettings->mRenderWidth, mSDASettings->mRenderHeight);
}
void PeoplizationApp::setUIVisibility(bool visible)
{
	if (visible)
	{
		showCursor();
	}
	else
	{
		hideCursor();
	}
}
void PeoplizationApp::fileDrop(FileDropEvent event)
{
	mSDASession->fileDrop(event);
}

void PeoplizationApp::cleanup()
{
	if (!mIsShutDown)
	{
		mIsShutDown = true;
		CI_LOG_V("shutdown");
		// save settings
		mSDASettings->save();
		mSDASession->save();
		quit();
	}
}
void PeoplizationApp::mouseMove(MouseEvent event)
{

}
void PeoplizationApp::mouseDown(MouseEvent event)
{
	if (event.isRightDown()) {
		iPos1x = (float)event.getX() / (float)mSDASettings->mRenderWidth;
		iPos1y = (float)event.getY() / (float)mSDASettings->mRenderHeight;
	}
	else {
		iPos0x = (float)event.getX() / (float)mSDASettings->mRenderWidth;
		iPos0y = (float)event.getY() / (float)mSDASettings->mRenderHeight;
	}
}
void PeoplizationApp::mouseDrag(MouseEvent event)
{
	if (!mSDASession->handleMouseDrag(event)) {
	}
}
void PeoplizationApp::mouseUp(MouseEvent event)
{

}


void PeoplizationApp::keyUp(KeyEvent event)
{
	if (!mSDASession->handleKeyUp(event)) {
	}
}

void PeoplizationApp::keyDown(KeyEvent event)
{
	switch (event.getCode()) {
	case KeyEvent::KEY_ESCAPE:
		// quit the application
		quit();
		break;
	case KeyEvent::KEY_f:
		pingTexIndex += 2;
		if (pingTexIndex > mTexs.size() - 1) pingTexIndex = 0;
		pongTexIndex += 2;
		if (pongTexIndex > mTexs.size() - 1) pongTexIndex = 1;
		break;

	case KeyEvent::KEY_h:
		// mouse cursor and ui visibility
		mSDASettings->mCursorVisible = !mSDASettings->mCursorVisible;
		setUIVisibility(mSDASettings->mCursorVisible);
		break;
	case KeyEvent::KEY_r:
		mReverse = !mReverse;
		break;
	}
}
void nextPingTexture()
{
	pingTexIndex += 2;
	mPingScale = mReverse ? zoomEnd : zoomStart;
	CI_LOG_I("nextPingTexture:"+toString(pingTexIndex));
	mPingAnimInProgress = false;
}
void nextPongTexture()
{
	pongTexIndex += 2;
	mPongScale = mReverse ? zoomEnd : zoomStart;
	CI_LOG_I("nextPongTexture:"+toString(pongTexIndex));
	mPongAnimInProgress = false;
}
void PeoplizationApp::update()
{
	mSDASession->setFloatUniformValueByIndex(mSDASettings->IFPS, getAverageFps());
	mSDASession->update();
	delta = getElapsedSeconds() - currentTime;
	if (!mPingAnimInProgress) {
		mPingAnimInProgress = true;
		CI_LOG_I("ping startAnimation");
		if (mReverse) {
			timeline().apply(&mPingScale, mScaleMax, mDuration * 1.1f, EaseNone()).finishFn(nextPongTexture);
			timeline().appendTo(&mPingScale, 0.0f , mDuration, EaseNone());
		}
		else {
			timeline().apply(&mPingScale, mScaleMax, mDuration * 1.1f, EaseNone()).finishFn(nextPongTexture);
			timeline().appendTo(&mPingScale, mScaleMax * 40.0f, mDuration, EaseNone());
		}
	}
	if (!mPongAnimInProgress) {
		mPongAnimInProgress = true;
		CI_LOG_I("pong startAnimation");
		if (mReverse) {
			timeline().apply(&mPongScale, mScaleMax, mDuration * 1.1f, EaseNone()).finishFn(nextPingTexture);
			timeline().appendTo(&mPongScale, 0.0f , mDuration, EaseNone());
		}
		else {
			timeline().apply(&mPongScale, mScaleMax, mDuration * 1.1f, EaseNone()).finishFn(nextPingTexture);
			timeline().appendTo(&mPongScale, mScaleMax * 40.0f, mDuration, EaseNone());
		}
	}
}
void PeoplizationApp::drawContent()
{
	// texture binding must be before ScopedGlslProg
	mTexs[pingTexIndex].mTexture->bind(0);
	mTexs[pongTexIndex].mTexture->bind(1);

	gl::ScopedGlslProg prog(mGlslBlend);
	gl::ScopedBlendPremult blend;

	//mGlslBlend->uniform("iGlobalTime", mSDAAnimation->getFloatUniformValueByIndex(mSDASettings->ITIME));
	mGlslBlend->uniform("iGlobalTime", (float)getElapsedSeconds());
	mGlslBlend->uniform("iResolution", vec3(mSDASettings->mRenderWidth, mSDASettings->mRenderHeight, 1.0));
	iZoom0 = mPingScale;
	iZoom1 = mPongScale;

	mGlslBlend->uniform("iZoom0", iZoom0);
	mGlslBlend->uniform("iZoom1", iZoom1);
	mGlslBlend->uniform("iPos0", vec2(0.5f)); 
	mGlslBlend->uniform("iPos1", vec2(0.5f));

	mGlslBlend->uniform("iMouse", vec3(mSDAAnimation->getFloatUniformValueByIndex(35), mSDAAnimation->getFloatUniformValueByIndex(36), mSDAAnimation->getFloatUniformValueByIndex(37)));
	mGlslBlend->uniform("iChannel0", 0); // texture 0
	mGlslBlend->uniform("iChannel1", 1); // texture 1
	mGlslBlend->uniform("iBlendmode", iBlendmode);

	gl::drawSolidRect(getWindowBounds());

}
void PeoplizationApp::draw()
{
	gl::clear(ColorA(0, 0, 0, 0));
	if (mFadeInDelay) {
		mSDASettings->iAlpha = 0.0f;
		if (getElapsedFrames() > mSDASession->getFadeInDelay()) {
			mFadeInDelay = false;
			timeline().apply(&mSDASettings->iAlpha, 0.0f, 1.0f, 1.5f, EaseInCubic());
		}
	}
	if (pingTexIndex > mTexs.size() - 1) pingTexIndex = 0;
	if (pongTexIndex > mTexs.size() - 1) pongTexIndex = 1;

	gl::setMatricesWindow(mSDASettings->mRenderWidth, mSDASettings->mRenderHeight, false);
	drawContent();

	// Spout Send
	mSpoutOut.sendViewport();
	getWindow()->setTitle(mSDASettings->sFps + " fps Peoplization");
}

void prepareSettings(App::Settings *settings)
{
	settings->setWindowSize(640, 480);
	
#ifdef _DEBUG
	settings->setConsoleWindowEnabled();
#else
#endif  // _DEBUG
}

CINDER_APP(PeoplizationApp, RendererGl, prepareSettings)
