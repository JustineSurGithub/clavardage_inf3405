#pragma warning(disable:4996) // warnings when using std::string::copy

#include "ConnectionInfos.h"

const int ConnectionInfos::NUM_PERIODS = 3;
const int ConnectionInfos::NUM_BYTES = ConnectionInfos::NUM_PERIODS + 1;
const int ConnectionInfos::BYTE_MAX_VALUE = 255;
const int ConnectionInfos::PORT_MIN = 5000;
const int ConnectionInfos::PORT_MAX = 5050;


ConnectionInfos::ConnectionInfos() {}

ConnectionInfos::~ConnectionInfos() {}

/**
* Lecture de l'adresse IP avec verifications.
*
* \param prompt string d'entree.
*/
void ConnectionInfos::setHost(const string& prompt) {
	string h;

	bool isValid = false;
	while (!isValid) {
		cout << prompt;
		getline(cin, h);
		isValid = true;

		if (h.empty()) {
			isValid = false;
		}
		else {
			// verification du nombre de points et de la presence d'espaces
			int spaceCount = std::count(h.begin(), h.end(), ' ');
			int periodCount = std::count(h.begin(), h.end(), '.');
			if (spaceCount > 0 || periodCount != ConnectionInfos::NUM_PERIODS) {
				cout << "\tVeuillez entrer " << ConnectionInfos::NUM_BYTES << " octets separes par des points." << endl;
				isValid = false;
			}
			else {
				// extraction des octets
				vector<string> octets = split(h, ".");
				// verifier que les 4 octets sont valides
				if (octets.size() == ConnectionInfos::NUM_BYTES) {
					for (unsigned int i = 0; i < octets.size(); ++i) {
						if (!is_number(octets[i])) {
							cout << "\tVeuillez entrer " << ConnectionInfos::NUM_BYTES << " octets positifs separes par des points." << endl;
							isValid = false;
							break;
						}
						else if (stoi(octets[i]) > ConnectionInfos::BYTE_MAX_VALUE) {
							cout << "\tVeuillez entrer " << ConnectionInfos::NUM_BYTES << " octets positifs entre 0 et " << BYTE_MAX_VALUE << " separes par des points." << endl;
							isValid = false;
							break;
						}
					}
				}
				else {
					cout << "\tVeuillez entrer " << ConnectionInfos::NUM_BYTES << " octets non vides separes par des points." << endl;
					isValid = false;
				}
			}
		}
	}
	host_ = h;
}

/**
* Lecture du port avec verifications.
*/
void ConnectionInfos::setPort() {
	string p;

	bool isValid = false;
	while (!isValid) {
		cout << "Entrez le port d'ecoute : ";
		isValid = true;

		getline(cin, p);

		if (p.empty()) {
			isValid = false;
		}
		else {
			// verifier que le port contient seulement 0123456789
			if (is_number(p)) {
				// verifier si le port est entre le min et le max
				int p_num = stoi(p);
				if (!((p_num >= ConnectionInfos::PORT_MIN) && (p_num <= ConnectionInfos::PORT_MAX))) {
					isValid = false;
					cout << "\tLe port doit etre entre " << ConnectionInfos::PORT_MIN << " et " << ConnectionInfos::PORT_MAX << "." << endl;
				}
			}
			else {
				isValid = false;
				cout << "\tCeci n'est pas un nombre positif valide." << endl;
			}
		}
	}

	port_ = p;
}

/**
* Accesseur pour le host.
*
* \return adresse IP valide (4 octets avec des nombres entre 0 et 255).
*/
string ConnectionInfos::getHost() {
	return host_;
}

/**
* Accesseur pour le port.
*
* \return adresse IP valide (4 octets avec des nombres entre 0 et 255).
*/
string ConnectionInfos::getPort() {
	return port_;
}

/**
* Accesseur pour le host qui le copie dans un char*.
*
* \param buff pointeur vers le buffer dans lequel copier le host.
*/
void ConnectionInfos::getHostChar(char* buff) {
	size_t lengthHost = host_.copy(buff, host_.length());
	buff[lengthHost] = '\0';
}

/**
* Accesseur pour le port qui le copie dans un char*.
*
* \param buff pointeur vers le buffer dans lequel copier le port.
*/
void ConnectionInfos::getPortChar(char* buff) {
	size_t lengthPort = port_.copy(buff, port_.length());
	buff[lengthPort] = '\0';
}

/**
* Divise un string en differents sous-strings selon un delimiteur.
*
* \param str string a diviser.
* \param delim delimiteur.
* \return vecteur resultant.
*/
vector<string> ConnectionInfos::split(const string& str, const string& delim) {
	vector<string> tokens;
	size_t prev = 0, pos = 0;
	do
	{
		pos = str.find(delim, prev);
		if (pos == string::npos) pos = str.length();
		string token = str.substr(prev, pos - prev);
		if (!token.empty()) tokens.push_back(token);
		prev = pos + delim.length();
	} while (pos < str.length() && prev < str.length());
	return tokens;
}

/**
* Verifie que le string contient seulement des chiffres (0123456789).
*
* \param s reference vers string a verifier.
* \return resultat.
*/
bool ConnectionInfos::is_number(const std::string &s) {
	bool res = true;
	for (unsigned int i = 0; i < s.length(); ++i) {
		int val = s.at(i) - '0';
		if (val < 0 || val > 9) {
			res = false;
			break;
		}
	}
	return res;
	//return !s.empty() && std::all_of(s.begin(), s.end(), ::isdigit);
}