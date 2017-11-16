//#undef UNICODE

#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <vector>

#include "ConnectionInfos.h"

using namespace std;


int __cdecl main(int argc, char **argv)
{
	string host, port;
	ConnectionInfos infos;

	infos.setHost("Entrez l'adresse IP du poste sur lequel s'execute le serveur : ");
	infos.setPort();

	host = infos.getHost();
	port = infos.getPort();

	cout << host << endl;
	cout << port << endl;

	/*
	//----------------------------
	char *adresseParfaite;
	adresseParfaite = "132.207.29.1XY";
	bool areTheSame = false;
	while (!areTheSame) {
		// Demander � l'usager l'addr serveur
		printf("Saisir l'adresse IP sur laquelle s'ex�cute le serveur entre 132.207.29.101 et 132.207.29.127 : ");
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


	// getaddrinfo obtient l'adresse IP du host donn�
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

	//----------------------------
	// Demander � l'usager un mot a envoyer au serveur
	printf("Saisir un mot de 7 lettres pour envoyer au serveur: ");
	gets_s(motEnvoye);

	//-----------------------------
	// Envoyer le mot au serveur
	iResult = send(leSocket, motEnvoye, 7, 0);
	if (iResult == SOCKET_ERROR) {
		printf("Erreur du send: %d\n", WSAGetLastError());
		closesocket(leSocket);
		WSACleanup();
		printf("Appuyez une touche pour finir\n");
		getchar();

		return 1;
	}

	printf("Nombre d'octets envoyes : %ld\n", iResult);

	//------------------------------
	// Maintenant, on va recevoir l' information envoy�e par le serveur
	iResult = recv(leSocket, motRecu, 7, 0);
	if (iResult > 0) {
		printf("Nombre d'octets recus: %d\n", iResult);
		motRecu[iResult] = '\0';
		printf("Le mot recu est %*s\n", iResult, motRecu);
	}
	else {
		printf("Erreur de reception : %d\n", WSAGetLastError());
	}

	// cleanup
	closesocket(leSocket);
	WSACleanup();

	printf("Appuyez une touche pour finir\n");
	getchar();
	*/
	system("pause");
	return 0;
}
