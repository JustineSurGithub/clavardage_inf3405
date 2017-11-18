#undef UNICODE

#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <winsock2.h>
#include <iostream>
#include <algorithm>
#include <strstream>
#include <vector>
#include <map>
#include <ctime>

#include "ConnectionInfos.h"
#include "Communications.h"
#include "DataBase.h"

//Constantes
//const int TAILLE_MAX_MESSAGES = 200;

//Enum qui tient compte des diff�rentes issues possibles � l'authentification
enum AuthentificationRep
{
	Acceptation,
	Creation,
	Refus,
};

struct UserInfo {
	string username;
	string ip;
	string port;
};

// link with Ws2_32.lib
#pragma comment( lib, "ws2_32.lib" )

using namespace std;


// External functions
DWORD WINAPI EchoHandler(void* sd_);
void diffuser(char *msg);
void envoyer(char* msg, SOCKET sd);
void enregistrer(char *msg);
char * creerFormatMessage(char *msg);
AuthentificationRep Authentifier(SOCKET sd);

//Vecteur des sockets qui tient lieu de �chatroom�
vector<SOCKET> sockets;
HANDLE socket_mutex;

//Map des pseudos associ�s aux sockets
map<SOCKET, UserInfo*> pseudonymes;
HANDLE pseudo_mutex;

// Communications
Communications comm;
DataBase db;

// List of Winsock error constants mapped to an interpretation string.
// Note that this list must remain sorted by the error constants'
// values, because we do a binary search on the list when looking up
// items.
static struct ErrorEntry {
	int nID;
	const char* pcMessage;

	ErrorEntry(int id, const char* pc = 0) :
		nID(id),
		pcMessage(pc)
	{
	}

	bool operator<(const ErrorEntry& rhs) const
	{
		return nID < rhs.nID;
	}
} gaErrorList[] = {
	ErrorEntry(0,                  "No error"),
	ErrorEntry(WSAEINTR,           "Interrupted system call"),
	ErrorEntry(WSAEBADF,           "Bad file number"),
	ErrorEntry(WSAEACCES,          "Permission denied"),
	ErrorEntry(WSAEFAULT,          "Bad address"),
	ErrorEntry(WSAEINVAL,          "Invalid argument"),
	ErrorEntry(WSAEMFILE,          "Too many open sockets"),
	ErrorEntry(WSAEWOULDBLOCK,     "Operation would block"),
	ErrorEntry(WSAEINPROGRESS,     "Operation now in progress"),
	ErrorEntry(WSAEALREADY,        "Operation already in progress"),
	ErrorEntry(WSAENOTSOCK,        "Socket operation on non-socket"),
	ErrorEntry(WSAEDESTADDRREQ,    "Destination address required"),
	ErrorEntry(WSAEMSGSIZE,        "Message too long"),
	ErrorEntry(WSAEPROTOTYPE,      "Protocol wrong type for socket"),
	ErrorEntry(WSAENOPROTOOPT,     "Bad protocol option"),
	ErrorEntry(WSAEPROTONOSUPPORT, "Protocol not supported"),
	ErrorEntry(WSAESOCKTNOSUPPORT, "Socket type not supported"),
	ErrorEntry(WSAEOPNOTSUPP,      "Operation not supported on socket"),
	ErrorEntry(WSAEPFNOSUPPORT,    "Protocol family not supported"),
	ErrorEntry(WSAEAFNOSUPPORT,    "Address family not supported"),
	ErrorEntry(WSAEADDRINUSE,      "Address already in use"),
	ErrorEntry(WSAEADDRNOTAVAIL,   "Can't assign requested address"),
	ErrorEntry(WSAENETDOWN,        "Network is down"),
	ErrorEntry(WSAENETUNREACH,     "Network is unreachable"),
	ErrorEntry(WSAENETRESET,       "Net connection reset"),
	ErrorEntry(WSAECONNABORTED,    "Software caused connection abort"),
	ErrorEntry(WSAECONNRESET,      "Connection reset by peer"),
	ErrorEntry(WSAENOBUFS,         "No buffer space available"),
	ErrorEntry(WSAEISCONN,         "Socket is already connected"),
	ErrorEntry(WSAENOTCONN,        "Socket is not connected"),
	ErrorEntry(WSAESHUTDOWN,       "Can't send after socket shutdown"),
	ErrorEntry(WSAETOOMANYREFS,    "Too many references, can't splice"),
	ErrorEntry(WSAETIMEDOUT,       "Connection timed out"),
	ErrorEntry(WSAECONNREFUSED,    "Connection refused"),
	ErrorEntry(WSAELOOP,           "Too many levels of symbolic links"),
	ErrorEntry(WSAENAMETOOLONG,    "File name too long"),
	ErrorEntry(WSAEHOSTDOWN,       "Host is down"),
	ErrorEntry(WSAEHOSTUNREACH,    "No route to host"),
	ErrorEntry(WSAENOTEMPTY,       "Directory not empty"),
	ErrorEntry(WSAEPROCLIM,        "Too many processes"),
	ErrorEntry(WSAEUSERS,          "Too many users"),
	ErrorEntry(WSAEDQUOT,          "Disc quota exceeded"),
	ErrorEntry(WSAESTALE,          "Stale NFS file handle"),
	ErrorEntry(WSAEREMOTE,         "Too many levels of remote in path"),
	ErrorEntry(WSASYSNOTREADY,     "Network system is unavailable"),
	ErrorEntry(WSAVERNOTSUPPORTED, "Winsock version out of range"),
	ErrorEntry(WSANOTINITIALISED,  "WSAStartup not yet called"),
	ErrorEntry(WSAEDISCON,         "Graceful shutdown in progress"),
	ErrorEntry(WSAHOST_NOT_FOUND,  "Host not found"),
	ErrorEntry(WSANO_DATA,         "No host data of that type was found")
};
const int kNumMessages = sizeof(gaErrorList) / sizeof(ErrorEntry);


//// WSAGetLastErrorMessage ////////////////////////////////////////////
// A function similar in spirit to Unix's perror() that tacks a canned 
// interpretation of the value of WSAGetLastError() onto the end of a
// passed string, separated by a ": ".  Generally, you should implement
// smarter error handling than this, but for default cases and simple
// programs, this function is sufficient.
//
// This function returns a pointer to an internal static buffer, so you
// must copy the data from this function before you call it again.  It
// follows that this function is also not thread-safe.
const char* WSAGetLastErrorMessage(const char* pcMessagePrefix, int nErrorID = 0)
{
	// Build basic error string
	static char acErrorBuffer[256];
	ostrstream outs(acErrorBuffer, sizeof(acErrorBuffer));
	outs << pcMessagePrefix << ": ";

	// Tack appropriate canned message onto end of supplied message 
	// prefix. Note that we do a binary search here: gaErrorList must be
	// sorted by the error constant's value.
	ErrorEntry* pEnd = gaErrorList + kNumMessages;
	ErrorEntry Target(nErrorID ? nErrorID : WSAGetLastError());
	ErrorEntry* it = lower_bound(gaErrorList, pEnd, Target);
	if ((it != pEnd) && (it->nID == Target.nID)) {
		outs << it->pcMessage;
	}
	else {
		// Didn't find error in list, so make up a generic one
		outs << "unknown error";
	}
	outs << " (" << Target.nID << ")";

	// Finish error message off and return it.
	outs << ends;
	acErrorBuffer[sizeof(acErrorBuffer) - 1] = '\0';
	return acErrorBuffer;
}

void getTime(tm* tim) {
	time_t t = time(0);
	localtime_s(tim, &t);
}

void formatDateTime(tm* timeInfo, string* date, string* time) {
	char buf[3];

	*date += to_string(timeInfo->tm_year + 1900);
	*date += "-";
	sprintf_s(buf, "%02d", (timeInfo->tm_mon + 1));
	*date += string(buf);
	*date += "-";
	sprintf_s(buf, "%02d", (timeInfo->tm_mday));
	*date += string(buf);

	sprintf_s(buf, "%02d", (timeInfo->tm_hour));
	*time += string(buf);
	*time += ":";
	sprintf_s(buf, "%02d", (timeInfo->tm_min));
	*time += string(buf);
	*time += ":";
	sprintf_s(buf, "%02d", (timeInfo->tm_sec));
	*time += string(buf);
}

int main(void)
{
	SetConsoleTitle("Serveur de clavardage");

	// Infos de connexion
	ConnectionInfos infos;

	infos.setHost("Entrez l'adresse IP du poste sur lequel s'execute le serveur : ");
	infos.setPort();

	char host[ConnectionInfos::HOST_BUFFER_LENGTH];
	int* port = new int(0); // delete port;

	infos.getHostChar(host);
	infos.getPortInt(port);

	//----------------------
	// Initialize Winsock.
	WSADATA wsaData;
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != NO_ERROR) {
		cerr << "Error at WSAStartup()\n" << endl;
		return 1;
	}

	//----------------------
	// Create a SOCKET for listening for
	// incoming connection requests.
	SOCKET ServerSocket;
	ServerSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (ServerSocket == INVALID_SOCKET) {
		cerr << WSAGetLastErrorMessage("Error at socket()") << endl;
		WSACleanup();
		return 1;
	}
	char option[] = "1";
	setsockopt(ServerSocket, SOL_SOCKET, SO_REUSEADDR, option, sizeof(option));

	//----------------------
	// The sockaddr_in structure specifies the address family,

	//Recuperation de l'adresse locale
	hostent* thisHost;
	thisHost = gethostbyname(host);
	char* ip;
	ip = inet_ntoa(*(struct in_addr*) *thisHost->h_addr_list);
	printf("Adresse locale trouvee %s : \n\n", ip);
	sockaddr_in service;
	service.sin_family = AF_INET;
	//service.sin_addr.s_addr = inet_addr("127.0.0.1");
	//	service.sin_addr.s_addr = INADDR_ANY;
	service.sin_addr.s_addr = inet_addr(ip);
	service.sin_port = htons(*port);

	if (bind(ServerSocket, (SOCKADDR*)&service, sizeof(service)) == SOCKET_ERROR) {
		cerr << WSAGetLastErrorMessage("bind() failed.") << endl;
		closesocket(ServerSocket);
		WSACleanup();
		return 1;
	}

	//----------------------
	// Listen for incoming connection requests.
	// on the created socket
	if (listen(ServerSocket, 30) == SOCKET_ERROR) {
		cerr << WSAGetLastErrorMessage("Error listening on socket.") << endl;
		closesocket(ServerSocket);
		WSACleanup();
		return 1;
	}

	//----------------------
	// Ouvrir la DB pour pr�parer la lecture et l'�criture


	printf("En attente des connections des clients sur le port %d...\n\n", ntohs(service.sin_port));

	//Cr�er les mutex puisqu'on aura s�rement plusieurs clients
	socket_mutex = CreateMutex(NULL, FALSE, NULL);
	pseudo_mutex = CreateMutex(NULL, FALSE, NULL);

	while (true) {
		sockaddr_in sinRemote;
		int nAddrSize = sizeof(sinRemote);
		// Create a SOCKET for accepting incoming requests.
		// Accept the connection.
		SOCKET sd = accept(ServerSocket, (sockaddr*)&sinRemote, &nAddrSize);
		if (sd != INVALID_SOCKET) {
			cout << "Connection acceptee De : " <<
					inet_ntoa(sinRemote.sin_addr) << ":" <<
					ntohs(sinRemote.sin_port) << "." <<
					endl;
			AuthentificationRep resultat = Authentifier(sd);
			if (resultat == AuthentificationRep::Acceptation || resultat == AuthentificationRep::Creation)
			{
				WaitForSingleObject(socket_mutex, INFINITE);
				sockets.push_back(sd);
				ReleaseMutex(socket_mutex);
				DWORD nThreadID;
				CreateThread(0, 0, EchoHandler, (void*)sd, 0, &nThreadID);
			}
			else {
				closesocket(sd); // Ce client a ete refuse e l'authentification.
			}
		}
		else {
			cerr << WSAGetLastErrorMessage("Echec d'une connection.") <<
				endl;
			// return 1;
		}
	}
}


//// EchoHandler ///////////////////////////////////////////////////////
// Handles the incoming data by reflecting it back to the sender.

DWORD WINAPI EchoHandler(void* sd_)
{
	SOCKET sd = (SOCKET)sd_;
	UserInfo* usr = pseudonymes[sd];

	while (true)
	{
		// Le serveur lit ce que le client lui envoie.
		char readBuffer[TAILLE_MAX_MESSAGES];
		int readBytes;

		readBytes = recv(sd, readBuffer, TAILLE_MAX_MESSAGES, 0);
		if (readBytes > 0) {
			//cout << "Received " << readBytes << " bytes from client." << endl;
			//cout << "Received " << readBuffer << " from client." << endl;

			// Get date and time
			tm* timeS = new tm;
			getTime(timeS);
			string* date = new string; // delete date;
			string* time = new string; // delete time;
			formatDateTime(timeS, date, time);

			// Format message with header
			char msgFormatte[TAILLE_MAX_MESSAGES];
			string header = comm.createChatMsgInfoHeader(usr->username, usr->ip, usr->port, *date, *time);
			string contenu = comm.getContentFromChatMsg(readBuffer);
			comm.createChatMsgEcho(header, contenu, msgFormatte);
			
			// Add to DB and broadcast
			string* msgEchoContent = new string;
			if (!comm.getEchoFromMsg(msgEchoContent, msgFormatte)) {
				// Error: not an echo message
			}
			db.addMessage(&((*msgEchoContent)[0]));
			diffuser(msgFormatte);
		}
		else if (readBytes == SOCKET_ERROR) {
			cout << WSAGetLastErrorMessage("Echec de la reception !") << endl;
			if (WSAGetLastError() == WSAECONNRESET)
				break;
		}
	}
	// Le serveur publie sur sa console la d�connexion.
	cout << "Le client " << sd << " s'est deconnecte." << endl;
	closesocket(sd);

	return 0;
}

// Authentifier l'utilisateur qui parle � un certain socket en s'assurant qu'il fait partie de la bd
AuthentificationRep Authentifier(SOCKET sd)
{
	AuthentificationRep rep;
	// Attend le pseudonyme et le mot de passe du client
	char readBuffer[TAILLE_MAX_MESSAGES];
	int readBytes;
	string* pseudo = new string;
	string* motPasse = new string;
	char secante = ' ';

	readBytes = recv(sd, readBuffer, TAILLE_MAX_MESSAGES, 0);
	if (readBytes > 0)
	{
		//cout << "Received " << readBytes << " bytes from client." << endl;
		//cout << "Received " << readBuffer << " from client." << endl;
	} else {
		rep = AuthentificationRep::Refus;
		return rep;
	}

	// Extraction des informations d'authentification
	if (!comm.getAuthentificationInfoFromRequest(readBuffer, pseudo, motPasse)) {
		// Error
	}

	// Verifie dans la bd si le pseudonyme et le mot de passe sont corrects; d�termine le type de retour.
	bool userExists = db.isExistingUser(*motPasse);
	if (userExists) {
		// Utilisateur existe, verification du mot de passe
		bool isValidUserInfo = db.isValidPassword(*pseudo, *motPasse);
		if (isValidUserInfo) {
			// Mot de passe valide
			rep = AuthentificationRep::Acceptation;
		} else {
			// Mot de passe invalide
			rep = AuthentificationRep::Refus;
		}
	}
	else {
		rep = AuthentificationRep::Creation;
	}

	// Si le type est cr�ation on ajoute � la bd (acces sans mutex, table des pseudos). On envoie un message de confirmation de cr�ation de compte.
	if (rep == AuthentificationRep::Creation) {
		db.createUser(*pseudo, *motPasse);
	}

	// On met le pseudonyme dans la map des pseudos (acces avec mutex).
	sockaddr_in client_info = { 0 };
	int addrsize = sizeof(client_info);
	// or get it from the socket itself at any time
	getpeername(sd, (sockaddr*)&client_info, &addrsize);
	
	UserInfo* usr = new UserInfo;
	usr->username = *pseudo;
	usr->ip = string(inet_ntoa(client_info.sin_addr));
	usr->port = string(to_string(ntohs(client_info.sin_port)));

	WaitForSingleObject(pseudo_mutex, INFINITE);
	pseudonymes.insert(pair<SOCKET, UserInfo*>(sd, usr));
	ReleaseMutex(pseudo_mutex);

	// Creation du message de reponse
	char authReplyMsg[TAILLE_MAX_MESSAGES];
	comm.createAuthentificationReplyMsg((rep == AuthentificationRep::Acceptation || rep == AuthentificationRep::Creation), authReplyMsg);

	// Envoit du message de reponse
	envoyer(authReplyMsg, sd);

	// On envoie le nombre de messages dans l'historique.
	vector<string> msgHist = db.getMessageHistory();
	int nbMsgHist = msgHist.size();

	char histNbMsg[TAILLE_MAX_MESSAGES];
	comm.createMessageHistoryAmountMsg(nbMsgHist, histNbMsg);

	envoyer(histNbMsg, sd);

	// On envoie les <=15 derniers messages (acces avec mutex).
	auto it = msgHist.begin();
	while (it != msgHist.end()) {
		envoyer(&(*it)[0u], sd);
		++it;
	}

	// TODO: On envoie aussi un message de bienvenue � l'usager et on diffuse un message de notification aux autres pour la connexion de la nouvelle personne.

	// Retourner le type de retour.
	return rep;
}

// Enjoliver le message en rajoutant les informations obligatoires comme le nom d'utilisateur
char* creerFormatMessage(char *msg)
{
	// Formater selon nom d'utilisateur et adresse ip (getpeername), date, heure puis message. Retourner.


	// TODO: enlever ceci (permet la compilation)
	return new char;
}


// Envoyer sur un socket
void envoyer(char* msg, SOCKET socket)
{
	WaitForSingleObject(socket_mutex, INFINITE);
	send(socket, msg, TAILLE_MAX_MESSAGES, 0);
	ReleaseMutex(socket_mutex);
}

// Diffuser aux autres sockets
void diffuser(char *msg)
{
	// Pour tous les sockets du chatroom, envoyer le message.
	WaitForSingleObject(socket_mutex, INFINITE);
	for (SOCKET socket : sockets) {
		send(socket, msg, TAILLE_MAX_MESSAGES, 0);
	}
	ReleaseMutex(socket_mutex);

}
