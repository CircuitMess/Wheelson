#include "IntroScreen.h"
#include "Apps/MainMenu/MainMenu.h"
#include <FS.h>
#include <Loop/LoopManager.h>
#include <FS/CompressedFile.h>
#include <SPIFFS.h>
#include <Wheelson.h>

IntroScreen::IntroScreen* IntroScreen::IntroScreen::instance = nullptr;


IntroScreen::IntroScreen::IntroScreen(Display& display) : Context(display){
	instance = this;

	fs::File f = SPIFFS.open("/intro.g565.hs");
	if(!f){
		Serial.println("Error opening intro gif");
		return;
	}

	gif = new AnimatedSprite(screen.getSprite(), CompressedFile::open(f, 14, 12));
	gif->setSwapBytes(true);
	gif->setXY(0, 0);

	IntroScreen::pack();
}

IntroScreen::IntroScreen::~IntroScreen(){
	delete gif;
	instance = nullptr;
}

void IntroScreen::IntroScreen::draw(){
	if(gif == nullptr){
		Serial.println("Intro gif error");
		return;
	}

	gif->nextFrame();
	gif->push();
}

void IntroScreen::IntroScreen::start(){
	if(!gif) return;

	gif->setLoopDoneCallback([]{
		if(instance == nullptr) return;

		Display& display = *instance->getScreen().getDisplay();

		instance->stop();
		delete instance;
		LED.setHeadlight(0);
		MainMenu* main = new MainMenu(display);
		main->unpack();
		main->start();
	});

	LoopManager::addListener(this);

	draw();
	screen.commit();
}

void IntroScreen::IntroScreen::stop(){
	LoopManager::removeListener(this);
}

void IntroScreen::IntroScreen::loop(uint micros){
	if(!gif || !gif->checkFrame()) return;
	draw();
	screen.commit();

	uint16_t currentTime = millis();
	uint16_t previousTime = 0;
	if(currentTime - previousTime >= 500){

		previousTime = currentTime;
		if(LED.getHeadlight() == 0){
			LED.setHeadlight(255);
		}else{
			LED.setHeadlight(0);
		}
	}

}