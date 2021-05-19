#ifndef WHEELSON_FIRMWARE_AUTONOMOUSDRIVING_H
#define WHEELSON_FIRMWARE_AUTONOMOUSDRIVING_H

#include <Arduino.h>
#include <Support/Context.h>
#include <UI/Layout.h>
#include <Elements/GridMenu.h>
#include <Input/InputListener.h>
#include "DrivingElement.h"

	class AutonomousDriving : public Context {
public:
		AutonomousDriving(Display& display);

	virtual ~AutonomousDriving();

	void start() override;

	void stop() override;

	void draw() override;

protected:
	void init() override;
	void deinit() override;

private:
	static AutonomousDriving* instance;

	LinearLayout* screenLayout;
	std::vector<DrivingElement*> engines;
	Color* backgroundBuffer = nullptr;

	void buildUI();

};

#endif //WHEELSON_FIRMWARE_AUTONOMOUSDRIVING_H
