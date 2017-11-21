/**
* Fichier : serveur.cpp
* Ce fichier principal declenche les threads responsables des sockets.
* Christophe Bourque Bedard, Justine Pepin
* 2017/11/20
*
* D'apres le fichier de serveur du TP precedent.
*/
#include <iostream>
#include <algorithm>
#include <strstream>
#include <vector>
#include <map>

#include "erreursWinsock2.h"
#include "ConnectionInfos.h"
#include "Communications.h"
#include "DataBase.h"

using namespace std;

/* Prototypes de fonction */
const char* WSAGetLastErrorMessage(const char* pcMessagePrefix, int nErrorID = 0);
bool creerServeurSocket(char* hote, int* port);
DWORD WINAPI connectionHandler(void* sd_);
bool estUsagerDejaConnecte(string pseudonyme);
void envoyer(char* msg, SOCKET sd);
void diffuser(char* msg, SOCKET s);
bool Authentifier(SOCKET sd);

/* Enum qui tient compte des differentes issues possibles a l'autentification */
enum AuthentificationRep
{
	Acceptation,
	Creation,
	Refus,
};

/* Enum d'informations pour l'utilisateur */
struct UserInfo
{
	string username;
	string ip;
	string port;
};

/* Vecteur des sockets qui tient lieu de salle de clavardage */
vector<SOCKET> sockets;
HANDLE socketMutex;

/* Map des pseudos associes aux sockets */
map<SOCKET, UserInfo*> pseudonymes;
HANDLE pseudoMutex;

/* Communications */
Communications comm;

/* Database de messages et pseudos */
DataBase db;

/* Le socket lui-meme */
SOCKET ServerSocket;

/**
* Creation du socket.
*
* \param host pointeur vers l'adresse IP de l'hote.
* \param port pointeur vers le port.
* \return success.
*/
bool creerServeurSocket(char* hote, int* port)
{
	// Initialiser Winsock.
	WSADATA wsaData;
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != NO_ERROR)
	{
		cerr << "Erreur au WSAStartup()\n" << endl;
		return false;
	}

	//----------------------
	// Creer un socket pour ecouter les requetes de connexion.
	ServerSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (ServerSocket == INVALID_SOCKET)
	{
		cerr << WSAGetLastErrorMessage("Erreur au socket()") << endl;
		WSACleanup();
		return false;
	}
	char option[] = "1";
	setsockopt(ServerSocket, SOL_SOCKET, SO_REUSEADDR, option, sizeof(option));

	//----------------------
	// La structure sockaddr_in specifie la famille d'adresse.
	//Recuperation de l'adresse locale
	hostent* thisHost;
	thisHost = gethostbyname(hote);
	char* ip;
	ip = inet_ntoa(*(struct in_addr*) *thisHost->h_addr_list);
	printf("Adresse locale trouvee : %s\n", ip);
	sockaddr_in service;
	service.sin_family = AF_INET;
	service.sin_addr.s_addr = inet_addr(ip);
	service.sin_port = htons(*port);

	if (bind(ServerSocket, (SOCKADDR*)&service, sizeof(service)) == SOCKET_ERROR)
	{
		cerr << WSAGetLastErrorMessage("bind() failed.") << endl;
		closesocket(ServerSocket);
		WSACleanup();
		return false;
	}

	//----------------------
	// Verifier si on est capable d'ecouter sur un socket.
	if (listen(ServerSocket, 30) == SOCKET_ERROR)
	{
		cerr << WSAGetLastErrorMessage("Error listening on socket.") << endl;
		closesocket(ServerSocket);
		WSACleanup();
		return false;
	}

	printf("En attente des connections des clients sur le port %d...\n", ntohs(service.sin_port));
	return true;
}

/**
* Handler de nouvelles connexions au serveur par des clients.
*
* \param sd pointeur vers socket.
* \return success.
*/
DWORD WINAPI connectionHandler(void* sd_)
{
	SOCKET sd = (SOCKET)sd_;
	UserInfo* usr = pseudonymes[sd];
	// Retrouver la date et l'heure.
	string date;
	string time;

	while (true)
	{
		// Le serveur lit ce que le client lui envoie.
		char readBuffer[TAILLE_MAX_MESSAGES];
		int readBytes;

		readBytes = recv(sd, readBuffer, TAILLE_MAX_MESSAGES, 0);
		if (readBytes > 0)
		{
			comm.getDateTime(date, time);

			// Format du message avec en-tete
			char msgFormatte[TAILLE_MAX_MESSAGES];
			string header = comm.createChatMsgInfoHeader(usr->username, usr->ip, usr->port, date, time);
			string contenu = comm.getContentFromChatMsg(readBuffer);
			comm.createChatMsgEcho(header, contenu, msgFormatte);

			// Ajouter a la DB et diffuser
			string msgEchoContent;
			if (!comm.getEchoFromMsg(msgEchoContent, msgFormatte))
			{
				// Erreur : Ce n'est pas un message de type echo.
				cerr << "Erreur : Ce n'est pas un message de type echo." << endl;
				return 1;
			}
			db.ajoutMessage(&(msgEchoContent[0]));
			diffuser(msgFormatte, sd);
		}
		else if (readBytes == SOCKET_ERROR)
		{
			cerr << WSAGetLastErrorMessage("Echec de la reception!") << endl;
			if (WSAGetLastError() == WSAECONNRESET)
				break;
		}
	}

	// Le serveur publie sur sa console la deconnexion.
	cout << "L'utilisateur " << usr->username << " s'est deconnecte." << endl;

	// Retrait de l'utilisateur dans la map
	WaitForSingleObject(pseudoMutex, INFINITE);
	auto it = pseudonymes.find(sd);
	if (it != pseudonymes.end()) {
		delete it->second;
		pseudonymes.erase(it);
	}
	ReleaseMutex(pseudoMutex);

	// Fermeture du socket associe
	closesocket(sd);

	return 0;
}

/**
* Verifier si un utilisateur est deja connecte avec le pseudo.
*
* \param string pseudo de l'utilisateur.
* \return vrai si l'utilisateur est deja connecte.
*/
bool estUsagerDejaConnecte(string pseudonyme)
{
	bool estDejaConnecte = false;
	WaitForSingleObject(pseudoMutex, INFINITE);
	auto it = pseudonymes.begin();
	while (it != pseudonymes.end())
	{
		if (it->second->username == pseudonyme) estDejaConnecte = true;
		++it;
	}
	ReleaseMutex(pseudoMutex);
	return estDejaConnecte;
}

/**
* Envoyer un message a un socket specifique.
*
* \param msg pointeur vers le message a transmettre.
* \param s socket a utiliser.
*/
void envoyer(char* msg, SOCKET socket)
{
	WaitForSingleObject(socketMutex, INFINITE);
	send(socket, msg, TAILLE_MAX_MESSAGES, 0);
	ReleaseMutex(socketMutex);
}

/**
* Diffuser un message aux autres sockets a part celui specifie.
*
* \param msg pointeur vers le message a diffuser.
* \param s socket a utiliser.
*/
void diffuser(char *msg, SOCKET s)
{
	// Pour tous les sockets du chatroom, envoyer le message.
	WaitForSingleObject(socketMutex, INFINITE);
	for (SOCKET socket : sockets)
	{
		if (socket != s)
		{
			send(socket, msg, TAILLE_MAX_MESSAGES, 0);
		}
	}
	ReleaseMutex(socketMutex);
}

/**
* Authentifier l'utilisateur qui parle a un certain socket en s'assurant qu'il fait partie de la bd.
*
* \param sd socket a authentifier.
* \return authentification success.
*/
bool Authentifier(SOCKET sd)
{
	AuthentificationRep rep;

	// Attend le pseudonyme et le mot de passe du client
	char readBuffer[TAILLE_MAX_MESSAGES];
	int readBytes;
	string* pseudo = new string;
	string* motPasse = new string;

	// Reception des informations d'authentification
	readBytes = recv(sd, readBuffer, TAILLE_MAX_MESSAGES, 0);
	if (readBytes <= 0)
	{
		rep = AuthentificationRep::Refus;
		return false;
	}

	// Extraction des informations d'authentification
	if (!comm.getAuthentificationInfoFromRequest(readBuffer, pseudo, motPasse))
	{
		// Erreur d'authentification
		cerr << "Erreur: authentification." << endl;
		return false;
	}

	// Verifie dans la bd si le pseudonyme et le mot de passe sont corrects; determine le type de retour.
	bool userExists = db.estUsagerExistant(*pseudo);
	bool isValidUserInfo;
	if (userExists)
	{
		// Utilisateur existe, verification du mot de passe
		isValidUserInfo = db.estMotPasseValide(*pseudo, *motPasse);
		if (isValidUserInfo)
		{
			// Mot de passe valide
			rep = AuthentificationRep::Acceptation;
		}
		else
		{
			// Mot de passe invalide
			rep = AuthentificationRep::Refus;
		}
	}
	else
	{
		rep = AuthentificationRep::Creation;
	}

	// Si le type est creation on ajoute a la bd (acces sans mutex, table des pseudos).
	if (rep == AuthentificationRep::Creation)
	{
		db.creerUsager(*pseudo, *motPasse);
	}

	// Obtention des informations de connexion du client concerne.
	sockaddr_in client_info = { 0 };
	int addrsize = sizeof(client_info);
	getpeername(sd, (sockaddr*)&client_info, &addrsize);
	UserInfo* usr = new UserInfo;
	usr->username = *pseudo;
	usr->ip = string(inet_ntoa(client_info.sin_addr));
	usr->port = string(to_string(ntohs(client_info.sin_port)));

	if (pseudo != nullptr) delete pseudo;
	if (pseudo != nullptr) delete motPasse;

	// Verifier si utilisateur est avec le meme username est deja connecte!
	bool isAlreadyConnected = estUsagerDejaConnecte(usr->username);

	// Creation du message de reponse
	char authReplyMsg[TAILLE_MAX_MESSAGES];
	bool successAuth = (rep == AuthentificationRep::Acceptation || rep == AuthentificationRep::Creation) && !isAlreadyConnected;
	comm.createAuthentificationReplyMsg(successAuth, isValidUserInfo, authReplyMsg);

	if (successAuth)
	{
		// On met la struct dans la map des pseudos (acces avec mutex).
		WaitForSingleObject(pseudoMutex, INFINITE);
		pseudonymes.insert(pair<SOCKET, UserInfo*>(sd, usr));
		ReleaseMutex(pseudoMutex);

		// Affichage d'une notice d'authentification sur le serveur
		if (rep == AuthentificationRep::Creation)
		{
			cout << "Authentification d'un nouvel utilisateur " << usr->username << "@" << usr->ip << ":" << usr->port << " reussie." << endl;
		}
		else
		{
			cout << "Authentification de l'utilisateur " << usr->username << "@" << usr->ip << ":" << usr->port << " reussie." << endl;
		}

		// Envoit du message de reponse
		envoyer(authReplyMsg, sd);

		// On envoie le nombre de messages dans l'historique.
		vector<string> msgHist = db.getHistoriqueMessages();
		int nbMsgHist = msgHist.size();

		char histNbMsg[TAILLE_MAX_MESSAGES];
		comm.createMessageHistoryAmountMsg(nbMsgHist, histNbMsg);

		envoyer(histNbMsg, sd);

		// On envoie les <=15 derniers messages (acces avec mutex).
		auto it = msgHist.begin();
		while (it != msgHist.end())
		{
			envoyer(&(*it)[0u], sd);
			++it;
		}

		return true;
	}
	else
	{
		// Affichage d'une notice de refus d'authentification sur le serveur
		if (!isValidUserInfo)
		{
			cout << "Authentification de l'utilisateur " << usr->username << "@" << usr->ip << ":" << usr->port << " refusee : mauvais mot de passe." << endl;
		}
		else
		{
			cout << "Authentification de l'utilisateur " << usr->username << "@" << usr->ip << ":" << usr->port << " refusee : utilisateur deja connecte avec un autre client." << endl;
		}

		// Envoit du message de refus
		envoyer(authReplyMsg, sd);
		return false;
	}
}

/*
* La fonction principale du serveur cree le serveur et autres objets du serveur.
* Elle attend ensuite qu'un port la contacte et cree un socket. Pour chaque nouveau socket,
* si l'utilisateur du nouveau socket n'est pas capable de s'authentifier correctement, on supprime le socket.
* Sinon, on cree un thread pour attendre les messages de cet utilisateur correctement identifie.
*/
int main(void)
{
	SetConsoleTitle("Serveur de clavardage");

	// Infos de connexion
	ConnectionInfos infos;

	infos.setHost("Entrez l'adresse IP du poste sur lequel s'execute le serveur : ");
	infos.setPort();

	char host[ConnectionInfos::HOST_BUFFER_LENGTH];
	int* port = new int(0);

	infos.getHostChar(host);
	infos.getPortInt(port);

	// Creation socket
	if (!creerServeurSocket(host, port))
	{
		cerr << "Socket fail." << endl;
		if (port != nullptr) delete port;
		return 1;
	}

	//----------------------
	//Creer les mutex puisqu'on aura surement plusieurs clients
	socketMutex = CreateMutex(NULL, FALSE, NULL);
	pseudoMutex = CreateMutex(NULL, FALSE, NULL);

	while (true)
	{
		sockaddr_in sinRemote;
		int nAddrSize = sizeof(sinRemote);
		// Creer un socket qui accepte les requetes des clients.
		// Accepter la connexion.
		SOCKET sd = accept(ServerSocket, (sockaddr*)&sinRemote, &nAddrSize);
		if (sd != INVALID_SOCKET)
		{
			cout << "Connection acceptee de : " <<
					inet_ntoa(sinRemote.sin_addr) << ":" <<
					ntohs(sinRemote.sin_port) << "." <<
					endl;
			bool isAuthValid = Authentifier(sd);
			if (isAuthValid)
			{
				WaitForSingleObject(socketMutex, INFINITE);
				sockets.push_back(sd);
				ReleaseMutex(socketMutex);
				DWORD nThreadID;
				CreateThread(0, 0, connectionHandler, (void*)sd, 0, &nThreadID);
			}
			else
			{
				closesocket(sd); // Ce client a ete refuse l'authentification.
			}
		}
		else
		{
			cerr << WSAGetLastErrorMessage("Echec d'une connection.") << endl;

			if (port != nullptr) delete port;

			// Vider map pseudonymes
			WaitForSingleObject(pseudoMutex, INFINITE);
			auto it = pseudonymes.begin();
			while (it != pseudonymes.end())
			{
				delete it->second;
				pseudonymes.erase(it);
				++it;
			}
			ReleaseMutex(pseudoMutex);
			return 1;
		}
	}

	if (port != nullptr) delete port;

	// Vider map pseudonymes
	WaitForSingleObject(pseudoMutex, INFINITE);
	auto it = pseudonymes.begin();
	while (it != pseudonymes.end())
	{
		delete it->second;
		pseudonymes.erase(it);
		++it;
	}
	ReleaseMutex(pseudoMutex);

	return 0;
}