/**
* Fichier : ConnectionsInfos.h
* Ce fichier contient des fonctions qui permettent de verifier les
* informations en lien avec le port et l'adresse hote.
* Christophe Bourque Bedard, Justine Pepin
* 2017/11/20
*/
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

	void getHostChar(char* buff);
	void getPortChar(char* buff);

	void getPortInt(int* p);

	static const int NUM_PERIODS;
	static const int NUM_BYTES;
	static const int BYTE_MAX_VALUE;
	static const int PORT_MIN;
	static const int PORT_MAX;
	static const int HOST_BUFFER_LENGTH = 16;
	static const int PORT_BUFFER_LENGTH = 5;

private:
	vector<string> split(const string& str, const string& delim);
	bool is_number(const std::string &s);

	string host_;
	string port_;

};