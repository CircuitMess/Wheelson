#include <FS/CompressedFile.h>
#include "Edit.h"
#include "Elements/ActionElement.h"
#include "ActionSelector.h"
#include "EditModal.h"
#include <Wheelson.h>
#include <Input/Input.h>
#include <Loop/LoopManager.h>

Simple::Edit* Simple::Edit::instance = nullptr;

Simple::Edit::Edit(Display& display, Storage* storage, int16_t programIndex) : Context(display),
																			   scrollLayout(new ScrollLayout(&screen)),
																			   list(new GridLayout(scrollLayout, 5)), storage(storage), programIndex(programIndex){

	const ProgStruct* program = storage->getProg(programIndex);

	actions = std::vector<Action>(program->actions, program->actions + program->numActions);

	ActionElement* add = new ActionElement(list, static_cast<Action::Type>(Action::Type::COUNT));
	list->addChild(add);
	add->setIsSelected(true);

	buildUI();
	scrollLayout->scrollIntoView(actionNum,2);
	Edit::pack();

}

Simple::Edit::~Edit(){

}

void Simple::Edit::start(){
	Input::getInstance()->addListener(this);
	draw();
	screen.commit();

}

void Simple::Edit::stop(){
	Input::getInstance()->removeListener(this);
	storage->updateProg(programIndex, actions.data(), actions.size());
}

void Simple::Edit::draw(){
	screen.getSprite()->drawIcon(backgroundBuffer, 0, 0, 160, 128, 1);
	screen.draw();
}

void Simple::Edit::init(){
	backgroundBuffer = static_cast<Color*>(ps_malloc(160 * 128 * 2));
	if(backgroundBuffer == nullptr){
		Serial.println("Edit background picture unpack error");
		return;
	}
	fs::File backgroundFile = CompressedFile::open(SPIFFS.open("/Simple/simple_edit_bg.raw.hs"), 12, 8);
	backgroundFile.read(reinterpret_cast<uint8_t*>(backgroundBuffer), 160 * 128 * 2);
	backgroundFile.close();

}

void Simple::Edit::deinit(){
	free(backgroundBuffer);

}

void Simple::Edit::buildUI(){
	scrollLayout->setWHType(FIXED, PARENT);
	scrollLayout->setWidth(130);
	scrollLayout->addChild(list);
	//scrollLayout->setBorder(2, TFT_RED);

	list->setWHType(CHILDREN, CHILDREN);
	list->setY(100);
	list->setPadding(5);
	list->setGutter(8);

	scrollLayout->reflow();
	list->reflow();

	screen.addChild(scrollLayout);
	screen.repos();
	scrollLayout->setX(screen.getTotalX() + 15);

}

void Simple::Edit::loop(uint micros){
	currentTime = millis();
	if(currentTime - previousTime > 1000){
		previousTime = currentTime;
		seconds++;
	}
}

void Simple::Edit::selectAction(uint8_t num){
	reinterpret_cast<ActionElement*>(list->getChildren()[actionNum])->setIsSelected(false);
	actionNum = num;
	reinterpret_cast<ActionElement*>(list->getChildren()[actionNum])->setIsSelected(true);

}

void Simple::Edit::buttonPressed(uint id){
	uint8_t totalNumActions = list->getChildren().size();
	int16_t num;
	switch(id){
		case BTN_LEFT:
			if(actionNum == 0){
				selectAction(totalNumActions - 1);
			}else{
				selectAction(actionNum - 1);
			}
			if(totalNumActions > 24){
				scrollLayout->scrollIntoView(actionNum, 2);
			}
			draw();
			screen.commit();
			break;

		case BTN_RIGHT:
			if(actionNum == totalNumActions - 1){
				selectAction(0);
			}else{
				selectAction(actionNum + 1);
			}
			if(totalNumActions > 24){
				scrollLayout->scrollIntoView(actionNum, 2);
			}
			draw();
			screen.commit();
			break;

		case BTN_UP:
			if(actionNum < 5){
				num = totalNumActions - (totalNumActions % 5) + actionNum;
				if(actionNum >= totalNumActions % 5){
					num -= 5;
				}
			}else{
				num = actionNum - 5;
			}

			selectAction(num);
			scrollLayout->scrollIntoView(actionNum, 2);
			draw();
			screen.commit();
			break;

		case BTN_DOWN:

			if(actionNum >= totalNumActions - 5){
				selectAction(actionNum % 5);
			}else{
				selectAction(actionNum + 5);
			}

			scrollLayout->scrollIntoView(actionNum, 2);
			draw();
			screen.commit();
			break;

		case BTN_MID:
			if(totalNumActions == 1 || actionNum == totalNumActions - 1){
				ActionSelector* popUpModul = new ActionSelector(*this);
				popUpModul->push(this);
			}else{
				if(actions[actionNum].type == Action::LED_OFF || actions[actionNum].type == Action::LED_ON) return;
				EditModal* editModal = new EditModal(*this, &actions[actionNum]);
				editModal->push(this);
			}
			break;

		case BTN_BACK:
			LoopManager::addListener(this);
			break;
	}
}

void Simple::Edit::returned(void* data){
	Context::returned(data);
	Action::Type* podatakPtr = static_cast<Action::Type*>(data);

	Action::Type type = *podatakPtr;
	delete podatakPtr;

	actions.push_back({type});
	switch(type){
		case Action::FORWARD:
		case Action::BACKWARD:
		case Action::LEFT:
		case Action::RIGHT:
		case Action::PAUSE:
			actions.back().time = 1;
			actions.back().speed = 100;
			break;
		case Action::LED_OFF:
		case Action::LED_ON:
			actions.back().time = 0;
			break;
	}

	ActionElement* newAction = new ActionElement(list, type);

	list->addChild(newAction);
	list->getChildren().swap(list->getChildren().size() - 1, list->getChildren().size() - 2);
	actionNum = list->getChildren().size() - 1;
	list->reflow();
	screen.repos();
	scrollLayout->scrollIntoView(list->getChildren().size() - 1, 5);
	scrollLayout->setX(screen.getTotalX() + 15);
}

void Simple::Edit::buttonReleased(uint id){
	switch(id){
		case BTN_BACK:
			if(seconds == 0)return;
			LoopManager::removeListener(this);
			if(seconds < 2){
				seconds = 0;
				pop();
				return;
			}else if(seconds >= 2){
				uint8_t totalNumActions = list->getChildren().size();
				if(totalNumActions == 1 || actionNum == totalNumActions - 1) return;
				seconds = 0;
				actions.erase(actions.begin() + actionNum);
				list->getChildren().erase(list->getChildren().begin() + actionNum);
				list->reflow();
				list->repos();
				selectAction(actionNum + 1);
				draw();
				screen.commit();
			}

	}
}



