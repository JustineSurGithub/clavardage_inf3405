/**
* Fichier : DataBase.cpp
* Cette classe est responsable des acces aux fichiers de donnees et d'usagers.
* Christophe Bourque Bedard, Justine Pepin
* 2017/11/20
*/
#include <fstream>
#include <queue>

#include "DataBase.h"

/* L'historique retourné doit contenir les 15 derniers messages.
*  Les fichiers .txt ont une ligne vide avec eof donc on met 16.
*/
#define TAILLE_MAX_HISTORIQUE 16

/**
* Constructeur : cree les mutexes.
*/
DataBase::DataBase()
{
	mutexUsagers_ = CreateMutex(NULL, FALSE, NULL);
	mutexMessages_ = CreateMutex(NULL, FALSE, NULL);
}

/**
* Destructeur : s'assurer que les fichiers sont bien fermes.
*/
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
bool DataBase::estUsagerExistant(const string& pseudonyme)
{
	// Regarder si l'un des noms des utilisateurs correspond a celui passe en parametre
	string ligne;
	ouvertureFichierLecture(FICHIER_USAGERS_, mutexUsagers_);
	while (!memUsagers_.eof())
	{
		// Lire un nom d'usager
		getline(memUsagers_, ligne);
		if (ligne == pseudonyme)
		{
			fermetureFichier(FICHIER_USAGERS_, mutexUsagers_);
			return true;
		}
		// Lire un mot de passe 
		getline(memUsagers_, ligne);
	}
	fermetureFichier(FICHIER_USAGERS_, mutexUsagers_);
	return false;
}

/**
* Verifie si le mot de passe est valide.
*
* \param username nom d'utilisateur.
* \param password mot de passe.
* \return resultat.
*/
bool DataBase::estMotPasseValide(const string& pseudonyme, const string& motPasse)
{
	// Regarder si l'un des noms des utilisateurs correspond a celui passe en parametre.
	// Si oui, regarder si le mot de passe correspond egalement.
	string ligne;
	ouvertureFichierLecture(FICHIER_USAGERS_, mutexUsagers_);
	while (!memUsagers_.eof())
	{
		// Lire un nom d'usager et verifier la correspondance.
		getline(memUsagers_, ligne);
		if (ligne == pseudonyme)
		{
			// Verifier si les mots de passe correspondent aussi.
			getline(memUsagers_, ligne);
			if (ligne == motPasse)
			{
				fermetureFichier(FICHIER_USAGERS_, mutexUsagers_);
				return true;
			}
		}
		else 
		{
			// Lire un mot de passe 
			getline(memUsagers_, ligne);
		}
	}
	fermetureFichier(FICHIER_USAGERS_, mutexUsagers_);
	return false;
}

/**
* Creation d'un utilisateur en l'ajoutant a la DB avec son mot de passe.
*
* \param username nom d'utilisateur.
* \param password mot de passe.
*/
void DataBase::creerUsager(const string& pseudonyme, const string& motPasse)
{
	// Ecrire le nom d'usager sur une ligne puis le mot de passe.
	ouvertureFichierEcriture(FICHIER_USAGERS_, mutexUsagers_);
	memUsagers_ << pseudonyme << endl << motPasse << endl;
	fermetureFichier(FICHIER_USAGERS_, mutexUsagers_);
}

/**
* Accesseur pour les messages de l'historique.
*
* \return derniers messages (max 15).
*/
vector<string> DataBase::getHistoriqueMessages()
{
	// Differents buffers de lecture
	string ligne;
	vector<string> historiqueMsg;
	queue<string> derniersMsg;
	// Lire les derniers messages
	ouvertureFichierLecture(FICHIER_DONNEES_, mutexMessages_);
	while (!memMessages_.eof())
	{
		// Lire un message et le mettre dans la queue
		getline(memMessages_, ligne);
		if (derniersMsg.size() == TAILLE_MAX_HISTORIQUE)
		{
			derniersMsg.pop();
		}
		derniersMsg.push(ligne);
	}
	fermetureFichier(FICHIER_DONNEES_, mutexMessages_);
	unsigned int tailleHistorique = derniersMsg.size();
	for (unsigned int i = 0; i < tailleHistorique - 1; ++i) 
	{
		historiqueMsg.push_back(derniersMsg.front());
		derniersMsg.pop();
	}

	return historiqueMsg;
}

/**
* Ajoute un message a la DB.
*
* \param msg message a ajouter.
*/
void DataBase::ajoutMessage(char* msg)
{
	string msgString(msg);
	// acces avec mutex; enregistrement de la donnee dans bd. 
	ouvertureFichierEcriture(FICHIER_DONNEES_, mutexMessages_);
	memMessages_ << msgString << endl;
	fermetureFichier(FICHIER_DONNEES_, mutexMessages_);
}

/**
* Ouvrir la DB pour preparer la lecture.
*
* \param nom du fichier a ouvrir.
* \param mutex protegeant le fichier.
*/
void DataBase::ouvertureFichierLecture(string nomFichier, HANDLE mutex)
{
	WaitForSingleObject(mutex, INFINITE);
	if (nomFichier == FICHIER_DONNEES_)
	{
		memMessages_.clear();
		memMessages_.seekg(0, ios::beg);
		memMessages_.open(nomFichier);
		if (!memMessages_.is_open())
		{
			cout << "Le serveur n'arrive pas a se connecter a la base de donnees. Arret du systeme." << endl;
			exit(EXIT_FAILURE);
		}
	}
	else
	{
		memUsagers_.clear();
		memUsagers_.seekg(0, ios::beg);
		memUsagers_.open(nomFichier);
		if (!memUsagers_.is_open())
		{
			cout << "Le serveur n'arrive pas a se connecter a la base de donnees. Arret du systeme." << endl;
			exit(EXIT_FAILURE);
		}
	}
}

/**
* Ouvrir la DB pour preparer l'ecriture.
*
* \param nom du fichier a ouvrir.
* \param mutex protegeant le fichier.
*/
void DataBase::ouvertureFichierEcriture(string nomFichier, HANDLE mutex)
{
	WaitForSingleObject(mutex, INFINITE);
	if (nomFichier == FICHIER_DONNEES_)
	{
		memMessages_.open(nomFichier, ios::app);
		if (!memMessages_.is_open())
		{
			cout << "Le serveur n'arrive pas a se connecter a la base de donnees. Arret du systeme." << endl;
			exit(EXIT_FAILURE);
		}
	}
	else
	{
		memUsagers_.open(nomFichier, ios::app);
		if (!memUsagers_.is_open()) 
		{
			cout << "Le serveur n'arrive pas a se connecter a la base de donnees. Arret du systeme." << endl;
			exit(EXIT_FAILURE);
		}
	}
}

/**
* Fermer un fichier de db.
*
* \param nom du fichier a fermer.
* \param mutex protegeant le fichier.
*/
void DataBase::fermetureFichier(string nomFichier, HANDLE mutex)
{
	if (nomFichier == FICHIER_DONNEES_)
	{
		memMessages_.close();
	}
	else
	{
		memUsagers_.close();
	}
	ReleaseMutex(mutex);
}