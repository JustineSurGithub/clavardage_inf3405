#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

using namespace std;


class ConnectionInfos
{
public:
	ConnectionInfos();
	~ConnectionInfos();

	void setHost(const string& prompt);
	void setPort();

	string getHost();
	string getPort();

	static const int NUM_PERIODS;
	static const int NUM_BYTES;
	static const int BYTE_MAX_VALUE;
	static const int PORT_MIN;
	static const int PORT_MAX;

private:
	vector<string> split(const string& str, const string& delim);
	bool is_number(const std::string &s);

	string host_;
	string port_;

};