#undef UNICODE

#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>

//Constantes
const int TAILLE_MAX_MESSAGES = 200;

// Link avec ws2_32.lib
#pragma comment(lib, "ws2_32.lib")

// External functions
extern DWORD WINAPI recepteur(void* sd_);

// Outils pour terminer le thread de réception
HANDLE ghMutex;
bool neVeutPlusParler;

int __cdecl main(int argc, char **argv)
{
	WSADATA wsaData;
	SOCKET leSocket;// = INVALID_SOCKET;
	struct addrinfo *result = NULL,
		*ptr = NULL,
		hints;
	char nouvelHost[15];
	int iResult;

	//--------------------------------------------
	// InitialisATION de Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("Erreur de WSAStartup: %d\n", iResult);
		return 1;
	}
	// On va creer le socket pour communiquer avec le serveur
	leSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (leSocket == INVALID_SOCKET) {
		printf("Erreur de socket(): %ld\n\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		printf("Appuyez une touche pour finir\n");
		getchar();
		return 1;
	}
	//--------------------------------------------
	// On va chercher l'adresse du serveur en utilisant la fonction getaddrinfo.
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;        // Famille d'adresses
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;  // Protocole utilisé par le serveur

									  // On indique le nom et le port du serveur auquel on veut se connecter
									  //char *host = "L4708-XX";
									  //char *host = "L4708-XX.lerb.polymtl.ca";
									  //char *host = "add_IP locale";
	char *host = "132.207.29.101";
	char *port = "5030";

	//----------------------------
	char *adresseParfaite;
	adresseParfaite = "132.207.29.1XY";
	bool areTheSame = false;
	while (!areTheSame) {
		// Demander à l'usager l'addr serveur
		printf("Saisir l'adresse IP sur laquelle s'exécute le serveur entre 132.207.29.101 et 132.207.29.127 : ");
		gets_s(nouvelHost);
		// make sure all char are correct :
		areTheSame = true;
		for (int i = 0; i < sizeof(nouvelHost) / sizeof(char); ++i) {
			if (adresseParfaite[i] - 'X' == 0) {
				// make sure its between 0 and 2
				if (!(nouvelHost[i] - '0' >= 0 && nouvelHost[i] - '2' <= 0))
					areTheSame = false;
			}
			else if (adresseParfaite[i] - 'Y' == 0) {
				// make sure its between 1 and 7
				if (!(nouvelHost[i] - '1' >= 0 && nouvelHost[i] - '7' <= 0))
					areTheSame = false;
			}
			else {
				// make sure both adresses are the same
				if (adresseParfaite[i] != nouvelHost[i])
					areTheSame = false;
			}

		}
	}


	host = nouvelHost;


	// getaddrinfo obtient l'adresse IP du host donné
	iResult = getaddrinfo(host, port, &hints, &result);
	if (iResult != 0) {
		printf("Erreur de getaddrinfo: %d\n", iResult);
		WSACleanup();
		return 1;
	}
	//---------------------------------------------------------------------		
	//On parcours les adresses retournees jusqu'a trouver la premiere adresse IPV4
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
		return 1;
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
		return 1;
	}

	printf("Connecte au serveur %s:%s\n\n", host, port);
	freeaddrinfo(result);

	// Une fois le client connecté au serveur, on veut faire l'authentification du client
	bool estUtilisateurAutorise = true;
	// switch sur le résultat : 
	// case accepté : Bienvenue, machin voici les 15 derniers messages! Voici le truc que vous devez entrer si jamais vous voulez arrêter de clavarder.
	// case créé : Bienvenue machin, nouvel utilisateur blahblahblah voici les 15 derniers messages! Voici le truc que vous devez entrer si jamais vous voulez arrêter de clavarder.
	// case refusé :  Erreur dans la saisie du mot de passe; estUtilisateurAutorise = false;


	if(estUtilisateurAutorise){
		//------------------------------
		// Un thread est maintenant en charge de recevoir l'information tandis qu'on en envoie selon nos envies.
		DWORD nThreadID;
		HANDLE recepteur_th = CreateThread(0, 0, recepteur, (void*)leSocket, 0, &nThreadID);
		if (recepteur_th == NULL)
		{
			printf("CreateThread error: %d\n", GetLastError());
			return 1;
		}
		ghMutex = CreateMutex(NULL, FALSE, NULL);
		if (ghMutex == NULL)
		{
			printf("CreateMutex error: %d\n", GetLastError());
			return 1;
		}

		//-----------------------------
		// Le thread principal sert maintenant à envoyer des messages
		while (true) {
			//----------------------------
			// Attend que l'usager envoie un message
			char motEnvoye[TAILLE_MAX_MESSAGES];
			motEnvoye[0] = 0;
			printf("Saisir un message : ");
			gets_s(motEnvoye);

			//-----------------------------
			// Envoyer le message au serveur
			iResult = send(leSocket, motEnvoye, TAILLE_MAX_MESSAGES, 0);
			if (iResult == SOCKET_ERROR) {
				printf("Erreur; le serveur n'est probablement plus disponible. Code : %d\n", WSAGetLastError());
				closesocket(leSocket);
				WSACleanup();
				printf("Appuyez une touche pour finir\n");
				getchar();
				// Arrêter le thread de la réception suite à l'erreur.
				WaitForSingleObject(ghMutex, INFINITE);
				neVeutPlusParler = true;
				ReleaseMutex(ghMutex);
				WaitForSingleObject(recepteur_th, INFINITE);

				return 1;
			}

			printf("Nombre d'octets envoyes : %ld\n", iResult);
		}
		// Arrêter le thread de la réception suite à la déconnexion sécuritaire.
		WaitForSingleObject(ghMutex, INFINITE);
		neVeutPlusParler = true;
		ReleaseMutex(ghMutex);
		WaitForSingleObject(recepteur_th, INFINITE);

	}
	// Nettoyage; le client a été refusé ou le client s'est déconnecté de façon sécuritaire.
	closesocket(leSocket);
	WSACleanup();

	printf("Appuyez une touche pour finir\n");
	getchar();
	return 0;
}

//// recepteur ///////////////////////////////////////////////////////
// Thread qui est en charge de recevoir les messages du serveur une fois l'authentification résolue.

DWORD WINAPI recepteur(void* sd_)
{
	SOCKET sd = (SOCKET)sd_;

	while (true)
	{
		char motRecu[TAILLE_MAX_MESSAGES + 1];
		motRecu[0] = 0;
		int iResult = recv(sd, motRecu, TAILLE_MAX_MESSAGES, 0);
		if (iResult > 0) {
			printf("Nombre d'octets recus: %d\n", iResult);
			motRecu[iResult] = '\0';
			printf("Le mot recu est %*s\n", iResult, motRecu);
		}
		else {
			printf("Erreur de reception : %d\n", WSAGetLastError());
		}
		WaitForSingleObject(ghMutex, INFINITE);
		if (neVeutPlusParler)
			ExitThread(0);
		ReleaseMutex(ghMutex);
	}

	return 0;
}
