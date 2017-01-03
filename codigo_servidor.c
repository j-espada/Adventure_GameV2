/*
* Jogo de Aventuras | Servidor
*
*  Actualizado a: 18/01/2016
*  Autores: Joaquim Espada e João Orvalho
*/

#include "stdafx.h" // ficheiro com definições do Windows etc
#include <stdio.h> // para o printf etc
#include <string.h>
#include <stdlib.h> // para o system("pause") , cores etc
#include <locale.h> // Para acentuacao etc
#include <time.h>
#include <windows.h>   // WinApi header



HANDLE ghEvents[2]; // 2 - espaco do array
HANDLE hMutexMonster;
HANDLE hMutexPlayer;
HANDLE hMutexEcran;

#define VERSAO "V2.0"
#define MOVE_INFO 100


//Player options
#define MAX_NAME 100
#define MAX_ENGERGY 10
#define GOD_MODE_ENERGY 100
#define PLAYER_INIT_CELL 0
#define NO_TREASURE  0
#define TREASURE  1
#define NO_COOKIE 0
#define COOKIE 1
#define COOKIE_ENERGY 40

//Monster options
#define MONSTER_NAME "Resident"
#define MONSTER_MAX_ENGERGY 10
#define MONSTER_INIT_CELL 7

//Map options
#define MAP_CELLS 100
#define MAX_CELL_DESCRIPTION 256

//Cell description
#define CELL_0_DESCRIPTION "You are in a dark and putrid forest . Oh look a Mansion !\nDo you have the guts to enter ? \n"
#define CELL_1_DESCRIPTION "You are at the main hall you can hear ticking ... Sounds like a clock \n!"
#define CELL_2_DESCRIPTION "You are at the dinning room. I found the clock! There is something strange about this ... \nA secret passage !! \n"
#define CELL_3_DESCRIPTION "You are at a small and dark corridor! Look a door, isn´t an ordinary door , has an armor craved in it ... \nI'm going to enter ! \n"
#define CELL_4_DESCRIPTION "You are at large room . There is a piano with a partiture . I'm going to play it. Another secret passage! \n "
#define CELL_5_DESCRIPTION "You are at a studio there are some strange notes about a Treasure hidden in the courtyard .There is only one way out of this room! \n"
#define CELL_6_DESCRIPTION "A small deposit room , there is a typewritter machine with a story about the Mansion! Horrible things happened here! \n"
#define CELL_7_DESCRIPTION "You are at the courtyard there is something next to that crypt! \n"
#define CELL_8_DESCRIPTION "You are in a small closet!\n"
/* Estruturas*/

#define MAX_LIN 1000
#define MAX_L 200


struct Cell {
	int north;
	int south;
	int east;
	int west;
	char description[MAX_CELL_DESCRIPTION];
	int treasure;
	int cookie;
};

struct Map {

	struct Cell cells[MAP_CELLS];
	int nCells;
};

struct Monster
{
	char name[MAX_NAME];
	int cell;
	int energy;

};

struct Player {
	char name[MAX_NAME];
	int energy;
	int cell;
	int treasure;
};

struct Data // Estrutura com várias estruturas lá dentro
{
	struct Player player;
	struct Monster monster;
	struct Map map;

};
/* Fim Estruturas*/



/* Prototipo de funções*/
void printPlayer(struct Player *pPlayer);
void PrintMap(struct Map *pMap);
void InitializeGameItems(struct Player *pPlayer, struct Map *pMap, struct Monster *pMonster);
void GameEngine(struct Data *pData);
void InitializeCombat(struct Player *pPlayer, struct Monster *pMonster);
void LockedDoors();
void NewGame();


/* Fim prototipo de funções */


int combat = 0;
int end_game = 0;


// Global variables

char MapToUse[50];
char Extension_Map[50];
int su = 0; // para saber em que modo se joga (super user,com mais energia, ou normal, energia normal)



/*Defines para o pipe*/
#define PIPE_NAME "\\\\.\\pipe\\app1_pipe" //só é possível criar um named pipe no próprio computador
#define PIPE_NAME2 "\\\\.\\pipe\\app2_pipe" //só é possível criar um named pipe no próprio computador
#define BUF_SIZE 512
#define MAX_MSG 512
HANDLE hPipe;
HANDLE hPipe2;
char msg[MAX_MSG];
char msg2[MAX_MSG];


void PrintToConsole(char text[]) {

	WaitForSingleObject(hMutexEcran, INFINITE);
	printf("%s", text);
	fflush(stdout);
	ReleaseMutex(hMutexEcran);
}

DWORD WINAPI ThreadMonsterMov(LPVOID lpParam) // Movimento Monstro
{
	int maxTime = 10000; // 10 segundos
	int minTime = 2000; // 2 segundos

	int timeRand = (rand() % (maxTime + 1 - minTime)) + minTime; // Tempo de Espera Ramdom


	struct Data *pDados = (struct Data*) lpParam;
	struct Player *pPlayer = &pDados->player;
	struct Monster *pMonster = &pDados->monster;
	struct Map *pMap = &pDados->map;


	WaitForSingleObject(hMutexMonster, INFINITE); //novo 17 -01 -16 
	while (pPlayer->cell != pMonster->cell) {

		timeRand = (rand() % (maxTime + 1 - minTime)) + minTime; // Tempo de Espera Ramdom
		Sleep(timeRand);

		//WaitForSingleObject(hMutexMonster, INFINITE);//antigo 17 -01 -16
		//char InfoMutexBegin[256];
		//sprintf(InfoMutexBegin, "--------------Thread Monster Mov LOCK Mutex \n");
		//PrintToConsole(InfoMutexBegin);

		if (pMonster->energy > 0) {

			int  i = 0; // i= numero de portas na celula
			int doors[4];

			if (pMap->cells[pMonster->cell].west != -1) {
				doors[i] = pMap->cells[pMonster->cell].west; // adiciona porta ao array
				i++;
			}


			if (pMap->cells[pMonster->cell].south != -1) {
				doors[i] = pMap->cells[pMonster->cell].south; // adiciona porta ao array
				i++;
			}


			if (pMap->cells[pMonster->cell].north != -1) {
				doors[i] = pMap->cells[pMonster->cell].north; // adiciona porta ao array
				i++;

			}


			if (pMap->cells[pMonster->cell].east != -1) {
				doors[i] = pMap->cells[pMonster->cell].east; // adiciona porta ao array
				i++;
			}



			if (pPlayer->cell != pMonster->cell) {
				int moveX = rand() % i;
				int newPos = doors[moveX];
				pMonster->cell = newPos;

				//printf(">>>>>>>>>>>>Monster moveu-se para %d: \n", pMonster->cell);
			}



		}


		//char InfoMutexEnd[256];
		//sprintf(InfoMutexEnd, "--------------Thread Monster Mov UNLOCK Mutex \n");
		//PrintToConsole(InfoMutexEnd);
		ReleaseMutex(hMutexMonster);

	}

	return 0;
}

void PrintErrorMsg() {
	char errorMsg[80];

	if (FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,
		0,
		GetLastError(),
		0,
		errorMsg,
		80,
		NULL))
		printf("%s\n", errorMsg);
}

int sendMessageToPipe(char message[])
{

	DWORD cbWritten;
	DWORD resSuccess = WriteFile(
		hPipe,        // handle to pipe 
		message,     // buffer to write from 
		MAX_MSG, // number of bytes to write 
		&cbWritten,   // number of bytes written 
		NULL);        // not overlapped I/O 

	if (!resSuccess) {
		printf("Servidor: Erro no envio.");
		PrintErrorMsg();
		return -1;
	}
}



int sendMessageToPipe2(char message[])
{

	DWORD cbWritten;
	DWORD resSuccess = WriteFile(
		hPipe2,        // handle to pipe 
		message,     // buffer to write from 
		MAX_MSG, // number of bytes to write 
		&cbWritten,   // number of bytes written 
		NULL);        // not overlapped I/O 

	if (!resSuccess) {
		printf("Servidor: Erro no envio.");
		PrintErrorMsg();
		return -1;
	}
}

void receiveCommand(char command[], LPVOID lpParam) {

	sendMessageToPipe("lock");

	char text[500];
	struct Data *pDados = (struct Data*) lpParam;
	struct Player *pPlayer = &pDados->player;
	struct Monster *pMonster = &pDados->monster;
	struct Map *pMap = &pDados->map;
	//printf("-----------INICIO DO MOVIMENTO %d\n", pPlayer->cell);
	char moveData[MOVE_INFO];

	strcpy(moveData, command);

	WaitForSingleObject(hMutexPlayer, INFINITE);
	//char InfMutexB[256];
	//sprintf(InfMutexB, "-----------------Thread PlayerMove LOCK Mutex x \n");
	//PrintToConsole(InfMutexB);


	sprintf(text, "%s -> %d\n", pPlayer->name, pPlayer->cell); // Descreve a localizacao do utilizador
	//PrintToConsole(text);



	ReleaseMutex(hMutexPlayer); // adicionado 11-01-15
	//char InfMutexEnd2[256];
	//sprintf(InfMutexEnd2, "-----------------Thread PlayerMove UNLOCK Mutex x \n");
	//PrintToConsole(InfMutexEnd2);

	WaitForSingleObject(hMutexPlayer, INFINITE); // adicionado 11-01-15
	//sprintf(InfMutexB, "-----------------Thread PlayerMove LOCK Mutex y \n");
	//PrintToConsole(InfMutexB);

	if (pPlayer->energy > 0) {
		switch (moveData[0]) {


		case 'w':
			if (pMap->cells[pPlayer->cell].west != -1) {


				pPlayer->cell = pMap->cells[pPlayer->cell].west;
				if (pPlayer->treasure == 0)
					pPlayer->treasure = pMap->cells[pPlayer->cell].treasure;

				sprintf(text, "%s \n", pMap->cells[pPlayer->cell].description);
				//PrintToConsole(text);
				sendMessageToPipe(text);

			}
			else
				LockedDoors();
			break;

		case 's':
			if (pMap->cells[pPlayer->cell].south != -1) {


				pPlayer->cell = pMap->cells[pPlayer->cell].south;
				if (pPlayer->treasure == 0)
					pPlayer->treasure = pMap->cells[pPlayer->cell].treasure;
				sprintf(text, "%s \n", pMap->cells[pPlayer->cell].description);
				//PrintToConsole(text);
				sendMessageToPipe(text);


			}
			else
				LockedDoors();
			break;

		case 'n':
			if (pMap->cells[pPlayer->cell].north != -1) {

				pPlayer->cell = pMap->cells[pPlayer->cell].north;
				if (pPlayer->treasure == 0)
					pPlayer->treasure = pMap->cells[pPlayer->cell].treasure;
				sprintf(text, "%s \n", pMap->cells[pPlayer->cell].description);
				sendMessageToPipe(text);
				//PrintToConsole(text);

			}
			else
				LockedDoors();
			break;

		case 'e':
			if (pMap->cells[pPlayer->cell].east != -1) {

				pPlayer->cell = pMap->cells[pPlayer->cell].east;
				if (pPlayer->treasure == 0)
					pPlayer->treasure = pMap->cells[pPlayer->cell].treasure;
				sprintf(text, "%s \n", pMap->cells[pPlayer->cell].description);
				sendMessageToPipe(text);
				//PrintToConsole(text);

			}
			else
				LockedDoors();
			break;
		default:

			sprintf(text, "Invalid Movement ! \n");
			//PrintToConsole(text);
			sendMessageToPipe(text);

			break;
		}

	}


	if (pMap->cells[pPlayer->cell].treasure == 1)
	{
		sprintf(text, "You found the treasure !!! \nYou need to return to the point of the start \n");
		//PrintToConsole(text);
		sendMessageToPipe(text);
		pMap->cells[pPlayer->cell].treasure = 0; // O jogador já encontrou o tesouro!
		pPlayer->treasure = 1;
	}

	if ((pMap->cells[pPlayer->cell].cookie == 1)) // Armario com uma bolacha de energia
	{
		pPlayer->energy = pPlayer->energy + COOKIE_ENERGY;
		pMap->cells[pPlayer->cell].cookie = 0;
		sprintf(text, "%s found a cookie !!\n", pPlayer->name);
		sendMessageToPipe(text);
		//PrintToConsole(text);
		sprintf(text, "%s energy -> %d\n", pPlayer->name, pPlayer->energy);
		//PrintToConsole(text);
	}



	if (pPlayer->treasure == 1 && pPlayer->energy > 0 && pPlayer->cell == 0) {

		strcpy(msg2, "Win!");
		sendMessageToPipe(msg2);
		end_game = 1;


	}




	//printf("---------FIM DO MOVIMENTO %d\n", pPlayer->cell);
	ReleaseMutex(hMutexPlayer);
	//char InfMutexEnd[256];
	//sprintf(InfMutexEnd, "-----------------Thread PlayerMove UNLOCK Mutex y \n");
	//PrintToConsole(InfMutexEnd);


	sendMessageToPipe("unlock");

}

int InicializePipes() {



	//Inicialização das variáveis que irão permitir criar o named pipe
	SECURITY_ATTRIBUTES secAttrib;
	SECURITY_DESCRIPTOR* pSecDesc;

	pSecDesc = (SECURITY_DESCRIPTOR*)LocalAlloc(LPTR,
		SECURITY_DESCRIPTOR_MIN_LENGTH);

	InitializeSecurityDescriptor(pSecDesc,
		SECURITY_DESCRIPTOR_REVISION);

	//(PACL) NULL permite o acesso de todos ao named pipe
	SetSecurityDescriptorDacl(pSecDesc, TRUE, (PACL)NULL, FALSE);

	secAttrib.nLength = sizeof(SECURITY_ATTRIBUTES);
	secAttrib.bInheritHandle = TRUE;
	secAttrib.lpSecurityDescriptor = pSecDesc;



	//Criação do named pipe
	//Nota alterar em Project Properties o Character Set para Not Set (ASCII)
	//caso contrário irá utilizar a versão UNICODE da função CreateNamedPipe 
	hPipe = CreateNamedPipe(
		PIPE_NAME,				// pipe name 
		PIPE_ACCESS_DUPLEX,       // read/write access 
		PIPE_TYPE_BYTE |			// byte type pipe 
		PIPE_READMODE_BYTE |		// byte-read mode 
		PIPE_WAIT,                // blocking mode 
		PIPE_UNLIMITED_INSTANCES, // max. instances  
		BUF_SIZE,                 // output buffer size 
		BUF_SIZE,                 // input buffer size 
		0,                        // client time-out 
		&secAttrib);            // access to everyone

	if (hPipe == INVALID_HANDLE_VALUE) {
		printf("Servidor: Erro na criação do named pipe.\n");
		PrintErrorMsg();
		return -1;
	}

	hPipe2 = CreateNamedPipe(
		PIPE_NAME2,				// pipe name 
		PIPE_ACCESS_DUPLEX,       // read/write access 
		PIPE_TYPE_BYTE |			// byte type pipe 
		PIPE_READMODE_BYTE |		// byte-read mode 
		PIPE_WAIT,                // blocking mode 
		PIPE_UNLIMITED_INSTANCES, // max. instances  
		BUF_SIZE,                 // output buffer size 
		BUF_SIZE,                 // input buffer size 
		0,                        // client time-out 
		&secAttrib);            // access to everyone

	if (hPipe2 == INVALID_HANDLE_VALUE) {
		printf("Servidor: Erro na criação do named pipe.\n");
		PrintErrorMsg();
		return -1;
	}

}

DWORD WINAPI ThreadPlayerMov(LPVOID lpParam) // Movimento Jogador
{
	struct Data *pDados = (struct Data*) lpParam;
	struct Player *pPlayer = &pDados->player;
	struct Monster *pMonster = &pDados->monster;
	struct Map *pMap = &pDados->map;


	LPVOID lpVoid = lpParam;


	strcpy(msg, "");
	strcpy(msg2, "");
	while (strcmp(msg, "shutdown") != 0) {

		InicializePipes();
		printf("\nServidor: vou conectar-me ao named pipe para esperar por uma ligação.\n");
		ConnectNamedPipe(hPipe, NULL);
		ConnectNamedPipe(hPipe2, NULL);
		printf("\nServidor: Conexão estabelecida, vou ler do named pipe.\n");
		InitializeGameItems(pPlayer, pMap, pMonster); // Iniciliza os Itens do jogo:  mapa , jogador, monstro


		sprintf(msg, "%s \n", pMap->cells[pPlayer->cell].description);
		sendMessageToPipe(msg);

		//Leitura do named pipe
		strcpy(msg, "");
		strcpy(msg2, "");
		while (strcmp(msg, "shutdown") != 0 && strcmp(msg, "close") != 0 && end_game == 0)
		{

			if (combat == 0) {

				DWORD cbRead;

				//A função ReadFile só deve ser chamada depois de ambos os processos
				//se encontrarem ligados através do pipe
				BOOL resSuccess = ReadFile(
					hPipe,    // pipe handle 
					msg,    // buffer to receive reply 
					BUF_SIZE,  // size of buffer 
					&cbRead,  // number of bytes read 
					NULL);    // not overlapped 


				if (resSuccess != TRUE) {
					printf("Servidor: Erro na leitura do named pipe.\n");
					PrintErrorMsg();
					return -1;
				}


				printf("Servidor: Recebi >%s<\n", msg);
				if (strcmp(msg, "close") != 0 && strcmp(msg, "shutdown") != 0) {
					receiveCommand(msg, lpVoid);
				}

			}





		}


		DisconnectNamedPipe(hPipe);
		DisconnectNamedPipe(hPipe2);


	}



	CloseHandle(hPipe);
	CloseHandle(hPipe2);

	exit(0);
	return 0;
}

int main(int argc, char* argv[]) {
	setlocale(LC_ALL, "Portuguese");
	srand(time(NULL));
	NewGame();
	return 0;
}






void NewGame() {
	// Inicializa da Estrutura
	struct Data data;
	// Fim da inicialização
	GameEngine(&data); // Inicializa motor de jogo
}

void InitializePlayer(struct Player *pPlayer) {

	strcpy(pPlayer->name, "Jogador");
	pPlayer->cell = PLAYER_INIT_CELL;
	pPlayer->energy = MAX_ENGERGY;
	pPlayer->treasure = NO_TREASURE;
}

void InitializeMonster(struct Monster *pMonster) {

	strcpy(pMonster->name, MONSTER_NAME);
	pMonster->energy = MONSTER_MAX_ENGERGY;
	pMonster->cell = MONSTER_INIT_CELL;

}

void printMonster(struct Monster *pMonster) {

	printf("*********************Monster Info********************* \n");
	printf("*Monster 's name  -> %s\n", pMonster->name);
	printf("*Monster's energy -> %d\n", pMonster->energy);
	printf("*Monster's cell -> %d\n", pMonster->cell);
}

void printPlayer(struct Player *pPlayer) {
	printf("*********************Traveler Info********************* \n");
	printf("*Traveler's name  -> %s\n", pPlayer->name);
	printf("*Traveler's energy -> %d\n", pPlayer->energy);
	printf("*Traveler's cell -> %d\n", pPlayer->cell);
	if (pPlayer->treasure == 0) {
		printf("*Traveler has treasure ? -> No \n");
	}

	else {

		printf("*Traveler has treasure ? -> Yes \n");
	}

}

void GameEngine(struct Data *pdata) {

	InitializeGameItems(&pdata->player, &pdata->map, &pdata->monster); // Iniciliza os Itens do jogo:  mapa , jogador, monstro
	/* Criação dos mutex e threads*/
	hMutexPlayer = CreateMutex(
		NULL,                       // default security attributes
		FALSE,                      // initially not owned
		NULL);                      // unnamed mutex

	hMutexMonster = CreateMutex(
		NULL,                       // default security attributes
		FALSE,                      // initially not owned
		NULL);                      // unnamed mutex

	hMutexEcran = CreateMutex(
		NULL,                       // default security attributes
		FALSE,                      // initially not owned
		NULL);                      // unnamed mutex

	HANDLE hThread1 = CreateThread( // Movimento Monstro
		NULL,              // default security attributes
		0,                 // use default stack size  
		ThreadMonsterMov,
		(LPVOID)pdata,
		NULL,             // argument to thread function 
		0);   // returns the thread identifier 

	HANDLE hThread2 = CreateThread( // Movimento Jogador
		NULL,              // default security attributes
		0,                 // use default stack size  
		ThreadPlayerMov,
		(LPVOID)pdata,
		NULL,             // argument to thread function 
		0);   // returns the thread identifier 

	// utilizado para wait multiplo 
	ghEvents[0] = hMutexPlayer;
	ghEvents[1] = hMutexMonster;



	struct Data *pDados = pdata; // Estrutura com várias estruturas lá dentro

	struct Player *pPlayer = &pDados->player;
	struct Monster *pMonster = &pDados->monster;

	while (strcmp(msg, "shutdown") != 0) {

		while (pPlayer->energy > 0 && pMonster->energy > 0) {


			if (pPlayer->cell == pMonster->cell) {

				DWORD dwEvent = WaitForMultipleObjects( // Parar/Trancar os 2 Mutex ao mesmo tempo
					2,           // number of objects in array
					ghEvents,     // array of objects
					TRUE,       // wait for any object
					INFINITE);

				//WaitForSingleObject(hMutexPlayer, INFINITE);
				//WaitForSingleObject(hMutexMonster, INFINITE);

				//char InfMutexEnd2[256];
				//sprintf(InfMutexEnd2, "-----------------Parar/Trancar os 2 Mutex ao mesmo tempo \n");
				//PrintToConsole(InfMutexEnd2);


				combat = 1;
				InitializeCombat(pPlayer, pMonster); // Se os 2 estiverem na mesma célula	
				combat = 0;

				ReleaseMutex(hMutexMonster);
				ReleaseMutex(hMutexPlayer);

			}

			

			//Sleep(500);
		}



	}

	//WaitForSingleObject(hThread2, INFINITE);
	//WaitForSingleObject(hThread1, INFINITE);
	/* Destuição dos mutex e threads*/
	CloseHandle(hThread2);
	CloseHandle(hThread1);
	CloseHandle(hMutexPlayer);
	CloseHandle(hMutexMonster);
	CloseHandle(hMutexEcran);

}


void InitializeCombat(struct Player *pPlayer, struct Monster *pMonster) {



	char text[MAX_MSG]; // acrescetado à 2º parte
	char message[MAX_MSG];
	strcpy(message, "begin_combat");
	sendMessageToPipe2(message);
	strcpy(message, "");

	if ((pPlayer->cell == pMonster->cell) && (pMonster->energy > 0) && (pPlayer->energy > 0)) {



		sprintf(message, "\n\nThe %s apperead Let the Combat begin !!!\n", pMonster->name);
		sendMessageToPipe2(message);


		while ((pMonster->energy > 0) && (pPlayer->energy > 0)) {


			int monsterDamage = rand() % 3;
			int playerDamage = rand() % 3;


			if ((rand() % 2) == 1) { // Ataca o Jogador

				pMonster->energy = pMonster->energy - playerDamage;
				//printf("%s attack -> %d  \n%s energy -> %d ", pPlayer->name, playerDamage, pMonster->name, pMonster->energy);
				sprintf(message, "%s attack -> %d  \n%s energy -> %d ", pPlayer->name, playerDamage, pMonster->name, pMonster->energy);
				sendMessageToPipe2(message);
				strcpy(message, "");

				if (pMonster->energy <= 0) {

					printf("\nYou won the batlle! \n");
					strcpy(message, "");
					sprintf(message, "\nYou won the batlle! \n");
					sendMessageToPipe2(message);
					strcpy(message, "");

					sprintf(text, "Type w , e , s , n to move>");
					sendMessageToPipe2(text);
					//PrintToConsole(text);

				}
			}
			else
			{ // Ataca Monstro
				strcpy(message, "");
				pPlayer->energy = pPlayer->energy - monsterDamage;
				//printf("%s attack ->  %d  \n%s energy -> %d  ", pMonster->name, monsterDamage, pPlayer->name, pPlayer->energy);
				sprintf(message, "%s attack ->  %d  \n%s energy -> %d  ", pMonster->name, monsterDamage, pPlayer->name, pPlayer->energy);
				sendMessageToPipe2(message);
				strcpy(message, "");

				if (pPlayer->energy <= 0) {

					//printf("You died !\n");
					strcpy(message, "");
					sprintf(message, "You died !\nThe game will shutdown \n");
					sendMessageToPipe2(message);
					/*desligar o cliente */
					strcpy(message, "");




				}

			}

			Sleep(1000);
		}

	}

	strcpy(message, "end_combat");
	sendMessageToPipe2(message);
	strcpy(message, "");


	if (pPlayer->energy <= 0) {

		strcpy(message, "dead");
		sendMessageToPipe2(message);
		strcpy(message, "");

	}
	else {
		strcpy(message, "win");
		sendMessageToPipe2(message);
		strcpy(message, "");
	}





}


void LockedDoors() {

	char doorsSmg[100];
	int randInt = rand() % 3; // 0 - 2

	switch (randInt)
	{
	case 0:
		//printf("That door is locked !\n");
		sprintf(doorsSmg, "That door is locked !\n");
		sendMessageToPipe(doorsSmg);
		break;
	case 1:
		//printf("Cannot go that way!\n");
		sprintf(doorsSmg, "Cannot go that way!\n");
		sendMessageToPipe(doorsSmg);
		break;
	case 2:
		//printf("I sense a dark force that way ... Not going ...\n");
		sprintf(doorsSmg, "I sense a dark force that way ... Not going ...\n");
		sendMessageToPipe(doorsSmg);
		break;
	default:
		//printf("Invalid path\n");
		sprintf(doorsSmg, "Invalid path\n");
		sendMessageToPipe(doorsSmg);
		break;
	}

}







void InitializeMap(struct Map *pMap) { // Caracteristicas dos mapas
	pMap->nCells = 8;

	//Cell 0
	pMap->cells[0].north = -1;
	pMap->cells[0].south = -1;
	pMap->cells[0].west = -1;
	pMap->cells[0].east = 1;
	pMap->cells[0].treasure = NO_TREASURE;
	pMap->cells[0].cookie = NO_COOKIE;
	strcpy(pMap->cells[0].description, CELL_0_DESCRIPTION);

	//Cell 1

	pMap->cells[1].north = -1;
	pMap->cells[1].south = 2;
	pMap->cells[1].west = 0;
	pMap->cells[1].east = -1;
	pMap->cells[1].treasure = NO_TREASURE;
	pMap->cells[1].cookie = NO_COOKIE;
	strcpy(pMap->cells[1].description, CELL_1_DESCRIPTION);

	//Cell 2

	pMap->cells[2].north = 1;
	pMap->cells[2].south = 8;
	pMap->cells[2].west = -1;
	pMap->cells[2].east = 3;
	pMap->cells[2].treasure = NO_TREASURE;
	pMap->cells[2].cookie = NO_COOKIE;
	strcpy(pMap->cells[2].description, CELL_2_DESCRIPTION);

	//Cell 3

	pMap->cells[3].north = 4;
	pMap->cells[3].south = -1;
	pMap->cells[3].west = 2;
	pMap->cells[3].east = -1;
	pMap->cells[3].treasure = NO_TREASURE;
	pMap->cells[3].cookie = NO_COOKIE;
	strcpy(pMap->cells[3].description, CELL_3_DESCRIPTION);

	//Cell 4

	pMap->cells[4].north = -1;
	pMap->cells[4].south = 3;
	pMap->cells[4].west = -1;
	pMap->cells[4].east = 5;
	pMap->cells[4].treasure = NO_TREASURE;
	pMap->cells[4].cookie = NO_COOKIE;
	strcpy(pMap->cells[4].description, CELL_4_DESCRIPTION);

	//Cell 5

	pMap->cells[5].north = -1;
	pMap->cells[5].south = 6;
	pMap->cells[5].west = 4;
	pMap->cells[5].east = -1;
	pMap->cells[5].treasure = NO_TREASURE;
	pMap->cells[5].cookie = NO_COOKIE;
	strcpy(pMap->cells[5].description, CELL_5_DESCRIPTION);

	//Cell 6

	pMap->cells[6].north = 5;
	pMap->cells[6].south = -1;
	pMap->cells[6].west = -1;
	pMap->cells[6].east = 7;
	pMap->cells[6].treasure = NO_TREASURE;
	pMap->cells[6].cookie = NO_COOKIE;
	strcpy(pMap->cells[6].description, CELL_6_DESCRIPTION);

	//Cell 7

	pMap->cells[7].north = -1;
	pMap->cells[7].south = -1;
	pMap->cells[7].west = 6;
	pMap->cells[7].east = -1;
	pMap->cells[7].treasure = TREASURE;
	pMap->cells[7].cookie = NO_COOKIE;
	strcpy(pMap->cells[7].description, CELL_7_DESCRIPTION);

	// Cell 8

	pMap->cells[8].north = 2;
	pMap->cells[8].south = -1;
	pMap->cells[8].west = -1;
	pMap->cells[8].east = -1;
	pMap->cells[8].treasure = NO_TREASURE;
	pMap->cells[8].cookie = COOKIE;
	strcpy(pMap->cells[8].description, CELL_8_DESCRIPTION);


}


void PrintMap(struct Map *pMap) {

	printf("*********************Map Info********************* \n");

	for (int i = 0; i < pMap->nCells; i++)
	{

		printf("Cell %d -> Treasure  %d -> %s \n", i, pMap->cells[i].treasure, pMap->cells[i].description);

	}
}

void InitializeGameItems(struct Player *pPlayer, struct Map *pMap, struct Monster *pMonster) {
	end_game = 0;
	InitializePlayer(pPlayer); // Inicializa o Jogador
	InitializeMonster(pMonster); // Inicializa o monstro
	//InitializeMapFTXT(pMap);

	InitializeMap(pMap);

	//printPlayer(pPlayer); // Mostra detalhes sobre o Jogador
	//printMonster(pMonster); // Mostra detalhes sobre o Monstro
	//PrintMap(pMap);//Mostra detalhes do mapa
	//printf("******************************\n");

	//cout << ("%s \n%s \n", "The game is about to begin", pMap->cells[pPlayer->cell].description) << endl;

	//printf("******************************\n");
}






