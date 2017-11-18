#pragma once

#include <string>
#include <vector>

using namespace std;

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


};
