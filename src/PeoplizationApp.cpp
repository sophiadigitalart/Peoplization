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
	mScale = 0.01f;
	mDuration = 1.5f;
	texIndex = 0;
	
	mTexturesJson = getAssetPath("") / mSDASettings->mAssetsPath / "textures.json";
	if (fs::exists(mTexturesJson)) {
		loadTextures(loadFile(mTexturesJson));
		mPingStart = vec2(0.5f);
		mPingEnd = vec2(0.7f);
		mPongStart = vec2(0.5f);
		mPongEnd = vec2(0.3f);
	}
	else {
		quit();
	}
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
			mTexs.push_back(mTex);
		}
	}
}
void PeoplizationApp::startAnimation()
{
	timeline().apply(&mScale, 1.0f, mDuration, EaseInOutQuad())
		.finishFn(incrementTextureIndex);
	timeline().apply(&mPingStart, 1.0f, mDuration, EaseInOutQuad())
		.finishFn(incrementTextureIndex);
	timeline().appendTo(&mScale, 0.1f, mDuration, EaseInOutQuad()).delay(1.0f);
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
void PeoplizationApp::update()
{
	mSDASession->setFloatUniformValueByIndex(mSDASettings->IFPS, getAverageFps());
	mSDASession->update();
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
			startAnimation();
			break;

		case KeyEvent::KEY_h:
			// mouse cursor and ui visibility
			mSDASettings->mCursorVisible = !mSDASettings->mCursorVisible;
			setUIVisibility(mSDASettings->mCursorVisible);
			break;
		}
	}
}
void incrementTextureIndex()
{
	texIndex++;
	
}
void PeoplizationApp::keyUp(KeyEvent event)
{
	if (!mSDASession->handleKeyUp(event)) {
	}
}
void PeoplizationApp::draw()
{
	gl::clear(Color::black());
	if (mFadeInDelay) {
		mSDASettings->iAlpha = 0.0f;
		if (getElapsedFrames() > mSDASession->getFadeInDelay()) {
			mFadeInDelay = false;
			timeline().apply(&mSDASettings->iAlpha, 0.0f, 1.0f, 1.5f, EaseInCubic());
		}
	}
	gl::ScopedModelMatrix scpModel;
	gl::translate(0.5f * mSDASettings->mRenderWidth, 0.5f * mSDASettings->mRenderHeight);
	//gl::translate(mScale() * mSDASettings->mRenderWidth, mScale() * mSDASettings->mRenderHeight);
	gl::scale(mScale(), mScale());
	if (texIndex > mTexs.size() - 1) texIndex = 0;
	gl::draw(mTexs[texIndex].mTexture);
	gl::translate(0.1f * mSDASettings->mRenderWidth, 0.1f * mSDASettings->mRenderHeight);
	gl::draw(mTexs[2].mTexture);

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
	getWindow()->setTitle(mSDASettings->sFps + " fps SDA");
}

void prepareSettings(App::Settings *settings)
{
	settings->setWindowSize(640, 480);
}

CINDER_APP(PeoplizationApp, RendererGl, prepareSettings)
