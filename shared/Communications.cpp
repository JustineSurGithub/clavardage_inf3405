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
* \param .
* \param msg pointeur vers le message a creer.
*/
void Communications::createChatMsgEcho(char* msg) {
	string txt;
	createMsg(TypeMessage::MESSAGE_ECHO, txt, msg);
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