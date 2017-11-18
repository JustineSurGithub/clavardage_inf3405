#pragma once

#include <string>
#include <vector>
#include <iostream>

using namespace std;

static const int MESSAGE_HISTORY_MAX = 15;

class DataBase
{
public:
	DataBase();
	~DataBase();

	bool isExistingUser(const string& username);
	bool isValidPassword(const string& username, const string& password);
	void createUser(const string& username, const string& password);

	vector<string> getMessageHistory();

	void addMessage(char* msg);

private:
	// TEST
	vector<string> msgDb_;

};