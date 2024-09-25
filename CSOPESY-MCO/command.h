#pragma once
#include <string>
#include <sstream>

using namespace std;
class command {
	public: 
		string commandName;
		map<string, string> parameters;

		void argumentsParser(string command) {
			istringstream iss(command);

			iss >> this->commandName;

			for (string s; iss >> s;) {
				string value;
				iss >> value;
				parameters[s] = value;
			}
		}
		void print() {
			cout << "Command: " << commandName << endl;
			for (auto const& x : parameters) {
				cout << x.first << ": " << x.second << endl;
			}
		}
		command(string command) {
			argumentsParser(command);
		}
};