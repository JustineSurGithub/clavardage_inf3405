/**
* Fichier : DataBase.h
* Cette classe est responsable des acces aux fichiers de donnees et d'usagers.
* Christophe Bourque Bedard, Justine Pepin
* 2017/11/20
*/
#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <winsock2.h>

using namespace std;

class DataBase
{
public:
	DataBase();
	~DataBase();

	bool estUsagerExistant(const string& pseudonyme);
	bool estMotPasseValide(const string& pseudonyme, const string& motPasse);
	void creerUsager(const string& pseudonyme, const string& motPasse);

	vector<string> getHistoriqueMessages();
	void ajoutMessage(char* msg);

private:
	fstream memMessages_;
	fstream memUsagers_;
	HANDLE mutexUsagers_;
	HANDLE mutexMessages_;

	void ouvertureFichierLecture(string nomFichier, HANDLE mutex);
	void ouvertureFichierEcriture(string nomFichier, HANDLE mutex);
	void fermetureFichier(string nomFichier, HANDLE mutex);

	const string FICHIER_DONNEES_ = "donnees.txt";
	const string FICHIER_USAGERS_ = "usagers.txt";
};
