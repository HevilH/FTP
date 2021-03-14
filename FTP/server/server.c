#include "ftp_header.h"

int main(int argc, char *argv[])
{
	int byte_transfered = 0;

	int port = 5000;
	int is_user_id = 0;
	int is_user_pwd = 0;
	int PASV_mode = -1;
	FILE *fp;
	int listenfd, connfd, sockfd;
	int m_listenfd, m_connfd;
	
	int len, n;
	char file_contents[8192];
	char m_sentence[8192];
	char sentence[8100];
	
	int id_num;
	user user_table[MAXCLIENT];
	char directory[50] = "/tmp";
	char origin_name[100];


	if (argc == 5)
	{
		port = atoi(argv[2]);
		strcpy(directory, argv[4]);
	}

	chdir(directory);

	for (int i = 0; i < MAXCLIENT; i++)
	{
		strcpy(user_table[i].id, "anonymous");
		strcpy(user_table[i].pwd, "anonymous@");
		user_table[i].is_working = 0;
	}

	listenfd = sock_listen(port);
	
	while (1)
	{
		if ((connfd = accept(listenfd, NULL, NULL)) == -1)
		{
			printf("Error accept(): %s(%d)\n", strerror(errno), errno);
			continue;
		}

		for (int i = 0; i < MAXCLIENT; i++)
		{
			if (waitpid(user_table[i].pid, NULL, WNOHANG))
			{
				user_table[i].is_working = 0;
			}
		}

		for (int i = 0; i < MAXCLIENT; i++)
		{
			if (!user_table[i].is_working)
			{
				user_table[i].is_working = 1;
				id_num = i;
				break;
			}
			if (i == MAXCLIENT - 1)
			{
				printf("Server is busy\n");
				id_num = -1;
				close(listenfd);
			}
		}
		
		strcpy(sentence, "220 Anonymous FTP server ready\r\n");
		
		len = strlen(sentence);
		write(connfd, sentence, len);
		user_table[id_num].pid = fork();

		if (user_table[id_num].pid == 0)
		{
			while (1)
			{
				int p = 0;
				while (1)
				{
					n = read(connfd, sentence, 8191);
					if (n < 0)
					{
						printf("Error read(): %s(%d)\n", strerror(errno), errno);
						close(connfd);
						continue;
					}
					else
					{
						p += n;
						break;
					}
				}
				sentence[p - 2] = '\0';
				connect_info info;
				if (is_user_id && is_user_pwd)
				{
					if (!strcmp("SYST", sentence))
					{
						strcpy(sentence, "215 UNIX Type: L8");
					}
					else if (!strncmp("TYPE", sentence, 4))
					{
						if (!strncmp("TYPE I", sentence, 6))
						{
							strcpy(sentence, "200 Type set to I.");
						}
						else
						{
							strcpy(sentence, "504 Type dose not exist");
						}
					}
					else if (!strncmp("PORT ", sentence, 5))
					{

						memset(info.ip_address, 0, sizeof(info.ip_address));
						char ip[3];
						int m_port[6];
						emit_cmd(sentence, 5);
						divide_port(sentence, m_port, 0);
						for (int i = 0; i < 3; i++)
						{
							sprintf(ip, "%d", m_port[i]);
							strcat(info.ip_address, ip);
							strcat(info.ip_address, ".");
							memset(ip, 0, sizeof(ip));
						}
						sprintf(ip, "%d", m_port[3]);
						strcat(info.ip_address, ip);
						info.port = m_port[4] * 256 + m_port[5];
						PASV_mode = 0;
						strcpy(sentence, "200 PORT command successful.");
					}
					else if (!strcmp("PASV", sentence))
					{
						PASV_mode = 1;
						srand(time(NULL));
						int port1 = rand() % 400 + 100;
						int port2 = rand() % 1000;
						strcpy(info.ip_address, "192.168.55.73");
						info.port = port1 * 256 + port2;
						sprintf(sentence, "227 Entering Passive Mode (192,168,55,73,%d,%d)\r\n", port1, port2);
						len = strlen(sentence);
						write(connfd, sentence, len);
						m_listenfd = sock_listen(info.port);
						if ((m_connfd = accept(m_listenfd, NULL, NULL)) == -1)
						{
							printf("Error accept(): %s(%d)\n", strerror(errno), errno);
							continue;
						}
						continue;
					}
					else if (!strncmp("STOR ", sentence, 5))
					{
						if (!strcmp(sentence, "STOR "))
						{
							strcpy(sentence, "550 File dose not exist");
						}
						else
						{
							if (PASV_mode == 0)
							{
								sockfd = sock_connect(info.ip_address, info.port);
								sprintf(m_sentence, "150 Opening BINARY mode data connection for %s.\r\n", sentence);
								len = strlen(m_sentence);
								write(connfd, m_sentence, len);
								emit_cmd(sentence, 5);
								fp = fopen(sentence, "wb");
								while (1)
								{
									n = read(sockfd, file_contents, sizeof(file_contents));
									if (n == 0)
									{
										strcpy(sentence, "226 Transfer complete.");
										break;
									}
									fwrite(file_contents, 1, n, fp);
								}
								fclose(fp);
								close(sockfd);
							}
							else if (PASV_mode == 1)
							{
								sprintf(m_sentence, "150 Opening BINARY mode data connection for %s.\r\n", sentence);
								len = strlen(m_sentence);
								write(connfd, m_sentence, len);
								emit_cmd(sentence, 5);
								fp = fopen(sentence, "wb");
								while (1)
								{
									n = read(m_connfd, file_contents, sizeof(file_contents));
									if (n == 0)
									{
										strcpy(sentence, "226 Transfer complete.");
										break;
									}
									fwrite(file_contents, 1, n, fp);
								}
								fclose(fp);
								close(m_listenfd);
								close(m_connfd);
							}
							else
							{
								strcpy(sentence, "425 Please input PASV or PORT first.");
							}
						}
					}
					else if (!strncmp("RETR ", sentence, 5))
					{
						emit_cmd(sentence, 5);
						len = strlen(sentence);
						fp = fopen(sentence, "rb");
						if (fp == NULL)
						{
							strcpy(sentence, "550 file dose not exist");
						}
						else
						{
							fseek(fp, 0, SEEK_END);
							int file_size = ftell(fp);
							fseek(fp, 0, SEEK_SET);

							if (PASV_mode == 0)
							{
								sprintf(m_sentence, "150 Opening BINARY mode data connection for %s(%d bytes).\r\n", sentence, file_size);
								sockfd = sock_connect(info.ip_address, info.port);
								len = strlen(m_sentence);
								write(connfd, m_sentence, len);
								while (!feof(fp))
								{
									len = fread(file_contents, 1, sizeof(file_contents), fp);
									write(sockfd, file_contents, len);
								}
								byte_transfered += file_size;
								fclose(fp);
								strcpy(sentence, "226 Transfer complete.");
								close(sockfd);
							}
							else if (PASV_mode == 1)
							{
								sprintf(m_sentence, "150 Opening BINARY mode data connection for %s( %d bytes).\r\n", sentence, file_size);
								len = strlen(m_sentence);
								write(connfd, m_sentence, len);
								while (!feof(fp))
								{
									len = fread(file_contents, 1, sizeof(file_contents), fp);
									write(m_connfd, file_contents, len);
								}
								byte_transfered += file_size;
								strcpy(sentence, "226 Transfer complete.");
								fclose(fp);
								close(m_listenfd);
								close(m_connfd);
							}
							else
							{
								strcpy(sentence, "425 Please input PASV or PORT first.");
							}
							sleep(1);
						}
					}
					else if (!strcmp(sentence, "QUIT") || !strcmp(sentence, "ABOR"))
					{
						sprintf(sentence, "221 You have transferred %d bytes\n221 Thank you for using the FTP server.\n221 Goodbye.\r\n", byte_transfered);
						len = strlen(sentence);
						write(connfd, sentence, len);
						close(connfd);
						return 0;
					}
					else if (!strncmp("MKD", sentence, 3))
					{
						emit_cmd(sentence, 4);
						mkdir(sentence, S_IRWXU);
						strcpy(sentence, "257 directory is created");
					}
					else if (!strncmp("RMD", sentence, 3))
					{
						char buf[100];
						memset(buf, 0, sizeof(buf));
						emit_cmd(sentence, 4);
						strcpy(buf, sentence);
						rmdir(buf);
						strcpy(sentence, "250 directory is removed!");
					}
					else if (!strncmp("CWD", sentence, 3))
					{
						emit_cmd(sentence, 4);
						char buf[100] = {0,};
						getcwd(buf, sizeof(buf));
						if(!strcmp(buf, directory) && !strcmp(sentence, "..")){
							strcpy(sentence, "Not allowed");
						}
						else{
							chdir(sentence);
							strcpy(sentence, "250 Directory successfully changed.");
						}
					}
					else if (!strncmp("PWD", sentence, 3))
					{
						char buf[100];
						memset(sentence, 0, sizeof(sentence));
						getcwd(buf, sizeof(buf));
						sprintf(sentence, "257  %s", buf);
					}
					else if (!strncmp("RNFR", sentence, 4))
					{ //RNTO origin new
						emit_cmd(sentence, 5);
						memset(origin_name, 0, sizeof(origin_name));
						strcpy(origin_name, sentence);
						strcpy(sentence, "350 READY TO RNTO");
					}
					else if (!strncmp("RNTO", sentence, 4))
					{ //RNTO origin new
						emit_cmd(sentence, 5);
						char new_name[100];
						strcpy(new_name, sentence);
						rename(origin_name, new_name);
						memset(origin_name, 0, sizeof(origin_name));
						strcpy(sentence, "250 RENAME successful");
					}
					else if (!strncmp("LIST", sentence, 4))
					{
						strcpy(sentence, "150 directory ready\r\n");
						len = strlen(sentence);
						write(connfd, sentence, len);
						DIR *dir;
						struct dirent *dp;
						dir = opendir(".");
						if (PASV_mode == 0)
						{
							sockfd = sock_connect(info.ip_address, info.port);
						}
						while ((dp = readdir(dir)) != NULL)
						{
							memset(sentence, 0, sizeof(sentence));
							if(dp->d_type == 4){
							    sentence[0] = 'd';
							}
							else
							{
								sentence[0] = '-';
							}
							
							sentence[1] = ' ';
							strcat(sentence, dp->d_name);
							strcat(sentence, "\r\n");
							if (PASV_mode == 0)
							{
								len = strlen(sentence);
								write(sockfd, sentence, len);
							}
							else if (PASV_mode == 1)
							{
								len = strlen(sentence);
								write(m_connfd, sentence, len);
							}
						}
						if (PASV_mode == 0)
						{
							close(sockfd);
						}
						else if (PASV_mode == 1)
						{
							close(m_listenfd);
							close(m_connfd);
						}
						(void)closedir(dir);
						strcpy(sentence, "226 Transfer complete.");
					}

					else
					{
						strcpy(sentence, "500 Command dose not exist");
					}
				}
				else if (!is_user_id)
				{
					if (!strncmp(sentence, "USER ", 5))
					{
						emit_cmd(sentence, 5);
						if (!strcmp(sentence, user_table[id_num].id))
						{
							strcpy(sentence, "331 Guest login ok, send your complete e-mail address as password.");
							is_user_id = 1;
						}
						else
						{
							strcpy(sentence, "504 User dose not exist");
						}
					}
					else
					{
						strcpy(sentence, "500 Command dose not exist");
					}
				}
				else
				{
					if (!strncmp(sentence, "PASS ", 5))
					{
						emit_cmd(sentence, 5);
						if (!strcmp(sentence, user_table[id_num].pwd))
						{
							strcpy(sentence, "230 Welcome to server.");
							is_user_pwd = 1;
						}
						else
						{
							strcpy(sentence, "504 Password dose not exist");
						}
					}
					else
					{
						strcpy(sentence, "500 Command dose not exist");
					}
				}

				p = 0;
				len = strlen(sentence);
				sentence[len] = '\r';
				sentence[len + 1] = '\n';
				sentence[len + 2] = '\0';
				len = strlen(sentence);
				n = write(connfd, sentence, len);
				if (n < 0)
				{
					printf("Error write(): %s(%d)\n", strerror(errno), errno);
					return 1;
				}	
			}
		}
	}
	close(listenfd);
}
