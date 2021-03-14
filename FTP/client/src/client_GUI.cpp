#include "client_GUI.h"


client_GUI::client_GUI(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
	//ui.centralWidget->setCurrentWidget(this);
    connect(ui.pushButton, SIGNAL(clicked()), this, SLOT(SIGN_IN_clicked()));
	connect(ui.quitButton, SIGNAL(clicked()), this, SLOT(ABOR()));
	connect(ui.pasvButton, SIGNAL(clicked()), this, SLOT(PASV()));
	connect(ui.portButton, SIGNAL(clicked()), this, SLOT(PORT()));
	connect(ui.refreshButton, SIGNAL(clicked()), this, SLOT(REFRESH()));
	connect(ui.clientList, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(STOR(QListWidgetItem*)));
	connect(ui.serverList, SIGNAL(itemDoubleClicked(QListWidgetItem *)), this, SLOT(RETR(QListWidgetItem*)));
	connect(ui.mkdButton, SIGNAL(clicked()), this, SLOT(MKD()));
	connect(ui.rmdButton, SIGNAL(clicked()), this, SLOT(RMD()));
	connect(ui.rnButton, SIGNAL(clicked()), this, SLOT(RN()));
	connect(ui.systButton, SIGNAL(clicked()), this, SLOT(SYST()));
	connect(ui.typeButton, SIGNAL(clicked()), this, SLOT(TYPE()));

	ui.lineEdit->setText("192.168.55.73");
	ui.lineEdit_2->setText("5000");
	ui.lineEdit_3->setText("anonymous");
	ui.lineEdit_4->setText("anonymous@");

	/*ui.lineEdit->setText("166.111.80.127");
	ui.lineEdit_2->setText("40121");
	ui.lineEdit_3->setText("ftp2020");
	ui.lineEdit_4->setText("ftp2020");*/
	ui.stackedWidget->setCurrentIndex(3);

}

void client_GUI::SIGN_IN_clicked()
{
	ip_address = ui.lineEdit->text().toStdString();
	const char* input_ip = ip_address.c_str();
	int input_port = atoi(ui.lineEdit_2->text().toLatin1().data());

	if (WSAStartup(MAKEWORD(2, 2), &wData) != 0)
	{
		printf("Init Windows Socket Failed! Error: %d\n", GetLastError());
		getchar();
	}
	sockfd = connect_sock(input_ip, input_port);
	p = recv(sockfd, sentence, 8192, 0);
	if (!strncmp(sentence, "220 ", 4)) {
		ui.textEdit->setText(sentence);
		std::string temp = ui.lineEdit_3->text().toStdString();
		std::string temp_user = "USER " + ui.lineEdit_3->text().toStdString();
		const char* user2 = temp_user.c_str();
		memset(sentence, 0, sizeof(sentence));
		strcpy(sentence, user2);
		len = strlen(sentence);
		sentence[len] = '\r';
		sentence[len + 1] = '\n';
		sentence[len + 2] = '\0';
		len = strlen(sentence);
		n = send(sockfd, sentence, len, 0);
		p = recv(sockfd, sentence, 8192, 0);
		if (!strncmp(sentence, "331 ", 4)) {
			ui.textEdit->setText(sentence);
			temp = ui.lineEdit_4->text().toStdString();
			std::string temp_pwd = "PASS " + ui.lineEdit_4->text().toStdString();
			const char* pwd2 = temp_pwd.c_str();
			memset(sentence, 0, sizeof(sentence));
			strcpy(sentence, pwd2);
			len = strlen(sentence);
			sentence[len] = '\r';
			sentence[len + 1] = '\n';
			sentence[len + 2] = '\0';
			len = strlen(sentence);
			n = send(sockfd, sentence, len, 0);
			p = recv(sockfd, sentence, 8192, 0);
			if (!strncmp(sentence, "230 ", 4)) {
				ui.stackedWidget->setCurrentIndex(1);
				ui.cmd->setText(sentence);
				PWD();
				REFRESH();
			}
			else {
				ui.textEdit->setText("WRONG PASSWORD");
			}
		}
		else {
			ui.textEdit->setText("WRONG USER");
		}
	}
	else {
		ui.textEdit->setText("WRONG SERVER");
	}
}

void client_GUI::PORT() {
	PASV_mode = 0;
	int port1 = rand() % 400 + 100;
	int port2 = rand() % 1000;
	strcpy(info.ip_address, "192.168.55.250");
	info.port = port1 * 256 + port2;
	memset(sentence, 0, sizeof(sentence));
	sprintf(sentence, "PORT 192,168,55,250,%d,%d", port1, port2);
	len = strlen(sentence);
	sentence[len] = '\r';
	sentence[len + 1] = '\n';
	sentence[len + 2] = '\0';
	len = strlen(sentence);
	n = send(sockfd, sentence, len, 0);
	p = recv(sockfd, sentence, 8192, 0);
	ui.cmd->setText(sentence);
}

void client_GUI::PASV() {
	PASV_mode = 1;
	memset(sentence, 0, sizeof(sentence));
	strcpy(sentence, "PASV");
	len = strlen(sentence);
	sentence[len] = '\r';
	sentence[len + 1] = '\n';
	sentence[len + 2] = '\0';
	len = strlen(sentence);
	n = send(sockfd, sentence, len, 0);
	p = recv(sockfd, sentence, 8191, 0);
	if (!strncmp(sentence, "227", 3)) {
		memset(info.ip_address, 0, sizeof(info.ip_address));
		ui.cmd->setText(sentence);
		char ip[3];
		int port[6];
		emit_cmd(sentence, 26);
		divide_port(sentence, port, 1);
		for (int i = 0; i < 3; i++) {
			sprintf(ip, "%d", port[i]);
			strcat(info.ip_address, ip);
			strcat(info.ip_address, ".");
			memset(ip, 0, sizeof(ip));
		}
		sprintf(ip, "%d", port[3]);
		strcat(info.ip_address, ip);

		//for zhujiaoserver
		//memset(info.ip_address, 0, sizeof(info.ip_address));
		//strcpy(info.ip_address, "166.111.80.127");
		PASV_mode = 1;
		info.port = port[4] * 256 + port[5];
	}
}

void client_GUI::ABOR() {
	memset(sentence, 0, sizeof(sentence));
	strcpy(sentence, "QUIT");
	len = strlen(sentence);
	sentence[len] = '\r';
	sentence[len + 1] = '\n';
	sentence[len + 2] = '\0';
	len = strlen(sentence);
	n = send(sockfd, sentence, len, 0);
	p = recv(sockfd, sentence, 8192, 0);
	ui.cmd->setText(sentence);
	closesocket(sockfd);
	ui.stackedWidget->setCurrentIndex(3);
}

void client_GUI::REFRESH() {

	//server Refresh
	
	ui.serverList->clear();
	PASV();
	memset(sentence, 0, sizeof(sentence));
	strcpy(sentence, "LIST");
	len = strlen(sentence);
	sentence[len] = '\r';
	sentence[len + 1] = '\n';
	sentence[len + 2] = '\0';
	len = strlen(sentence);
	n = send(sockfd, sentence, len, 0);
	m_sockfd = connect_sock(info.ip_address, info.port);
	memset(sentence, 0, sizeof(sentence));
	n = recv(sockfd, sentence, 8191, 0);
	sentence[n] = 0;
	ui.cmd->setText(sentence);

	char* line = (char*)malloc(256);
	char* filename = (char*)malloc(256);
	if (!strncmp(sentence, "150", 3)) {
		ui.serverList->addItem(QString(".."));
		while (1) {
			memset(sentence, 0, sizeof(sentence));
			n = recv(m_sockfd, sentence, 8191, 0);
			if (n == 0) {
				break;
			}
			line = strtok(sentence, "\n");
			while (line != NULL) {
				if (line[0] == '.') {
					line = strtok(NULL, "\n");
					continue;
				}
				ui.serverList->addItem(QString(line));
				line = strtok(NULL, "\n");
			}
		}

		closesocket(m_sockfd);
		memset(sentence, 0, sizeof(sentence));
	}


	n = recv(sockfd, sentence, 8191, 0);
	sentence[n] = 0;
	ui.cmd->setText(sentence);

	//client refresh
	ui.clientList->clear();
	QDir dir(root);

	QFileInfoList list = dir.entryInfoList();
	for (int i = 0; i < list.size(); ++i) {
		QFileInfo fileInfo = list.at(i);
		if (fileInfo.fileName().at(0) == '.') continue;
		ui.clientList->addItem(fileInfo.fileName());
	}
	PASV_mode = -1;
}

void client_GUI::STOR(QListWidgetItem* item) {

	memset(sentence, 0, sizeof(sentence));
	strcpy(sentence, "STOR ");
	strcat(sentence, item->text().toStdString().c_str());
	len = strlen(sentence);
	sentence[len] = '\r';
	sentence[len + 1] = '\n';
	sentence[len + 2] = '\0';
	len = strlen(sentence);
	n = send(sockfd, sentence, len, 0);
	if (PASV_mode == -1) {
		p = recv(sockfd, sentence, 8191, 0);
		ui.cmd->setText(sentence);
	}
	else if (PASV_mode == 0) {
		listenfd = listen_sock(info.port);
		if ((connfd = accept(listenfd, NULL, NULL)) == -1) {
			printf("Error accept(): %s(%d)\n", strerror(errno), errno);
		}
		n = recv(sockfd, m_sentence, 8191, 0);
		ui.cmd->setText(m_sentence);
		emit_cmd(sentence, 5);
		sentence[strlen(sentence) - 2] = 0;
		char filename[100] = "downloads/";
		strcat(filename, sentence);
		fp = fopen(filename, "rb");
		while (!feof(fp)) {
			len = fread(file_contents, 1, sizeof(file_contents), fp);
			n = send(connfd, file_contents, len, 0);
		}
		fclose(fp);
		closesocket(listenfd);
		closesocket(connfd);
	}
	else {
		m_sockfd = connect_sock(info.ip_address, info.port);
		n = recv(sockfd, m_sentence, 8191, 0);
		ui.cmd->setText(m_sentence);
		emit_cmd(sentence, 5);
		sentence[strlen(sentence) - 2] = 0;
		char filename[100] = "downloads/";
		strcat(filename, sentence);
		fp = fopen(filename, "rb");
		while (!feof(fp)) {
			len = fread(file_contents, 1, sizeof(file_contents), fp);
			n = send(m_sockfd, file_contents, len, 0);
		}
		fclose(fp);
		closesocket(m_sockfd);
	}
	n = recv(sockfd, m_sentence, 8191, 0);
	ui.cmd->setText(sentence);
	PASV_mode = -1;
	REFRESH();
}

void client_GUI::RETR(QListWidgetItem* item) {

	if (is_rn) {
		std::string tmp = item->text().toStdString();
		std::string new_name = ui.inputEdit->text().toStdString();
		strcpy(sentence, "RNFR ");
		std::string tmp2 = tmp.substr(tmp.find_last_of(' ') + 1);
		strcat(sentence, tmp2.c_str());
		len = strlen(sentence);
		sentence[len] = '\r';
		sentence[len + 1] = '\n';
		sentence[len + 2] = '\0';
		len = strlen(sentence);
		n = send(sockfd, sentence, len, 0);
		p = recv(sockfd, sentence, 8191, 0);

		strcpy(sentence, "RNTO ");
		strcat(sentence, new_name.c_str());
		len = strlen(sentence);
		sentence[len] = '\r';
		sentence[len + 1] = '\n';
		sentence[len + 2] = '\0';
		len = strlen(sentence);
		n = send(sockfd, sentence, len, 0);
		p = recv(sockfd, sentence, 8191, 0);
		REFRESH();
		is_rn = false;
		return;
	}
	std::string tmp = item->text().toStdString();
	if (tmp[0] == 'd' || tmp[0] == '.') {
		CWD(tmp);
		return;
	}
	memset(sentence, 0, sizeof(sentence));
	strcpy(sentence, "RETR ");
	std::string tmp2 = tmp.substr(tmp.find_last_of(' ') + 1);
	strcat(sentence, tmp2.c_str());
	len = strlen(sentence);
	sentence[len] = '\n';
	sentence[len + 1] = '\0';
	len = strlen(sentence);
	n = send(sockfd, sentence, len, 0);
	if (PASV_mode == -1) {
		p = recv(sockfd, sentence, 8191, 0);
		ui.cmd->setText(sentence);
	}
	else if (PASV_mode == 0) {
		listenfd = listen_sock(info.port);
		if ((connfd = accept(listenfd, NULL, NULL)) == -1) {
			printf("Error accept(): %s(%d)\n", strerror(errno), errno);
		}
		n = recv(sockfd, m_sentence, 8191, 0);
		ui.cmd->setText(m_sentence);
		emit_cmd(sentence, 5);
		sentence[tmp2.size() - 1] = 0;
		char filename[100] = "downloads/";
		strcat(filename, sentence);
		fp = fopen(filename, "wb");
		while (1) {
			n = recv(connfd, file_contents, sizeof(file_contents), 0);
			if (n == 0) {
				break;
			}
			fwrite(file_contents, 1, n, fp);
		}
		fclose(fp);
		closesocket(listenfd);
		closesocket(connfd);
	}
	else {
			m_sockfd = connect_sock(info.ip_address, info.port);
			n = recv(sockfd, m_sentence, 8191, 0);
			ui.cmd->setText(m_sentence);
			emit_cmd(sentence, 5);
			sentence[tmp2.size() - 1] = 0;
			char filename[100] = "downloads/";
			strcat(filename, sentence);
			fp = fopen(filename, "wb");
			while (1) {
				n = recv(m_sockfd, file_contents, sizeof(file_contents), 0);
				if (n == 0) {
					break;
				}
				fwrite(file_contents, 1, n, fp);
			}
			fclose(fp);
			closesocket(m_sockfd);
	}
	n = recv(sockfd, m_sentence, 8191, 0);
	ui.cmd->setText(sentence);
	PASV_mode = -1;
	REFRESH();
}

void client_GUI::RN() {
	is_rn = true;
}

void client_GUI::emit_cmd(char* str, int num) {
	char temp[8192] = { 0, };
	for (int i = 0; i < strlen(str) - num; i++) {
		temp[i] = str[i + num];
		if (temp[i] == 0)
			break;
	}
	strcpy(str, temp);
}

void client_GUI::CWD(std::string dir) {
	memset(sentence, 0, sizeof(sentence));
	strcpy(sentence, "CWD ");
	std::string tmp2 = dir.substr(dir.find_last_of(' ') + 1);
	strcat(sentence, tmp2.c_str());
	len = strlen(sentence);
	sentence[len] = '\n';
	sentence[len + 1] = '\0';
	len = strlen(sentence);
	n = send(sockfd, sentence, len, 0);
	p = recv(sockfd, sentence, 8192, 0);
	PWD();
	REFRESH();
}

void client_GUI::PWD() {
	memset(sentence, 0, sizeof(sentence));
	strcpy(sentence, "PWD");
	len = strlen(sentence);
	sentence[len] = '\r';
	sentence[len + 1] = '\n';
	sentence[len + 2] = '\0';
	len = strlen(sentence);
	n = send(sockfd, sentence, len, 0);
	p = recv(sockfd, sentence, 8192, 0);
	sentence[p] = 0;
	emit_cmd(sentence, 4);
	ui.address->setText(sentence);
}

void client_GUI::MKD() {
	std::string dir = ui.inputEdit->text().toStdString();
	memset(sentence, 0, sizeof(sentence));
	strcpy(sentence, "MKD ");
	strcat(sentence, dir.c_str());
	len = strlen(sentence);
	sentence[len] = '\r';
	sentence[len + 1] = '\n';
	sentence[len + 2] = '\0';
	len = strlen(sentence);
	n = send(sockfd, sentence, len, 0);
	p = recv(sockfd, sentence, 8191, 0);
	ui.cmd->setText(sentence);
	REFRESH();
}

void client_GUI::RMD() {
	std::string dir = ui.inputEdit->text().toStdString();
	memset(sentence, 0, sizeof(sentence));
	strcpy(sentence, "RMD ");
	strcat(sentence, dir.c_str());
	len = strlen(sentence);
	sentence[len] = '\r';
	sentence[len + 1] = '\n';
	sentence[len + 2] = '\0';
	len = strlen(sentence);
	n = send(sockfd, sentence, len, 0);
	p = recv(sockfd, sentence, 8191, 0);
	ui.cmd->setText(sentence);
	REFRESH();
}

void client_GUI::SYST() {
	memset(sentence, 0, sizeof(sentence));
	strcpy(sentence, "SYST");
	len = strlen(sentence);
	sentence[len] = '\r';
	sentence[len + 1] = '\n';
	sentence[len + 2] = '\0';
	len = strlen(sentence);
	n = send(sockfd, sentence, len, 0);
	p = recv(sockfd, sentence, 8192, 0);
	sentence[p] = 0;
	ui.cmd->setText(sentence);
}

void client_GUI::TYPE() {
	memset(sentence, 0, sizeof(sentence));
	strcpy(sentence, "TYPE I");
	len = strlen(sentence);
	sentence[len] = '\r';
	sentence[len + 1] = '\n';
	sentence[len + 2] = '\0';
	len = strlen(sentence);
	n = send(sockfd, sentence, len, 0);
	p = recv(sockfd, sentence, 8192, 0);
	sentence[p] = 0;
	ui.cmd->setText(sentence);
}

//connect & listen
int connect_sock(const char* ip_address, int port_num) {
	SOCKET sockfd;
	struct sockaddr_in addr;
	if ((sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
		printf("Error socket(): %s(%d)\n", strerror(errno), errno);
		return 1;
	}
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port_num);
	addr.sin_addr.s_addr = inet_addr(ip_address);
	memset(addr.sin_zero, 0X00, 8);
	WSAAsyncSelect(sockfd, 0, 0, FD_READ | FD_WRITE | FD_CLOSE);
	//Sleep(1);
	if (connect(sockfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
		printf("Error bind(): %s(%d)\n", strerror(errno), errno);
		qDebug() << strerror(errno) << errno;
		return 1;
	}
	return sockfd;
}

int listen_sock(int port_num) {
	int listenfd;
	struct sockaddr_in addr;
	if ((listenfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
	{
		printf("Error socket(): %s(%d)\n", strerror(errno), errno);
		return 1;
	}
	if (listenfd == -1) {
		return 1;
	}

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port_num);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(listenfd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
		printf("Error bind(): %s(%d)\n", strerror(errno), errno);
		return 1;
	}

	if (listen(listenfd, 10) == -1) {
		printf("Error listen(): %s(%d)\n", strerror(errno), errno);
		return 1;
	}
	return listenfd;
}

void divide_port(char* str, int* port, int n) {
	char temp[10];
	int cnt = 0;
	int j = 0;
	int flag = 0;
	int k = 0;
	for (int i = n; i < strlen(str); i++) {
		if (str[i] != ',' && str[i] > 0) {
			temp[j] = str[i];
			j++;
		}
		else if (str[i] == ',') {
			port[cnt] = atoi(temp);
			cnt++;
			j = 0;
			memset(temp, 0, sizeof(temp));
		}
		else if (str[i] < 0) {
			break;
		}
	}
	port[cnt] = atoi(temp);
}