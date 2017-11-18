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
		cout << "ATTENTION : message tronque a " << CHAT_MESSAGE_MAX_LENGTH << "caracteres." << endl;
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
* \param msg pointeur vers le message a creer.
*/
void Communications::createAuthentificationReplyMsg(bool isAccepted, char* msg) {
	string resp;
	if (isAccepted) {
		resp += TypeMessage::AUTHENTIFICATION_ACCEPTED;
	} else {
		resp += TypeMessage::AUTHENTIFICATION_DENIED;
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

/** TODOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO
* Creation d'un echo de message de chat (fait par un autre utilisateur).
*
* \param header le header avec les informations.
* \param contenu le contenu du message.
* \param msg pointeur vers le message a creer.
*/
void Communications::createChatMsgEcho(string& header, string& contenu, char* msg) {
	createMsg(TypeMessage::MESSAGE_ECHO, header+ contenu, msg);
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

	if ((TypeMessage)msg[0] == TypeMessage::MESSAGE_ECHO) {
		++msg;
		*echo = string(msg);
		return true;
	}
	else {
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
		res = false;
	}

	return res;
}

/**
* Extraction du resultat de la requete d'authentifcation.
*
* \param msg pointeur vers le message.
* \return resultat de la requete d'authentification.
*/
bool Communications::getAuthentificationReplyResult(char* msg) {
	// TODO: check if message header is actually TypeMessage::AUTHENTIFICATION_REQUEST
	return (TypeMessage)msg[1] == TypeMessage::AUTHENTIFICATION_ACCEPTED;
}

/**
* Extraction du nombre de messages de l'historique a recevoir.
*
* \param msg pointeur vers le message.
* \return nombre de messages de l'historique a recevoir.
*/
int Communications::getMessageHistoryAmount(char* msg) {
	// TODO: check if message header is actually TypeMessage::MESSAGE_HISTORY_AMOUNT
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