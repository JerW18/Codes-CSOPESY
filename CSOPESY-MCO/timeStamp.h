#pragma once
#include <ctime>
#include <string>
using namespace std;
class timeStamp {

public: 
	time_t t;

	string getTimeStamp() {
		struct tm ltm;
		localtime_s(&ltm, &t);
		// MM/DD/YYYY, HH:MM:SS AM/PM
		char dateTime[32];
		strftime(dateTime, 32, "%m/%d/%Y, %I:%M:%S %p", &ltm);
		return string(dateTime);
	}

	timeStamp() {
		this->t = time(0);
	}
};
