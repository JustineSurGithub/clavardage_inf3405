#undef UNICODE

// Suppression des warnings
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

Communications comm;

// Socket stuff
WSADATA wsaData;
SOCKET leSocket;// = INVALID_SOCKET;
struct addrinfo *result = NULL,
				*ptr = NULL,
				hints;
int iResult;

/**
* Lecture d'un password avec remplacement des characteres par des *.
*
* \param password entre.
*/
string readPassword(const string& prompt) {
	string pass;
	char ch;
	cout << prompt;
	ch = _getch();
	while (ch != '\r') {
		pass.push_back(ch);
		cout << '*';
		ch = _getch();
	}
	cout << endl;
	return pass;
}

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
	printf("Adresse trouvee pour le serveur %s : %s\n\n", host, inet_ntoa(adresse->sin_addr));
	printf("Tentative de connexion au serveur %s avec le port %s\n\n", inet_ntoa(adresse->sin_addr), port);

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

void endConnection() {
	closesocket(leSocket);
	WSACleanup();
}


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

	// Creation connexion
	if (!createSocket(host, port)) {
		// Fail
		return 1;
	}

	// Nom d'utilisateur et mot de passe
	string username, password;
	cout << "Entrez votre nom d'utilisateur : ";
	getline(cin, username);
	password = readPassword("Entrez votre mot de passe : ");

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
	//printf("Nombre d'octets envoyes : %ld\n", iResult);

	// Reception de la reponse a la requete d'authentification
	char authReply[TAILLE_MAX_MESSAGES];
	iResult = recv(leSocket, authReply, TAILLE_MAX_MESSAGES, 0);
	if (iResult > 0) {
		authReply[iResult-1] = '\0';
		
		bool authSuccessful = comm.getAuthentificationReplyResult(authReply);
		if (!authSuccessful) {
			cout << "Erreur dans la saisie du mot de passe." << endl << "Appuyez sur une touche pour quitter." << endl;
			getchar();
			endConnection();
			return 1;
		} else {
			cout << "Bienvenue dans la salle de clavardage!" << endl;
		}
	}
	else {
		printf("Erreur de reception : %d\n", WSAGetLastError());
	}

	// Reception du message contenant le nombre de message historiques a recevoir
	int nombreMsgHistoriques = -1;
	char msgHistoryAmount[TAILLE_MAX_MESSAGES];
	iResult = recv(leSocket, msgHistoryAmount, TAILLE_MAX_MESSAGES, 0);
	if (iResult > 0) {
		msgHistoryAmount[iResult-1] = '\0';

		nombreMsgHistoriques = comm.getMessageHistoryAmount(msgHistoryAmount);
		if (nombreMsgHistoriques < 0) {
			cout << "Erreur dans la reception du nombre de messages historiques." << endl;
			endConnection();
			return 1;
		}
	}
	else {
		printf("Erreur de reception : %d\n", WSAGetLastError());
	}

	// Reception des derniers <= nombreMsgHistoriques messages
	for (int i = 0; i < nombreMsgHistoriques; ++i) {
		char msgHistory[TAILLE_MAX_MESSAGES];
		iResult = recv(leSocket, msgHistory, TAILLE_MAX_MESSAGES, 0);
		if (iResult > 0) {
			msgHistory[iResult-1] = '\0';
			// TODO: extract info/should messages directly contain headers+message content?
			cout << msgHistory << endl;
		}
		else {
			printf("Erreur de reception : %d\n", WSAGetLastError());
		}
	}

	// TODO: take care of threads for receiving and sending at the same time
	// Lecture et envoit de messages a volonte
	cout << "Entrez votre message a envoyer : " << endl;
	string chatMsg = comm.inputChatMessage();
	while (!chatMsg.empty()) {
		char msg[TAILLE_MAX_MESSAGES];
		comm.createChatMsg(chatMsg, msg);

		iResult = send(leSocket, msg, TAILLE_MAX_MESSAGES, 0);
		if (iResult == SOCKET_ERROR) {
			printf("Erreur du send: %d\n", WSAGetLastError());
			endConnection();
			printf("Appuyez une touche pour finir\n");
			getchar();
			return 1;
		}
		chatMsg = comm.inputChatMessage();
	}


	// Fin
	endConnection();

	printf("FIN Appuyez une touche pour finir\n");
	getchar();
	return 0;
}
