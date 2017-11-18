#include "DataBase.h"

DataBase::DataBase()
{
}

DataBase::~DataBase()
{
}

/**
* Verifie si un utilisateur existe dans la DB.
*
* \param username nom d'utilisateur.
* \return resultat.
*/
bool DataBase::isExistingUser(const string& username) {


	return true;
}

/**
* Verifie si le mot de passe est valide.
*
* \param username nom d'utilisateur.
* \param password mot de passe.
* \return resultat.
*/
bool DataBase::isValidPassword(const string& username, const string& password) {


	return true;
}

/**
* Creation d'un utilisateur en l'ajoutant a la DB avec son mot de passe.
*
* \param username nom d'utilisateur.
* \param password mot de passe.
*/
void DataBase::createUser(const string& username, const string& password) {
	
}

/**
* Accesseur pour les messages de l'historique.
*
* \return derniers messages (max 15).
*/
vector<string> DataBase::getMessageHistory() {
	vector<string> msgHistory;

	// TEST
	msgHistory.push_back("THIS IS A MESSAGE");
	msgHistory.push_back("THIS IS ANOTHER MESSAGE");
	msgHistory.push_back("THIS IS A THIRD MESSAGE");

	return msgHistory;
}

/**
* Ajoute un message a la DB.
*
* \param msg message a ajouter.
*/
void DataBase::addMessage(char* msg) {
	string msg_str(msg);
	// acces avec mutex; enregistrement de la donnee dans bd. Donne un id a la donnee qui est 1 de plus que le precedent.
}