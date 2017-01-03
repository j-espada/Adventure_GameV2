/*
* Jogo de Aventuras | Cliente
*
*  Actualizado a: 17/01/2016
*  Autores: Joaquim Espada e João Orvalho
*/



#include "stdafx.h"
#include <windows.h>
#include <stdio.h>
#include "locale.h"


#define PIPE_NAME "\\\\.\\pipe\\app1_pipe" //cliente e servidor na mesma máquina 
#define PIPE_NAME2 "\\\\.\\pipe\\app2_pipe" //cliente e servidor na mesma máquina 
//#define PIPE_NAME "\\\\LabSI2-PC1\\pipe\\app1_pipe" //servidor na máquina LabSI2-PC1
//#define PIPE_NAME "\\\\Bit4\\pipe\\app1_pipe" //servidor na máquina Bit4
#define BUF_SIZE 512
#define MAX_MSG 512

HANDLE hPipe;
HANDLE hPipe2;

//Solicitação de uma mensagem ao utilizador
char msg_in[MAX_MSG];
char msg_in2[MAX_MSG];
char msg_out[MAX_MSG];
char msg_out2[MAX_MSG];

/*Prototipos*/
int WriteFile();
DWORD WINAPI ThreadBattleInfo(LPVOID lpParam);


int exit_app = 0;
int error_app = 0;

void Exit() {
	if (exit_app == 0) {
		exit_app = 1;
		printf("O seu jogo terminou!\nO seu jogo irá terminar dentro de momentos...\n");
		Sleep(3000);

		//Fecho da ligação ao named pipe
		CloseHandle(hPipe);
		CloseHandle(hPipe2);

		//Fechar Thread
		CloseHandle(ThreadBattleInfo);
		exit(0);

	}
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


void PrintToConsole(char text[]) {

	printf("%s", text);
	fflush(stdout);
}


/*
Função que envia mensagens através do Pipe 2
*/
int WriteFile2() {

	DWORD cbWritten;
	BOOL resSuccess = FALSE;
	resSuccess = WriteFile(
		hPipe2,        // handle to pipe 
		msg_out2,     // buffer to write from 
		MAX_MSG, // number of bytes to write 
		&cbWritten,   // number of bytes written 
		NULL);        // not overlapped I/O 


	if (!resSuccess)
	{
		printf("Cliente: Erro no envio.\n");
		PrintErrorMsg();
		return -1;
	}

}

/*
Função que lê 2º pipe
*/
int ReadFile2() {

	DWORD cbRead;
	BOOL resSuccess = ReadFile(
		hPipe2,    // pipe handle 
		msg_in2,    // buffer to receive reply 
		BUF_SIZE,  // size of buffer 
		&cbRead,  // number of bytes read 
		NULL);    // not overlapped 


	if (!resSuccess)
	{
		printf("Cliente: Erro na leitura (Pipe 2).\n");
		PrintErrorMsg();
		error_app = 1;
		return -1;
		
	}


}


DWORD WINAPI ThreadBattleInfo(LPVOID lpParam)
{
	/*
	DWORD cbRead;

	BOOL resSuccess2 = ReadFile(
		hPipe2,    // pipe handle 
		msg_in2,    // buffer to receive reply 
		BUF_SIZE,  // size of buffer 
		&cbRead,  // number of bytes read 
		NULL);    // not overlapped 


		if (!resSuccess2)
		{
			if ((strcmp(msg_out, "close") != 0 && strcmp(msg_out, "shutdown") != 0)) {
				printf("Cliente: Erro ao receber.\n");
				PrintErrorMsg();
			}
			return -1;
		}


	


	//printf(msg_in2);

	*/

	int sensor = 0;



	do {
		ReadFile2();
		if (strcmp(msg_in2, "end_combat") == 0) {
			sensor = 1;
		}
		else {
			if (strcmp(msg_in2, "begin_combat") != 0) {
				printf("%s\n", msg_in2);
			}
		}


	} while (sensor != 1 && error_app == 0);

	if (error_app == 0) {

		ReadFile2();
		if (strcmp(msg_in2, "dead") == 0) {
			sensor = 1;

			strcpy(msg_out, "close");
			WriteFile();
			//Exit();
			//Fecho da ligação ao named pipe
			CloseHandle(hPipe2);
			CloseHandle(hPipe);
			//Fechar Thread
			CloseHandle(ThreadBattleInfo);
			Exit();
		}

	}
	else {
		Exit();
	}

	return 0;
}

/*
Função que envia mensagens através do Pipe 1
*/
int WriteFile() {

	DWORD cbWritten;
	BOOL resSuccess = FALSE;
	resSuccess = WriteFile(
		hPipe,        // handle to pipe 
		msg_out,     // buffer to write from 
		MAX_MSG, // number of bytes to write 
		&cbWritten,   // number of bytes written 
		NULL);        // not overlapped I/O 


	if (!resSuccess)
	{
		printf("Cliente: Erro no envio.\n");
		PrintErrorMsg();
		return -1;
	}

}


/*
Função que lê mensagens do Pipe 1
*/
int ReadFile() {

	DWORD cbRead;
	BOOL resSuccess = ReadFile(
		hPipe,    // pipe handle 
		msg_in,    // buffer to receive reply 
		BUF_SIZE,  // size of buffer 
		&cbRead,  // number of bytes read 
		NULL);    // not overlapped 

	if (!resSuccess)
	{
		printf("Cliente: Erro na leitura (Pipe 1).\n");
		PrintErrorMsg();
		return -1;
	}

}

int InicializePipes() {
	// criação da thread que lê o combate

	HANDLE hThreadBattleInfo = CreateThread(
		NULL,              // default security attributes
		0,                 // use default stack size  
		ThreadBattleInfo,        // thread function 
		NULL,             // argument to thread function 
		0,                 // use default creation flags 
		NULL);   // returns the thread identifier 



				 //Ligação ao named pipe criado pelo servidor
	hPipe = CreateFile(
		PIPE_NAME,   		// pipe name 
		GENERIC_READ |  	// read and write access 
		GENERIC_WRITE,
		0,              	// no sharing 
		NULL,           	// default security attributes
		OPEN_EXISTING,  	// opens existing pipe 
		0,              	// default attributes 
		NULL);          	// no template file


	if (hPipe == INVALID_HANDLE_VALUE) {
		printf("Cliente: Erro na ligação ao named pipe.\n");
		PrintErrorMsg();
		return -1;
	}


	//Ligação ao named pipe2 criado pelo servidor
	hPipe2 = CreateFile(
		PIPE_NAME2,   		// pipe name 
		GENERIC_READ |  	// read and write access 
		GENERIC_WRITE,
		0,              	// no sharing 
		NULL,           	// default security attributes
		OPEN_EXISTING,  	// opens existing pipe 
		0,              	// default attributes 
		NULL);          	// no template file


	if (hPipe2 == INVALID_HANDLE_VALUE) {
		printf("Cliente: Erro na ligação ao named pipe 2.\n");
		PrintErrorMsg();
		return -1;
	}
}

int main(int argc, char* argv[])
{
	setlocale(LC_ALL, "Portuguese");


	InicializePipes();

	strcpy(msg_in, "");
	strcpy(msg_out, "");

	int sensor = 0;

	//Receber informação da celula actual (onde se inicia):
	ReadFile();
	printf("%s\n", msg_in);

	//Envio da mensagem ao servidor através do named pipe
	while ((strcmp(msg_out, "close") != 0 && strcmp(msg_out, "shutdown") != 0) && strcmp(msg_in, "Win!") != 0) {



		printf("Type w , e , s , n to move>");
		scanf("%s", msg_out);

		WriteFile();





		if ((strcmp(msg_out, "close") != 0 && strcmp(msg_out, "shutdown") != 0)) {
			do {

				if (exit_app == 0) {
					ReadFile();

					if (strcmp(msg_in, "lock") != 0 && strcmp(msg_in, "unlock") != 0) {
						printf("%s\n", msg_in);
					}

					if (strcmp(msg_in, "lock") == 0) {
						sensor = 1;
					}
					else if (strcmp(msg_in, "unlock") == 0 || strcmp(msg_in, "Win!") == 0) {
						sensor = 0;
					}
				}

			} while (sensor != 0);

		}


	}


	Exit();



	return 0;
}