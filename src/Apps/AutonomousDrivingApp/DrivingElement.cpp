#include "DrivingElement.h"
#include <U8g2_for_TFT_eSPI.h>

const char* const DrivingElement::Icons[] = {"/AutoDrive/engine.raw"};

DrivingElement::DrivingElement(ElementContainer* parent, DrivingIcon icon, String iconText, bool needPercentage) : CustomElement(parent, 17, 13), icon(icon), iconText(iconText), needPercentage(needPercentage){

	iconBuffer = static_cast<Color*>(ps_malloc(17 * 13 * 2));
	if(iconBuffer == nullptr){
		Serial.printf("Driving Element icon %s, unpack error\n", Icons[icon]);
		return;
	}
	fs::File bgFile = SPIFFS.open(Icons[icon]);
	bgFile.read(reinterpret_cast<uint8_t*>(iconBuffer), 17 * 13 * 2);
	bgFile.close();

	if(needPercentage){
		percentageBuffer = static_cast<Color*>(ps_malloc(6 * 6 * 2));
		if(percentageBuffer == nullptr){
			Serial.println("Percentage icon unpack error\n");
			return;
		}
		fs::File bgFile = SPIFFS.open("/AutoDrive/percentage.raw");
		bgFile.read(reinterpret_cast<uint8_t*>(percentageBuffer), 6 * 6 * 2);
		bgFile.close();
	}
}

DrivingElement::~DrivingElement(){
	free(iconBuffer);
	free(percentageBuffer);
}

void DrivingElement::draw(){
	getSprite()->drawIcon(iconBuffer, getTotalX(), getTotalY(), 17, 13, 1, TFT_TRANSPARENT);

	FontWriter u8f = getSprite()->startU8g2Fonts();
	u8f.setFont(u8g2_font_profont10_tf);
	u8f.setForegroundColor(TFT_WHITE);
	u8f.setFontMode(1);
	u8f.setCursor(getTotalX() - 3, getTotalY() + 23);
	u8f.print(iconText);
	if(needPercentage){
		getSprite()->drawIcon(percentageBuffer, getTotalX() + 13, getTotalY() + 17, 6, 6, 1, TFT_TRANSPARENT);
	}

}

void DrivingElement::setIconText(const String& iconText){
	DrivingElement::iconText = iconText;
}
