#include "HardwareTest.h"
#include <SPI.h>
#include <SD.h>
#include "Wire.h"
#include "HWTestSPIFFS.hpp"
#include <Wheelson.h>
#include <SPIFFS.h>

HardwareTest *HardwareTest::test = nullptr;

HardwareTest::HardwareTest(Display &_display) : canvas(_display.getBaseSprite()), display(&_display){

	test = this;

	tests.push_back({HardwareTest::psram, "PSRAM"});
	tests.push_back({HardwareTest::nuvotonTest, "Nuvoton"});
	tests.push_back({HardwareTest::SPIFFSTest, "SPIFFS"});

	Wire.begin(I2C_SDA, I2C_SCL);
}

void HardwareTest::start(){
	Serial.println();
	uint64_t mac = ESP.getEfuseMac();
	uint32_t upper = mac >> 32;
	uint32_t lower = mac & 0xffffffff;
	Serial.printf("TEST:begin:%x%x\n", upper, lower);

	canvas->clear(TFT_BLACK);
	canvas->setTextColor(TFT_GOLD);
	canvas->setTextFont(2);
	canvas->setTextSize(1);
	canvas->setCursor(0, 0);
	canvas->printCenter("Wheelson Hardware Test");
	canvas->setCursor(0, 10);
	canvas->println();
	display->commit();

	bool pass = true;
	for(const Test &test : tests){

		currentTest = test.name;

		canvas->setTextColor(TFT_WHITE);
		canvas->printf("%s: ", test.name);
		display->commit();

		bool result = test.test();

		canvas->setTextColor(result ? TFT_GREEN : TFT_RED);
		canvas->printf("%s\n", result ? "PASSED" : "FAILED");
		display->commit();

		if(!(pass &= result)) break;
	}

	if(pass){
		Serial.printf("TEST:pass:%s\n",currentTest);

		canvas->setTextColor(TFT_CYAN);
		canvas->printf("\n");
		canvas->printCenter("Test Successful!");
		display->commit();

	}else{
		Serial.printf("TEST:fail:%s\n", currentTest);
	}

	for(;;);
}

bool HardwareTest::psram(){
	if(!(psramFound() && psramInit())) return false;

	uint32_t free = ESP.getFreePsram();

	if(free != 4194252){
		test->log("free", free);
		return false;
	}

	return true;
}

bool HardwareTest::nuvotonTest(){

	pinMode(WSNV_PIN_RESET, OUTPUT);
	digitalWrite(WSNV_PIN_RESET, HIGH);

	/* Nuvoton reset */
	digitalWrite(WSNV_PIN_RESET, LOW);
	delay(5);
	digitalWrite(WSNV_PIN_RESET, HIGH);
	delay(500);
	LED.setBacklight(true);

	/* Test i2c transmission */
	Wire.beginTransmission(WSNV_ADDR);
	if(Wire.endTransmission() != 0){
		test->log("Wire Transmission","Failed");
		return false;
	}

	/* Nuvoton Identification */
	Wire.beginTransmission(WSNV_ADDR);
	Wire.write(IDENTIFY_BYTE);
	Wire.endTransmission();
	Wire.requestFrom(WSNV_ADDR, 1);
	if(Wire.available()){
		if(Wire.read() == WSNV_ADDR){
			return true;
		}else{
			test->log("Identification", "Failed -> Wrong data acquired.");
			return false;
		}
	}else{
		test->log("Identification", "Failed -> Wire not available.");
	}
}
bool HardwareTest::SPIFFSTest(){

	File file;

	/* SPIFFS begin test */
	if(!SPIFFS.begin()){
		test->log("Begin","Failed");
		return false;
	}

	for(const auto & check : SPIFFSChecksums){

		file = SPIFFS.open(check.name);

		if(!file){
			test->log("File open error", check.name);
			return false;
		}

		char buff;
		uint32_t fileBytesSum = 0;

		while(file.readBytes(&buff,1)){

			fileBytesSum+=buff;
		}

		if(fileBytesSum != check.sum){
			char logBuffer[100];
			sprintf(logBuffer, "%s - expected %d, got %d", check.name.c_str(), check.sum, fileBytesSum);
			test->log("Checksum mismatch", logBuffer);
			file.close();
			return false;
		}

		file.close();
	}

	return true;
}
void HardwareTest::log(const char *property, char *value){
	Serial.printf("\n%s:%s:%s\n", currentTest, property, value);
}

void HardwareTest::log(const char *property, float value){
	Serial.printf("\n%s:%s:%f\n", currentTest, property, value);
}

void HardwareTest::log(const char *property, uint32_t value){
	Serial.printf("\n%s:%s:%u\n", currentTest, property, value);
}

void HardwareTest::log(const char *property, double value){
	Serial.printf("\n%s:%s:%lf\n", currentTest, property, value);
}

void HardwareTest::log(const char *property, bool value){
	Serial.printf("\n%s:%s:%d\n", currentTest, property, value ? 1 : 0);
}

void HardwareTest::log(const char* property, String value){
	Serial.printf("\n%s:%s:%s\n", currentTest, property, value.c_str());
}
