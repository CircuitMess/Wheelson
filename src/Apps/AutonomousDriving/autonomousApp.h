#ifndef AUTOCAR_AUTONOMOUSAPP_H
#define AUTOCAR_AUTONOMOUSAPP_H

#include <Arduino.h>
#include <CircuitOS.h>
#include <Support/Context.h>
#include <Loop/LoopListener.h>
#include <Util/Task.h>
#include <UI/Image.h>
#include "../../Components/MotorControl.h"
#include "../../Components/AutoAction.h"
#include "../../Components/CameraFeed.h"

class AutonomousApp : public Context, public LoopListener {
public:
	AutonomousApp(Display& display);

	void draw() override;
	void start() override;
	void stop() override;

    void loop(uint micros) override;

private:
	static const char* DirectionStrings[];
	static AutonomousApp* instance;

	CameraFeed feed;
	Mutex bufferMut;

	Image contrastPopup;

	uint16_t* frameBuffers[2];
	const uint16_t* imageBuffer = nullptr;

	ulong contrastShown = -1;
	const uint16_t contrastDuration = 2000;

    MotorControl* motors;
    bool driving = false;
    AutoAction::Type currentDirection = AutoAction::FORWARD;
	Task processTask;
	static void updateFeedTask(Task* task);
	void processFrame();

	bool debug = true;
	uint frameMicros = 0;
	uint camMicros = 0;
};

#endif