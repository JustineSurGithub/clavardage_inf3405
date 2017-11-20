#pragma warning(disable:4996) // warnings when using std::string::copy

#include "Communications.h"


Communications::Communications() {}

Communications::~Communications() {}

/**
* Lecture d'un message de chat.
*
* \return message de 200 char max.
*/
string Communications::inputChatMessage() {
	string message;
	getline(cin, message);
	if (message.length() > CHAT_MESSAGE_MAX_LENGTH) {
		message.resize(CHAT_MESSAGE_MAX_LENGTH);
		cout << "ATTENTION : message tronque a " << CHAT_MESSAGE_MAX_LENGTH << " caracteres." << endl;
	}
	return message;
}

/**
* Creation d'un header contenant les informations pour un message de chat.
*
* \param user nom de l'utilisation auteur du message.
* \param ip adresse IP de l'utilisation auteur du message.
* \param port port correspondant a la connection de l'auteur du message.
* \param date date du message (YYYY-MM-DD).
* \param time temps du message (HH:MM:SS).
* \return header formatte.
*/
string Communications::createChatMsgInfoHeader(string& user, string& ip, string& port, string& date, string& time) {
	return "[" + user + " - " + ip + ":" + port + " - " + date + "@" + time + "]: ";
}

/**
* Creation d'un message de base.
*
* \param type type du message a creer.
* \param contenu contenu du message.
* \param msg pointeur vers le message a creer.
*/
void Communications::createMsg(TypeMessage type, string& contenu, char* msg) {
	string message;
	message += type;
	message += contenu;
	stringToCharPointer(message, msg);
}

/**
* Creation d'un message de requete d'authentification.
*
* \param user username.
* \param pass password.
* \param msg pointeur vers le message a creer.
*/
void Communications::createAuthentificationRequestMsg(string& user, string& pass, char* msg) {
	string userpass;
	userpass += user;
	userpass += TypeMessage::PASSWORD_SEPARATOR;
	userpass += pass;
	createMsg(TypeMessage::AUTHENTIFICATION_REQUEST, userpass, msg);
}

/**
* Creation d'un message de reponse d'authentification.
*
* \param isAccepted reponse d'authentification (acceptation ou refus).
* \param isPasswordValid indique la raison du refus (true : password invalide; false : deja connecte).
* \param msg pointeur vers le message a creer.
*/
void Communications::createAuthentificationReplyMsg(bool isAccepted, bool isPasswordValid, char* msg) {
	string resp;
	if (isAccepted) {
		resp += TypeMessage::AUTHENTIFICATION_ACCEPTED;
	} else {
		resp += TypeMessage::AUTHENTIFICATION_DENIED;
		if (!isPasswordValid) {
			// Mot de passe invalide
			resp += TypeMessage::AUTHENTIFICATION_DENIED_PASSWORD;
		} else {
			// donc raison de refus = 
			resp += TypeMessage::AUTHENTIFICATION_DENIED_CONNECTED;
		}
	}
	createMsg(TypeMessage::AUTHENTIFICATION_REPLY, resp, msg);
}

/**
* Creation d'un message d'info pour le nombre de message de l'historique a recevoir.
*
* \param numberOfMsg nombre de message a recevoir.
* \param msg pointeur vers le message a creer.
*/
void Communications::createMessageHistoryAmountMsg(int numberOfMsg, char* msg) {
	createMsg(TypeMessage::MESSAGE_HISTORY_AMOUNT, to_string(numberOfMsg), msg);
}

/**
* Creation d'un echo de message de chat (fait par un autre utilisateur).
*
* \param header le header avec les informations.
* \param contenu le contenu du message.
* \param msg pointeur vers le message a creer.
*/
void Communications::createChatMsgEcho(string& header, string& contenu, char* msg) {
	createMsg(TypeMessage::MESSAGE_ECHO, header + contenu, msg);
}

/**
* Creation d'un message de chat fait par un utilisateur.
*
* \param txt contenu du message en texte.
* \param msg pointeur vers le message a creer.
*/
void Communications::createChatMsg(string& txt, char* msg) {
	createMsg(TypeMessage::MESSAGE, txt, msg);
}

/**
* Extraction du contenu d'un message recu d'un client.
*
* \param msg pointeur vers le message provenant du client.
* \return contenu.
*/
string Communications::getContentFromChatMsg(char* msg) {
	++msg;
	return string(msg);
}

/**
* Extraction du contenu d'un message d'echo recu du serveur.
*
* \param echo pointeur vers le string dans lequel mettre l'echo.
* \param msg pointeur vers le message provenant du client.
* \return success.
*/
bool Communications::getEchoFromMsg(string* echo, char* msg) {
	// Verification message
	if (getTypeFromMsg(msg) == TypeMessage::MESSAGE_ECHO) {
		++msg;
		*echo = string(msg);
		return true;
	}
	else {
		// Message invalide
		cerr << "Erreur : message d'echo invalide." << endl;
		return false;
	}
}


/**
* Extraction du type d'un message.
*
* \param msg pointeur vers le message.
* \return type du message.
*/
Communications::TypeMessage Communications::getTypeFromMsg(char* msg) {
	//check if first char is valid (wrt enum)
	return (TypeMessage)msg[0];
}

/**
* Extraction du nom d'utilisateur et du mot de passe d'un message de requete d'authentification.
*
* \param user pointeur vers le string dans lequel mettre le nom d'utilisateur.
* \param pass pointeur vers le string dans lequel mettre le password.
* \param msg pointeur vers le message a analyser.
* \return succes de l'extraction.
*/
bool Communications::getAuthentificationInfoFromRequest(char* msg, string* user, string* pass) {
	bool res = true;
	string msg_str(msg);

	// Verification message
	if ((TypeMessage)msg_str[0] == TypeMessage::AUTHENTIFICATION_REQUEST) {
		size_t sep_pos = msg_str.find((char)TypeMessage::PASSWORD_SEPARATOR);
		if (sep_pos != string::npos) {
			*user = msg_str.substr(1, sep_pos - 1);
			*pass = msg_str.substr(sep_pos + 1, string::npos);
		} else {
			res = false;
		}
	} else {
		// Not a valid authentification request message
		cerr << "Erreur : message de requete d'authentification invalide." << endl;
		res = false;
	}

	return res;
}

/**
* Extraction du resultat de la requete d'authentifcation.
*
* \param msg pointeur vers le message.
* \param passwordInvalid pointeur vers un bool a verifier si l'authentification est refusee (true : deja connecte; false : password invalide).
* \return resultat de la requete d'authentification.
*/
bool Communications::getAuthentificationReplyResult(char* msg, bool& passwordValid) {
	// Verification message
	if (getTypeFromMsg(msg) != TypeMessage::AUTHENTIFICATION_REPLY) {
		// Message invalide
		cerr << "Erreur : message de reponse d'authentification invalide." << endl;
		return false;
	}

	bool authAccepted = (TypeMessage)msg[1] == TypeMessage::AUTHENTIFICATION_ACCEPTED;
	if (!authAccepted) {
		if ((TypeMessage)msg[2] == TypeMessage::AUTHENTIFICATION_DENIED_CONNECTED) {
			// Utilisateur deja connecte
			passwordValid = true;
		} else {
			// Password invalide
			passwordValid = false;
		}
	}
	return authAccepted;
}

/**
* Extraction du nombre de messages de l'historique a recevoir.
*
* \param msg pointeur vers le message.
* \return nombre de messages de l'historique a recevoir.
*/
int Communications::getMessageHistoryAmount(char* msg) {
	// Verification message
	if (getTypeFromMsg(msg) != TypeMessage::MESSAGE_HISTORY_AMOUNT) {
		// Message invalide
		cerr << "Erreur : message de quantite de messages historiques invalide." << endl;
		return 0;
	}
	string number;
	number += msg[1];
	number += msg[2];
	return stoi(number);
}

/**
* Copie d'un string dans un tableau de char.
*
* \param str string a copier.
* \param chr pointeur vers le tableau de char dans lequel copier.
*/
void Communications::stringToCharPointer(string& str, char* chr) {
	size_t length = str.copy(chr, str.length());
	chr[length] = '\0';
}

/**
* Copie d'un string dans un tableau de char.
*
* \param timeInfo pointeur vers structure.
* \param chr pointeur vers le tableau de char dans lequel copier.
*/
void Communications::getDateTime(string* dateStr, string* timeStr) {
	tm* timeInfo = new tm;
	time_t t = time(0);
	localtime_s(timeInfo, &t);

	char buf[3];

	*dateStr += to_string(timeInfo->tm_year + 1900);
	*dateStr += "-";
	sprintf_s(buf, "%02d", (timeInfo->tm_mon + 1));
	*dateStr += string(buf);
	*dateStr += "-";
	sprintf_s(buf, "%02d", (timeInfo->tm_mday));
	*dateStr += string(buf);

	sprintf_s(buf, "%02d", (timeInfo->tm_hour));
	*timeStr += string(buf);
	*timeStr += ":";
	sprintf_s(buf, "%02d", (timeInfo->tm_min));
	*timeStr += string(buf);
	*timeStr += ":";
	sprintf_s(buf, "%02d", (timeInfo->tm_sec));
	*timeStr += string(buf);
}