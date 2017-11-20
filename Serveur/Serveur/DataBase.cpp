#include "DataBase.h"
#include <fstream>
#include <queue>

#define MAX_HISTORY_SIZE 15

DataBase::DataBase()
{
	// Ouvrir la DB pour preparer la lecture et l'ecriture
	memMessages_.open("donnees.txt", ios::app);
	memUsagers_.open("usagers.txt", ios::app);
	if (!memUsagers_.is_open() || !memMessages_.is_open())
	{
			cout << "Le serveur n'arrive pas a se connecter a la base de donnees. Arret du systeme." << endl;
			exit(EXIT_FAILURE);
	}
	memMessages_.close();
	memUsagers_.close();
	mutex_usagers_ = CreateMutex(NULL, FALSE, NULL);
	mutex_messages_ = CreateMutex(NULL, FALSE, NULL);
}

DataBase::~DataBase()
{
	if (memMessages_.is_open())
	{
		memMessages_.close();
	}
	if (memUsagers_.is_open())
	{
		memUsagers_.close();
	}
}

/**
* Verifie si un utilisateur existe dans la DB.
*
* \param username nom d'utilisateur.
* \return resultat.
*/
bool DataBase::isExistingUser(const string& username) {
	// Regarder si l'un des noms des utilisateurs correspond a celui passe en parametre
	string line;
	WaitForSingleObject(mutex_usagers_, INFINITE);
	memUsagers_.clear();
	memUsagers_.seekg(0, ios::beg);
	memUsagers_.open("usagers.txt");
	while (!memUsagers_.eof())
	{
		// Lire un nom d'usager
		getline(memUsagers_, line);
		if (line == username)
		{
			memUsagers_.close();
			ReleaseMutex(mutex_usagers_);
			return true;
		}
		// Lire un mot de passe 
		getline(memUsagers_, line);
	}
	memUsagers_.close();
	ReleaseMutex(mutex_usagers_);
	return false;
}

/**
* Verifie si le mot de passe est valide.
*
* \param username nom d'utilisateur.
* \param password mot de passe.
* \return resultat.
*/
bool DataBase::isValidPassword(const string& username, const string& password) {
	// Regarder si l'un des noms des utilisateurs correspond a celui passe en parametre.
	// Si oui, regarder si le mot de passe correspond egalement.
	string line;
	WaitForSingleObject(mutex_usagers_, INFINITE);
	memUsagers_.clear();
	memUsagers_.seekg(0, ios::beg);
	memUsagers_.open("usagers.txt");
	while (!memUsagers_.eof())
	{
		// Lire un nom d'usager et verifier la correspondance.
		getline(memUsagers_, line);
		if (line == username)
		{
			// Verifier si les mots de passe correspondent aussi.
			getline(memUsagers_, line);
			if (line == password)
			{
				memUsagers_.close();
				ReleaseMutex(mutex_usagers_);
				return true;
			}
		}
		else 
		{
			// Lire un mot de passe 
			getline(memUsagers_, line);
		}
	}
	memUsagers_.close();
	ReleaseMutex(mutex_usagers_);
	return false;
}

/**
* Creation d'un utilisateur en l'ajoutant a la DB avec son mot de passe.
*
* \param username nom d'utilisateur.
* \param password mot de passe.
*/
void DataBase::createUser(const string& username, const string& password) {
	// Ecrire le nom d'usager sur une ligne puis le mot de passe.
	WaitForSingleObject(mutex_usagers_, INFINITE);
	memUsagers_.open("usagers.txt", ios::app);
	memUsagers_ << username << endl << password << endl;
	memUsagers_.close();
	ReleaseMutex(mutex_usagers_);
}

/**
* Accesseur pour les messages de l'historique.
*
* \return derniers messages (max 15).
*/
vector<string> DataBase::getMessageHistory() {
	// Differents buffers de lecture
	string line;
	vector<string> msgHistory;
	queue<string> derniersMsg;
	// Lire les derniers messages
	WaitForSingleObject(mutex_messages_, INFINITE);
	memMessages_.clear();
	memMessages_.seekg(0, ios::beg);
	memMessages_.open("donnees.txt");
	while (!memMessages_.eof())
	{
		// Lire un message et le mettre dans la queue
		getline(memMessages_, line);
		if (derniersMsg.size() == MAX_HISTORY_SIZE)
		{
			derniersMsg.pop();
		}
		derniersMsg.push(line);
	}
	memMessages_.close();
	ReleaseMutex(mutex_messages_);
	for (unsigned int i = 0; i < derniersMsg.size(); ++i) 
	{
		msgHistory.push_back(derniersMsg.front());
		derniersMsg.pop();
	}

	return msgHistory;
}

/**
* Ajoute un message a la DB.
*
* \param msg message a ajouter.
*/
void DataBase::addMessage(char* msg) {
	string msg_str(msg);
	// acces avec mutex; enregistrement de la donnee dans bd. 
	WaitForSingleObject(mutex_messages_, INFINITE);
	memMessages_.open("donnees.txt", ios::app);
	memMessages_ << msg_str << endl;
	memMessages_.close();
	ReleaseMutex(mutex_messages_);
}