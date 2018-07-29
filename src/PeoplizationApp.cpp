#include "PeoplizationApp.h"

PeoplizationApp::PeoplizationApp()
	: mSpoutOut("SDA", app::getWindowSize())
{
	// Settings
	mSDASettings = SDASettings::create();
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
	mRenderWindowTimer = 0.0f;
	timeline().apply(&mRenderWindowTimer, 1.0f, 2.0f).finishFn([&] { positionRenderWindow(); });

	// initialize 
	mDuration = 5.5f;
	mPosDuration = 0.10f;
	pingTexIndex = 0;
	pongTexIndex = 1;
	mPingPong = mPingAnimInProgress = mPongAnimInProgress = false;
	currentTime = 2.0f;
	mScaleMax = 1.0f;
	delta = 0.01f;
	mPingScale = mPongScale = iZoom0 = iZoom1 = zoomStart;
	iPos0x = iPos1x = xStart;
	iPos0y = iPos1y = yStart;

	mTexturesJson = getAssetPath("") / mSDASettings->mAssetsPath / "texturesjpg.json";
	if (fs::exists(mTexturesJson)) {
		loadTextures(loadFile(mTexturesJson));
		mPingStart = vec2(iPos0x, iPos0y);
		mPongStart = vec2(iPos1x, iPos1y);
	}
	else {
		quit();
	}

	iBlendmode = 8;
	mGlslBlend = gl::GlslProg::create(gl::GlslProg::Format().vertex(loadAsset("passthrough.vs")).fragment(loadAsset("mixtextures.glsl")));

	mParams = params::InterfaceGl::create(getWindow(), "Parameters", toPixels(ivec2(200, 400)));
	mParams->addParam("mDuration", &mDuration).min(0.1f).max(20.5f).keyIncr("d").keyDecr("D").precision(2).step(0.2f);
	mParams->addParam("currentTime", &currentTime);
	mParams->addParam("mScaleMax", &mScaleMax).min(0.1f).max(20.5f).keyIncr("m").keyDecr("M").precision(2).step(0.2f);
	mParams->addParam("mPingPong", &mPingPong);
	mParams->addParam("iBlendmode", &iBlendmode).min(0).max(26).keyIncr("b").keyDecr("B");
	//mParams->addParam("delta", &delta);
	mParams->addParam("iPos0x", &iPos0x).keyIncr("x").keyDecr("X").precision(2).step(0.1f);
	mParams->addParam("iPos0y", &iPos0y).keyIncr("y").keyDecr("Y").precision(2).step(0.1f);
	mParams->addParam("iPos1x", &iPos1x).keyIncr("c").keyDecr("C").precision(2).step(0.1f);
	mParams->addParam("iPos1y", &iPos1y).keyIncr("u").keyDecr("U").precision(2).step(0.1f);
	mParams->addParam("iZoom0", &iZoom0).keyIncr("z").keyDecr("Z").precision(2).step(0.1f);
	mParams->addParam("iZoom1", &iZoom1).keyIncr("e").keyDecr("E").precision(2).step(0.1f);

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
		jPosX = (u.hasChild("posx")) ? u.getValueForKey<float>("posx") : 0.56f;
		jPosY = (u.hasChild("posy")) ? u.getValueForKey<float>("posy") : 0.73f;
		fs::path fullPath = getAssetPath("") / "sequence" / jName;
		if (fs::exists(fullPath)) {
			Tex mTex;
			mTex.mTexture = ci::gl::Texture::create(ci::loadImage(fullPath));
			mTex.mPosStart = vec2(jPosX, jPosY);
			mTex.mPosEnd = vec2(jPosX, jPosY);
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
		mPingStart().x = iPos0x;
		mPingStart().y = iPos0y;
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

void PeoplizationApp::startAnimation()
{
	/*if (mPingPong) {
		CI_LOG_I("ping startAnimation");
		timeline().apply(&mPingScale, mScaleMax, mDuration, EaseNone()).finishFn(nextPingTexture);
		//timeline().appendTo(&mPingScale, 0.1f, mDuration, EaseNone()).delay(1.0f);
		timeline().apply(&mPingStart, mTexs[pingTexIndex].mPosEnd, mDuration, EaseNone());
	}
	else {
		CI_LOG_I("pong startAnimation");
		timeline().apply(&mPongScale, mScaleMax, mDuration, EaseNone()).finishFn(nextPongTexture);
		timeline().apply(&mPongStart, mTexs[pongTexIndex].mPosEnd, mDuration, EaseNone());

	}
	*/
	pingTexIndex += 1;
	mPingScale = zoomStart;
	mPingStart = vec2(xStart, yStart);
	mPingAnimInProgress = false;
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
	case KeyEvent::KEY_n:
		startAnimation();
		break;
	}
}
void nextPingTexture()
{
	CI_LOG_I("nextPingTexture");
	pingTexIndex += 2;
	mPingScale = zoomStart;
	//mPingStart = vec2(xStart, yStart);
	mPingStart = mTexs[pingTexIndex].mPosStart;

	mPingAnimInProgress = false;
}
void nextPongTexture()
{
	CI_LOG_I("nextPongTexture");
	pongTexIndex += 2;
	mPongScale = zoomStart;
	mPongStart = vec2(xStart, yStart);
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
		timeline().apply(&mPingScale, mScaleMax, mDuration, EaseNone());
		//timeline().appendTo(&mPingScale, mScaleMax * 2.0f, mDuration, EaseNone());
		//timeline().apply(&mPingStart, mTexs[pingTexIndex].mPosEnd, mPosDuration, EaseNone());// .finishFn(nextPongTexture);
	}
	/*if (delta > mDuration) {
		CI_LOG_I("ping");
		currentTime = getElapsedSeconds();

	} */
	//if (delta > mDuration) {
		//currentTime = getElapsedSeconds();
		//mPingPong = false;
				//if (!mPongAnimInProgress) {
		//	mPongAnimInProgress = true;
		//	CI_LOG_I("pong startAnimation");
		//	timeline().apply(&mPongScale, mScaleMax, mDuration, EaseNone()).finishFn(nextPongTexture);
		//	//timeline().appendTo(&mPongScale, mScaleMax * 4.0f, mDuration, EaseNone());// .delay(1.0f);
		//	timeline().apply(&mPongStart, mTexs[pongTexIndex].mPosEnd, mPosDuration, EaseNone());
		//	
		//}



	//}
	//else {
		/*mPingPong = true;
		if (!mPingAnimInProgress) {
			mPingAnimInProgress = true;
			CI_LOG_I("ping startAnimation");
			timeline().apply(&mPingScale, mScaleMax, mDuration, EaseNone()).finishFn(nextPongTexture);
			timeline().appendTo(&mPingScale, mScaleMax * 4.0f, mDuration, EaseNone());// .delay(1.0f);
			timeline().apply(&mPingStart, mTexs[pingTexIndex].mPosEnd, mPosDuration, EaseNone());
		} */
		//}

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
	
	iPos0x = mPingStart().x;
	iPos0y = mPingStart().y;
	iPos1x = mPongStart().x;
	iPos1y = mPongStart().y; 

	mGlslBlend->uniform("iZoom0", iZoom0);
	mGlslBlend->uniform("iZoom1", iZoom1);
	mGlslBlend->uniform("iPos0", mPingStart()); //vec2(iPos0x, iPos0y));
	mGlslBlend->uniform("iPos1", vec2(iPos1x, iPos1y)); //mPongStart());

	mGlslBlend->uniform("iMouse", vec3(mSDAAnimation->getFloatUniformValueByIndex(35), mSDAAnimation->getFloatUniformValueByIndex(36), mSDAAnimation->getFloatUniformValueByIndex(37)));
	mGlslBlend->uniform("iChannel0", 0); // texture 0
	mGlslBlend->uniform("iChannel1", 1); // texture 1
	mGlslBlend->uniform("iBlendmode", iBlendmode); // texture 0

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
	// Draw the interface
	mParams->draw();
	getWindow()->setTitle(mSDASettings->sFps + " fps SDA");
}

void prepareSettings(App::Settings *settings)
{
	settings->setWindowSize(640, 480);
}

CINDER_APP(PeoplizationApp, RendererGl, prepareSettings)
