//Here is an example of a simple C++ application that combines real-time and event-driven handling:
#include <conio.h> // For kbhit () and getch ()
#include <windows.h>
#include <iostream>
#include <string>
#include <thread>
using namespace std;

// Interface for Keyboard Events
class keyboard {
private:
	string currentInput = "";

public:
	string getInput() {
		return this->currentInput;
	}
	string pollKeyboard() {
		while (true) {
			if (!_kbhit()) { continue; }
			char key = _getch();

			if (GetAsyncKeyState(key) & 0x8000) {
				if (key == VK_RETURN) {
					this->currentInput += key;
					string returnString = this->currentInput;
					this->currentInput = "";
					return returnString;
				}
				else if (key == VK_BACK) {
					if (this->currentInput.length() > 0) {
						this->currentInput.pop_back();
					}
				}
				else {
					this->currentInput += key;
				}
			}
		}
	}
	keyboard() {
		thread t(&keyboard::pollKeyboard, this);
		t.detach();
	}
};
