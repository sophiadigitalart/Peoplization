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
	mScale = 1.0f;
	mDuration = 4.5f;
	texIndex = 0;
	mTexturesFilepath = getAssetPath("") / "textures.xml";
	if (fs::exists(mTexturesFilepath)) {
		// load textures from file if one exists
		mTexs = SDATexture::readSettings(mSDAAnimation, loadFile(mTexturesFilepath));
		i = 0;
		for (auto tex : mTexs)
		{
			if (tex->getType() == SDATexture::SEQUENCE) {
				texIndex = i;
				mTexs[texIndex]->setSpeed(0.02f);
			}
			i++;
		}
	}
	else {
		// otherwise create a texture from scratch
		mTexs.push_back(TextureAudio::create(mSDAAnimation));
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
		mTexs[texIndex]->setSpeed((float)event.getX() / (float)getWindowWidth() / 10.0f);
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
void PeoplizationApp::keyUp(KeyEvent event)
{
	if (!mSDASession->handleKeyUp(event)) {
	}
}
void PeoplizationApp::startAnimation() 
{
	
	timeline().apply(&mScale, 2.0f, mDuration, EaseInOutQuad());
	timeline().appendTo(&mScale, 1.0f, mDuration, EaseInOutQuad()).delay(1.0f);
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
	gl::scale(mScale(), mScale());
	
	gl::draw(mTexs[texIndex]->getTexture());

	 
	i = 0;
	for (auto tex : mTexs)
	{
		int x = 128 * i;
		gl::draw(tex->getTexture(), Rectf(0 + x, 0, 128 + x, 128));
		i++;
	}

	// Spout Send
	mSpoutOut.sendViewport();
	getWindow()->setTitle(mSDASettings->sFps + " fps SDA");
}

void prepareSettings(App::Settings *settings)
{
	settings->setWindowSize(640, 480);
}

CINDER_APP(PeoplizationApp, RendererGl, prepareSettings)
