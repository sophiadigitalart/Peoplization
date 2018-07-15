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
	mPingScale = 0.01f;
	mPongScale = 0.01f;
	mDuration = 2.5f;
	pingTexIndex = 0;
	pongTexIndex = 1;
	mPingPong = false;
	currentTime = 2.0f;
	mScaleMax = 2.0f;
	delta = 0.01f;

	mTexturesJson = getAssetPath("") / mSDASettings->mAssetsPath / "texturesjpg.json";
	if (fs::exists(mTexturesJson)) {
		loadTextures(loadFile(mTexturesJson));
		mPingStart = vec2(0.5f);
		mPongStart = vec2(0.5f);
	}
	else {
		quit();
	}
	
	iBlendmode = 8;
	//mGlslBlend = gl::GlslProg::create(mDefaultVextexShaderString, mMixFragmentShaderString);
	mGlslBlend = gl::GlslProg::create(gl::GlslProg::Format().vertex(loadAsset("passthrough.vs"))
		.fragment(loadAsset("mixtextures.glsl")));
	

	mParams = params::InterfaceGl::create(getWindow(), "Parameters", toPixels(ivec2(200, 400)));
	mParams->addParam("mDuration", &mDuration).min(0.1f).max(20.5f).keyIncr("d").keyDecr("D").precision(2).step(0.2f);
	mParams->addParam("currentTime", &currentTime).min(0.1f).max(20.5f).keyIncr("q").keyDecr("Q").precision(2).step(0.2f);
	mParams->addParam("mScaleMax", &mScaleMax).min(0.1f).max(20.5f).keyIncr("m").keyDecr("M").precision(2).step(0.2f);
	mParams->addParam("mPingPong", &mPingPong);
	mParams->addParam("iBlendmode", &iBlendmode).min(0).max(26).keyIncr("b").keyDecr("B");
	mParams->addParam("delta", &delta);
	
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
	float jValue, jMin, jMax;
	if (json.hasChild("texture")) {
		JsonTree u(json.getChild("texture"));
		jName = (u.hasChild("name")) ? u.getValueForKey<string>("name") : "unknown";
		jCtrlIndex = (u.hasChild("index")) ? u.getValueForKey<int>("index") : 249;
		jValue = (u.hasChild("value")) ? u.getValueForKey<float>("value") : 0.01f;
		jMin = (u.hasChild("min")) ? u.getValueForKey<float>("min") : 0.0f;
		jMax = (u.hasChild("max")) ? u.getValueForKey<float>("max") : 1.0f;
		fs::path fullPath = getAssetPath("") / "sequence" / jName;
		if (fs::exists(fullPath)) {
			Tex mTex;
			mTex.mTexture = ci::gl::Texture::create(ci::loadImage(fullPath));
			mTex.mPosEnd = vec2(0.09f, 0.03f);
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
	if (!mSDASession->handleMouseMove(event)) {
		// let your application perform its mouseMove handling here
	}
}
void PeoplizationApp::mouseDown(MouseEvent event)
{
	iBlendmode++;
	if (iBlendmode > 26) iBlendmode = 0;
	if (!mSDASession->handleMouseDown(event)) {
		// let your application perform its mouseDown handling here
		if (event.isRightDown()) {
		}
	}
}
void PeoplizationApp::mouseDrag(MouseEvent event)
{
	if (!mSDASession->handleMouseDrag(event)) {
		// let your application perform its mouseDrag handling here
		//mTexs[texIndex]->setSpeed((float)event.getX() / (float)getWindowWidth() / 10.0f);
	}
}
void PeoplizationApp::mouseUp(MouseEvent event)
{
	if (!mSDASession->handleMouseUp(event)) {
		// let your application perform its mouseUp handling here
	}
}

void PeoplizationApp::keyDown(KeyEvent event)
{

	if (!mSDASession->handleKeyDown(event)) {
		switch (event.getCode()) {
		case KeyEvent::KEY_ESCAPE:
			// quit the application
			quit();
			break;
		case KeyEvent::KEY_q:
			//startAnimation();
			break;

		case KeyEvent::KEY_h:
			// mouse cursor and ui visibility
			mSDASettings->mCursorVisible = !mSDASettings->mCursorVisible;
			setUIVisibility(mSDASettings->mCursorVisible);
			break;
		}
	}
}
void PeoplizationApp::keyUp(KeyEvent event)
{
	if (!mSDASession->handleKeyUp(event)) {
	}
}
void nextPingTexture()
{
	CI_LOG_I("nextPingTexture");
	pingTexIndex += 2;
	mPingScale = 0.01f;
	mPingStart = vec2(0.5f);
}
void nextPongTexture()
{
	CI_LOG_I("nextPongTexture");
	pongTexIndex += 2;
	mPongScale = 0.01f;
	mPongStart = vec2(0.5f);
}

void PeoplizationApp::startAnimation()
{
	CI_LOG_I("ping startAnimation");
	timeline().apply(&mPingScale, mScaleMax, mDuration, EaseNone()).finishFn(nextPingTexture);
	timeline().appendTo(&mPingScale, 0.1f, mDuration, EaseNone()).delay(1.0f);
	timeline().apply(&mPingStart, mTexs[pingTexIndex].mPosEnd, mDuration, EaseNone());
	CI_LOG_I("pong startAnimation");
	timeline().apply(&mPongScale, mScaleMax, mDuration, EaseNone()).finishFn(nextPongTexture);
	timeline().apply(&mPongStart, mTexs[pongTexIndex].mPosEnd, mDuration, EaseNone());
	
}
void PeoplizationApp::update()
{
	mSDASession->setFloatUniformValueByIndex(mSDASettings->IFPS, getAverageFps());
	mSDASession->update();
	delta = getElapsedSeconds() - currentTime;
	if (delta > mDuration) { // 6.0f) {
		CI_LOG_I("ping");
		currentTime = getElapsedSeconds();
		mPingPong = !mPingPong;
		startAnimation();
	}
	if (delta > mDuration * 2.0f) {
		
	}

	

}
void PeoplizationApp::drawContent()
{
	// texture binding must be before ScopedGlslProg
	mTexs[pingTexIndex].mTexture->bind(0);
	mTexs[pongTexIndex].mTexture->bind(1);
	// gl::ScopedTextureBind colorTex(mGBuffer->getTexture2d(G_COLOR), 0);
	gl::ScopedGlslProg prog(mGlslBlend);
	gl::ScopedBlendPremult blend;

	mGlslBlend->uniform("iGlobalTime", (float)getElapsedSeconds());
	mGlslBlend->uniform("iResolution", vec3(mSDASettings->mRenderWidth, mSDASettings->mRenderHeight, 1.0));
	mGlslBlend->uniform("iZoom0", mPingScale);
	mGlslBlend->uniform("iZoom1", mPongScale);

	//mGlslBlend->uniform("iGlobalTime", mSDAAnimation->getFloatUniformValueByIndex(mSDASettings->ITIME));

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
	//gl::ScopedModelMatrix scpModel; 
	/*gl::ScopedGlslProg glslScope(mGlslBlend);
	gl::clear(ColorA(0, 0, 0, 0));
	gl::drawSolidRect(Rectf(0, 0, mSDASettings->mRenderWidth, mSDASettings->mRenderHeight));
	
	//gl::enableAlphaBlending();
	gl::translate(mPingStart().x * mSDASettings->mRenderWidth, mPingStart().y * mSDASettings->mRenderHeight);
	gl::scale(mPingScale(), mPingScale());*/
	if (pingTexIndex > mTexs.size() - 1) pingTexIndex = 0;
	/*gl::draw(mTexs[pingTexIndex].mTexture);

	gl::translate(mPongStart().x * mSDASettings->mRenderWidth, mPongStart().y * mSDASettings->mRenderHeight);
	gl::scale(mPongScale(), mPongScale());*/
	if (pongTexIndex > mTexs.size() - 1) pongTexIndex = 1;
	//gl::draw(mTexs[pongTexIndex].mTexture);
//gl::draw(mFbo->getColorTexture());
	
gl::setMatricesWindow(mSDASettings->mRenderWidth, mSDASettings->mRenderHeight, false);
drawContent();

	/*
	i = 0;
	for (auto tex : mTexs)
	{
		int x = 128 * i;
		gl::draw(tex, Rectf(0 + x, 0, 128 + x, 128));
		i++;
	} */

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
