/**
* Fichier : Communications.h
* Ce fichier contient des fonctions qui verifient la validite des echanges
* et qui facilitent la communication entre le client et le serveur.
* Christophe Bourque Bedard, Justine Pepin
* 2017/11/20
*/
#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <conio.h>
#include <ctime>

using namespace std;

static const int CHAT_MESSAGE_MAX_LENGTH = 200;
static const int TAILLE_MAX_MESSAGES = 260;

class Communications
{
public:
	Communications();
	~Communications();

	enum TypeMessage {
		// Mauvaise idee d'utiliser des char qui sont dans l'extended set (>127)
		PASSWORD_SEPARATOR = (char)219,		// Separateur entre username et mot de passe
		AUTHENTIFICATION_REQUEST,			// Requete d'authentification
		AUTHENTIFICATION_REPLY,				// Reponse d'authentification
		AUTHENTIFICATION_ACCEPTED,			// Reponse d'authentification : acceptation
		AUTHENTIFICATION_DENIED,			// Reponse d'authentification : rejection
		AUTHENTIFICATION_DENIED_PASSWORD,	// Complement rejection : mauvais mot de passe
		AUTHENTIFICATION_DENIED_CONNECTED,	// Complement rejection : utilisateur deja connecte
		MESSAGE_HISTORY_AMOUNT,				// Message contenant le nombre de message de l'historique a recevoir
		MESSAGE_ECHO,						// Message recu du serveur (de la part d'autres clients)
		MESSAGE,							// Message envoye de la part d'un client
	};

	string inputChatMessage();

	string createChatMsgInfoHeader(string& user, string& ip, string& port, string& date, string& time);
	void createMsg(TypeMessage type, string& contenu, char* msg);
	void createAuthentificationRequestMsg(string& user, string& pass, char* msg);
	void createAuthentificationReplyMsg(bool isAccepted, bool isPasswordValid, char* msg);
	void createMessageHistoryAmountMsg(int numberOfMsg, char* msg);
	void createChatMsgEcho(string& header, string& contenu, char* msg);
	void createChatMsg(string& txt, char* msg);

	TypeMessage getTypeFromMsg(char* msg);
	bool getAuthentificationInfoFromRequest(char* msg, string* user, string* pass);
	bool getAuthentificationReplyResult(char* msg, bool& passwordValid);
	int getMessageHistoryAmount(char* msg);
	string getContentFromChatMsg(char* msg);
	bool getEchoFromMsg(string& echo, char* msg);

	void getDateTime(string& dateStr, string& timeStr);

private:
	void stringToCharPointer(string& str, char* chr);

};