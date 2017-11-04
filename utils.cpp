#include "utils.h"

/**
* Divise un string en differents sous-strings selon un delimiteur.
*
* \return vecteur resultant.
*/
vector<string> split(const string& str, const string& delim) {
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
bool is_number(const std::string &s) {
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

/**
* Lecture de l'adresse IP avec verifications.
*
* \return adresse IP valide (4 octets avec des nombres entre 0 et 255).
*/
string getHost(const string& prompt) {
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
			if (spaceCount > 0 || periodCount != NUM_PERIODS) {
				cout << "\tVeuillez entrer " << NUM_BYTES << " octets separes par des points." << endl;
				isValid = false;
			}
			else {
				// extraction des octets
				vector<string> octets = split(h, ".");
				// verifier que les 4 octets sont valides
				if (octets.size() == NUM_BYTES) {
					for (unsigned int i = 0; i < octets.size(); ++i) {
						if (!is_number(octets[i])) {
							cout << "\tVeuillez entrer " << NUM_BYTES << " octets positifs separes par des points." << endl;
							isValid = false;
							break;
						}
						else if (stoi(octets[i]) > BYTE_MAX_VALUE) {
							cout << "\tVeuillez entrer " << NUM_BYTES << " octets positifs entre 0 et " << BYTE_MAX_VALUE << " separes par des points." << endl;
							isValid = false;
							break;
						}
					}
				}
				else {
					cout << "\tVeuillez entrer " << NUM_BYTES << " octets non vides separes par des points." << endl;
					isValid = false;
				}
			}
		}
	}
	return h;
}

/**
* Lecture du port avec verifications.
*
* \return port valide.
*/
string getPort() {
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
				if (!((p_num >= PORT_MIN) && (p_num <= PORT_MAX))) {
					isValid = false;
					cout << "\tLe port doit etre entre " << PORT_MIN << " et " << PORT_MAX << "." << endl;
				}
			}
			else {
				isValid = false;
				cout << "\tCeci n'est pas un nombre positif valide." << endl;
			}
		}
	}

	return p;
}