#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

using namespace std;

const int NUM_PERIODS = 3;
const int NUM_BYTES = NUM_PERIODS + 1;
const int BYTE_MAX_VALUE = 255;
const int PORT_MIN = 5000;
const int PORT_MAX = 5050;

/**
* Divise un string en differents sous-strings selon un delimiteur.
*
* \return vecteur resultant.
*/
vector<string> split(const string& str, const string& delim);

/**
* Verifie que le string contient seulement des chiffres (0123456789).
*
* \param s reference vers string a verifier.
* \return resultat.
*/
bool is_number(const std::string &s);

/**
* Lecture de l'adresse IP avec verifications.
*
* \return adresse IP valide (4 octets avec des nombres entre 0 et 255).
*/
string getHost(const string& prompt);

/**
* Lecture du port avec verifications.
*
* \return port valide.
*/
string getPort();