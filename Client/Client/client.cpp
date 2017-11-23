/**
* Fichier : client.cpp
* Ce fichier principal declenche les threads responsables du socket.
* Christophe Bourque Bedard, Justine Pepin
* 2017/11/20
*
* D'apres le fichier de client du TP precedent.
*/
#undef UNICODE

// Suppression des avertissements
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _SCL_SECURE_NO_WARNINGS

#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <conio.h>

#include "Communications.h"
#include "ConnectionInfos.h"

// Link avec ws2_32.lib
#pragma comment(lib, "ws2_32.lib")

using namespace std;

/* Prototypes de fonction */
DWORD WINAPI receiveMsgs(void* sd_);
DWORD WINAPI sendMsgs(void* sd_);
string readPassword(const string& message);
bool createSocket(char* host, char* port);
void endConnection();

/* Informations pour le socket */
WSADATA wsaData;
SOCKET leSocket;
struct addrinfo *result = NULL,
				*ptr = NULL,
				hints;
int iResult;

/* Mutex pour le socket */
HANDLE socket_mutex = CreateMutex(NULL, FALSE, NULL);

/* Communications */
Communications comm;

/**
* Reception des messages.
*
* \param sd_ pointeur vers le socket.
* \return success.
*/
DWORD WINAPI receiveMsgs(void* sd_) {
	SOCKET sd = (SOCKET)sd_;

	while (true) {
		// Obtenir le message
		char msgEcho[TAILLE_MAX_MESSAGES];
		iResult = recv(sd, msgEcho, TAILLE_MAX_MESSAGES, 0);
		if (iResult > 0) {
			msgEcho[iResult - 1] = '\0';
		}
		else {
			// Si la connection n'a pas ete fermee par le client, afficher le message d'erreur
			int errorCode = WSAGetLastError();
			if (errorCode != 10053) {
				printf("Erreur de reception : %d\n", WSAGetLastError());
			}
			return 1;
		}

		// Extraire le contenu
		string msgEchoContent;
		if (!comm.getEchoFromMsg(msgEchoContent, msgEcho)) {
			cerr << "Error: not an echo message." << endl;
			return 1;
		}

		// Afficher le message
		cout << msgEchoContent << endl;
	}

	return 0;
}

/**
* Envoie des messages.
*
* \param sd_ pointeur vers le socket.
* \return success.
*/
DWORD WINAPI sendMsgs(void* sd_) {
	SOCKET sd = (SOCKET)sd_;

	// Lecture et envoit de messages a volonte
	string chatMsg = comm.inputChatMessage();
	while (!chatMsg.empty()) {
		char msg[TAILLE_MAX_MESSAGES];
		comm.createChatMsg(chatMsg, msg);

		iResult = send(leSocket, msg, TAILLE_MAX_MESSAGES, 0);
		if (iResult == SOCKET_ERROR) {
			printf("Erreur du send: %d\n", WSAGetLastError());
			endConnection();
			//ReleaseMutex(socket_mutex);
			printf("Appuyez une touche pour finir\n");
			getchar();
			return 1;
		}
		chatMsg = comm.inputChatMessage();
	}

	return 1;
}

/**
* Lecture d'un mot de passe sans l'afficher.
*
* \param message message d'entre du mot de passe.
* \return mot de passe.
*/
string readPassword(const string& message) {
	string pass;

	// Cacher l'echo dans la console
	HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
	DWORD mode;
	GetConsoleMode(hStdin, &mode);
	mode &= ~ENABLE_ECHO_INPUT;
	SetConsoleMode(hStdin, mode);
	
	// Lire mot de passe
	cout << message;
	getline(cin, pass);

	// Re-activation echo
	mode |= ENABLE_ECHO_INPUT;
	SetConsoleMode(hStdin, mode);

	return pass;
}

/**
* Creation du socket.
*
* \param host pointeur vers l'adresse IP de l'hote.
* \param port pointeur vers le port.
* \return success.
*/
bool createSocket(char* host, char* port) {
	//--------------------------------------------
	// Initialisation de Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("Erreur de WSAStartup: %d\n", iResult);
		return false;
	}
	// On va creer le socket pour communiquer avec le serveur
	leSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (leSocket == INVALID_SOCKET) {
		printf("Erreur de socket(): %ld\n\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		printf("Appuyez une touche pour finir\n");
		getchar();
		return false;
	}
	//--------------------------------------------
	// On va chercher l'adresse du serveur en utilisant la fonction getaddrinfo.
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;        // Famille d'adresses
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;  // Protocole utilis� par le serveur

									  // getaddrinfo obtient l'adresse IP du host donn�
	iResult = getaddrinfo(host, port, &hints, &result);
	if (iResult != 0) {
		printf("Erreur de getaddrinfo: %d\n", iResult);
		WSACleanup();
		return false;
	}
	//---------------------------------------------------------------------		
	//On parcours les adresses retournees jusqu'a trouve la premiere adresse IPV4
	while ((result != NULL) && (result->ai_family != AF_INET))
		result = result->ai_next;

	//	if ((result != NULL) &&(result->ai_family==AF_INET)) result = result->ai_next;  

	//-----------------------------------------
	if (((result == NULL) || (result->ai_family != AF_INET))) {
		freeaddrinfo(result);
		printf("Impossible de recuperer la bonne adresse\n\n");
		WSACleanup();
		printf("Appuyez une touche pour finir\n");
		getchar();
		return false;
	}

	sockaddr_in *adresse;
	adresse = (struct sockaddr_in *) result->ai_addr;
	//----------------------------------------------------
	printf("Adresse trouvee pour le serveur %s : %s\n", host, inet_ntoa(adresse->sin_addr));
	printf("Tentative de connexion au serveur %s avec le port %s\n", inet_ntoa(adresse->sin_addr), port);

	// On va se connecter au serveur en utilisant l'adresse qui se trouve dans
	// la variable result.
	iResult = connect(leSocket, result->ai_addr, (int)(result->ai_addrlen));
	if (iResult == SOCKET_ERROR) {
		printf("Impossible de se connecter au serveur %s sur le port %s\n\n", inet_ntoa(adresse->sin_addr), port);
		freeaddrinfo(result);
		WSACleanup();
		printf("Appuyez une touche pour finir\n");
		getchar();
		return false;
	}

	printf("Connecte au serveur %s:%s\n\n", host, port);
	freeaddrinfo(result);

	return true;
}

/**
* Fermeture de la connexion.
*/
void endConnection() {
	closesocket(leSocket);
	WSACleanup();
}

/*
* La fonction principale du client cree le client et autres objets du client.
* Elle lit les informations de connexion (port, hote) et cree le socket pour la communication avec le serveur.
* Elle lit ensuite les informations de l'utilisateur (nom d'utilisateur, mot de passe) et les envoie au serveur.
* Si l'authentification est acceptee, elle lit les messages a envoyer et cree un thread pour recevoir d'autres messages.
* Sinon, on ferme le client.
*/
int __cdecl main(int argc, char **argv)
{
	SetConsoleTitle("Client de clavardage");

	// Infos de connexion
	ConnectionInfos infos;

	infos.setHost("Entrez l'adresse IP du poste sur lequel s'execute le serveur : ");
	infos.setPort();

	char host[ConnectionInfos::HOST_BUFFER_LENGTH];
	char port[ConnectionInfos::PORT_BUFFER_LENGTH];

	infos.getHostChar(host);
	infos.getPortChar(port);

	// Creation socket
	if (!createSocket(host, port)) {
		cerr << "Socket fail." << endl;
		return 1;
	}

	// Nom d'utilisateur et mot de passe
	string username = "";
	// Verifier que le nom d'utilisateur n'est pas vide
	while (username.empty()) {
		cout << "Entrez votre nom d'utilisateur : ";
		getline(cin, username);
	}
	string password = readPassword("Entrez votre mot de passe : ");

	// Envoyer la requete d'authentification au serveur
	char authRequest[TAILLE_MAX_MESSAGES];
	comm.createAuthentificationRequestMsg(username, password, authRequest);

	iResult = send(leSocket, authRequest, TAILLE_MAX_MESSAGES, 0);
	if (iResult == SOCKET_ERROR) {
		printf("Erreur du send: %d\n", WSAGetLastError());
		endConnection();
		printf("Appuyez une touche pour finir\n");
		getchar();
		return 1;
	}

	// Nettoyer la console
	system("cls");

	// Reception de la reponse a la requete d'authentification
	char authReply[TAILLE_MAX_MESSAGES];
	iResult = recv(leSocket, authReply, TAILLE_MAX_MESSAGES, 0);
	if (iResult > 0) {
		authReply[iResult - 1] = '\0';

		bool passwordValid;

		bool authSuccessful = comm.getAuthentificationReplyResult(authReply, passwordValid);
		if (!authSuccessful) {
			// Raison du refus
			if (!passwordValid) {
				cout << "Erreur dans la saisie du mot de passe." << endl;
			}
			else {
				cout << "Utilisateur " << username << " deja connecte." << endl;
			}
			system("pause");
			endConnection();
			return 1;
		}
		else {
			cout << "Bienvenue dans la salle de clavardage, " << username << "!" << endl << endl;
		}
	}
	else {
		cerr << "Erreur de reception : " << WSAGetLastError() << endl;
	}
	// Si on est rendus ici dans la fonction, le client est authentifie
	// Reception du message contenant le nombre de message historiques a recevoir
	int nbrHistoryMsgs = -1;
	char msgHistoryAmount[TAILLE_MAX_MESSAGES];
	iResult = recv(leSocket, msgHistoryAmount, TAILLE_MAX_MESSAGES, 0);
	if (iResult > 0) {
		msgHistoryAmount[iResult - 1] = '\0';

		nbrHistoryMsgs = comm.getMessageHistoryAmount(msgHistoryAmount);
		if (nbrHistoryMsgs < 0) {
			cout << "Erreur dans la reception du nombre de messages historiques." << endl;
			endConnection();
			return 1;
		}
	}
	else {
		printf("Erreur de reception : %d\n", WSAGetLastError());
	}

	// Reception des derniers <= nombreMsgHistoriques messages
	for (int i = 0; i < nbrHistoryMsgs; ++i) {
		char msgHistory[TAILLE_MAX_MESSAGES];
		iResult = recv(leSocket, msgHistory, TAILLE_MAX_MESSAGES, 0);
		if (iResult > 0) {
			msgHistory[iResult - 1] = '\0';
			cout << msgHistory << endl;
		}
		else {
			printf("Erreur de reception : %d\n", WSAGetLastError());
		}
	}

	// Creer thread pour recevoir les messages
	DWORD nThreadID;
	CreateThread(0, 0, receiveMsgs, (void*)leSocket, 0, &nThreadID);

	// Envoyer les messages
	sendMsgs((void*)leSocket);

	// Fin
	endConnection();
	system("PAUSE");
	return 0;
}