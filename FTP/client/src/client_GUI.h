#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_client_GUI.h"

#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>
#include <memory.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string>

#include <winsock2.h>
#include <WinBase.h>
#include <ws2tcpip.h>
#include <Windows.h>

#include <qdebug.h>
#include <qstring.h>
#include <qlabel.h>
#include <qdirmodel.h>
#include <qfileinfo.h>
#include <qsortfilterproxymodel.h>

#pragma comment(lib, "ws2_32.lib")

#define MAXCLIENT 30

typedef struct connect_info
{
	char ip_address[20];
	int port;
}connect_info;


class client_GUI : public QMainWindow
{
    Q_OBJECT

public:
    client_GUI(QWidget *parent = Q_NULLPTR);

private:
    Ui::client_GUIClass ui;
	
	WSADATA wData;
	SOCKET m_sockfd, sockfd;
	int listenfd, connfd;
	char sentence[8192];
	int len;
	int n, p;
	connect_info info;
	int PASV_mode = -1;
	FILE* fp;
	char m_sentence[8192];
	char file_contents[8192];
	char root[128] = "downloads";
	std::string ip_address;
	bool is_rn = false;

	void client_GUI::emit_cmd(char* str, int num);

private slots:
    void SIGN_IN_clicked();
    void ABOR();
	void PASV();
	void PORT();
	void REFRESH();
	void STOR(QListWidgetItem* item);
	void RETR(QListWidgetItem* item);
	void RN();
	void PWD();
	void CWD(std::string dir);
	void MKD();
	void RMD();
	void SYST();
	void TYPE();
};

int connect_sock(const char* ip_address, int port_num);
int listen_sock(int port_num);
void divide_port(char* str, int* port, int n);