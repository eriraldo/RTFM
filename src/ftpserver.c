#include	<time.h>
#include	<sys/types.h>
#include	<sys/socket.h>
#include	<strings.h>
#include	<string.h>
#include	<arpa/inet.h>
#include	<unistd.h>
#include	<stdio.h>
#include	<stdlib.h>
#include	<ctype.h>
#include	<netinet/in.h>
#include	<stdbool.h>
#include	<netdb.h>
#include	<errno.h>
#include	<fcntl.h>
#include	<time.h>
#include	<netinet/tcp.h>
#include	<sys/stat.h>
#include	<sys/types.h>
#include 	<dirent.h>
#include 	<pthread.h>


#define 	MAXLINE 	4096
#define		LISTENQ		1024
#define		TRUE		1
#define		FALSE		0

struct args {
    int port;
    int listenfd;
};
pthread_key_t glob_var_key;

struct dir_data{
	char *curr_dir;
	size_t curr_dir_len;
	char *var_dir;
	char *last_dir;
}struct_data;

char *curr_dir;
size_t curr_dir_len;
char *var_dir;
char *last_dir;

static bool pre_thread = false;
int number_Process = 0;
pthread_mutex_t new_client_mutex = PTHREAD_MUTEX_INITIALIZER;

//function trims leading and trailing whitespaces
void trim(char *str)
{

	int i;
    int begin = 0;

    int end = strlen(str) - 1;

    while (isspace((unsigned char) str[begin]))
        begin++;

    while ((end >= begin) && isspace((unsigned char) str[end]))
        end--;

    // Shift all characters back to the start of the string array.
    for (i = begin; i <= end; i++)
        str[i - begin] = str[i];

    str[i - begin] = '\0'; // Null terminate string.
}
//we need to separate the input from the client to know the client ip and be capable of sending data back
int get_client_ip_port(char *str, char *client_ip, int *client_port){
	char *n1, *n2, *n3, *n4, *n5, *n6;
	int x5, x6;

	strtok(str, " ");
	n1 = strtok(NULL, ",");
	n2 = strtok(NULL, ",");
	n3 = strtok(NULL, ",");
	n4 = strtok(NULL, ",");
	n5 = strtok(NULL, ",");
	n6 = strtok(NULL, ",");

	sprintf(client_ip, "%s.%s.%s.%s", n1, n2, n3, n4);

	x5 = atoi(n5);
	x6 = atoi(n6);
	*client_port = (256*x5)+x6;

	printf("client_ip: %s client_port: %d\n", client_ip, *client_port);
	return 1;
}

int setup_data_connection(int *fd, char *client_ip, int client_port, int server_port){
	
	struct sockaddr_in cliaddr, tempaddr;
	if ( (*fd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
    	perror("socket error");
    	return -1;
    }

	//bind port for data connection to be server port - 1 by using a temporary struct sockaddr_in
	bzero(&tempaddr, sizeof(tempaddr));
    tempaddr.sin_family = AF_INET;
    tempaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    tempaddr.sin_port   = htons(server_port-1);

    while((bind(*fd, (struct sockaddr*) &tempaddr, sizeof(tempaddr))) < 0){
    	//perror("bind error");
    	server_port--;
    	tempaddr.sin_port   = htons(server_port);
    }


	//initiate data connection fd with client ip and client port             
    bzero(&cliaddr, sizeof(cliaddr));
    cliaddr.sin_family = AF_INET;
    cliaddr.sin_port   = htons(client_port);
    if (inet_pton(AF_INET, client_ip, &cliaddr.sin_addr) <= 0){
    	perror("inet_pton error");
    	return -1;
    }

    if (connect(*fd, (struct sockaddr *) &cliaddr, sizeof(cliaddr)) < 0){
    	perror("connect error");
    	return -1;
    }

    return 1;
}

int get_filename(char *input, char *fileptr){

    char *filename = NULL;
    filename = strtok(input, " ");
    filename = strtok(NULL, " ");
    if(filename == NULL){
        return -1;
    }else{
    	strncpy(fileptr, filename, strlen(filename));
        return 1;
    }
}
//a kind of switch cases to validate which command the server recived
int get_command(char *command){
	char cpy[1024];
	strcpy(cpy, command);
	char *str = strtok(cpy, " ");
	int value;

	//populated value valriable to indicate back to main which input was entered
    if(strcmp(str, "LIST") == 0){value = 1;}
    else if(strcmp(str, "RETR") == 0){value = 2;}
    else if(strcmp(str, "STOR") == 0){value = 3;}
    else if(strcmp(str, "SKIP") == 0){value = 4;}
    else if(strcmp(str, "ABOR") == 0){value = 5;}
	else if(strcmp(str, "CD") == 0){value = 6;}
	else if(strcmp(str, "PWD") == 0){value = 7;}

    return value;
}
//function that lists the actual directory in which the server is
int do_list(int controlfd, int datafd, char *input){
	struct dir_data *struct_data = pthread_getspecific(glob_var_key);
	if(!struct_data){
		printf("pthread_getspecific failed\n");
		pthread_exit((void *)13);
	}
	chdir(struct_data->var_dir);
	char filelist[1024], sendline[MAXLINE+1], str[MAXLINE+1];
	bzero(filelist, (int)sizeof(filelist));

	if(get_filename(input, filelist) > 0){
		printf("Filelist Detected\n");
		sprintf(str, "ls %s", filelist);
		printf("Filelist: %s\n", filelist);
		trim(filelist);
    	DIR *dir = opendir(filelist);
    	if(!dir){
    		sprintf(sendline, "550 No Such File or Directory\n");
    		write(controlfd, sendline, strlen(sendline));
    		return -1;
    	}else{closedir(dir);}

	}else{
		sprintf(str, "ls");
	}

	 //initiate file pointer for popen()
    FILE *in;
    extern FILE *popen();

    if (!(in = popen(str, "r"))) {
    	sprintf(sendline, "451 Requested action aborted. Local error in processing\n");
    	write(controlfd, sendline, strlen(sendline));
        return -1;
    }

    while (fgets(sendline, MAXLINE, in) != NULL) {
        write(controlfd, sendline, strlen(sendline)); //DEBERIA SER EN DATAFD EN LUGAR DE CONTROL FD
        printf("%s", sendline);
        bzero(sendline, (int)sizeof(sendline));
    }
	//sleep(0.2);
    sprintf(sendline, "200");
    write(controlfd, sendline, strlen(sendline));
    pclose(in);

    return 1;
}
//do_pwd prints the actual directory path
int do_pwd(int controlfd, int datafd, char *input){
	char sendline[MAXLINE+1], str[MAXLINE+1];
	bzero(sendline, (int)sizeof(sendline));
	struct dir_data *struct_data = pthread_getspecific(glob_var_key);
	if(!struct_data){
		printf("pthread_getspecific failed\n");
    	pthread_exit((void *)13);
	}
	printf("%s \n", struct_data->var_dir);
	sprintf(sendline,struct_data->var_dir);
	write(controlfd, sendline, strlen(sendline)); 
    return 1;
}
//this function changes the directory of the server
int do_cd(int controlfd, int datafd, char *input){
	char filename[1024], sendline[MAXLINE+1], str[MAXLINE+1];
	bzero(filename, (int)sizeof(filename));
	bzero(sendline, (int)sizeof(sendline));
	bzero(str, (int)sizeof(str));
	struct dir_data *struct_data = pthread_getspecific(glob_var_key);
	if(!struct_data){
		printf("pthread_getspecific failed\n");
    	pthread_exit((void *)13);
	}

	char *oldwd = getcwd(NULL, 0);
	get_filename(input, filename);

	int err = chdir(filename);
	printf("Filename: %s\n", filename);
	if (err) {
		perror("Unable to chdir");
		printf("Filename Not Detected\n");
		sprintf(sendline, "450 Requested file action not taken.\nFilename Not Detected\n");
    	write(controlfd, sendline, strlen(sendline));
		return -1;
	}

	char *wd = getcwd(NULL, 0);
	if (strncmp(struct_data->curr_dir, wd, struct_data->curr_dir_len - 1)) {
		sprintf(sendline, "450 Requested file action not taken.\nFilename Not Detected\n");
    	write(controlfd, sendline, strlen(sendline));
		if (chdir(oldwd)) {
			perror("Unable to cd to old workingdir");
			if (chdir(struct_data->curr_dir)) {
				perror("Unable to cd home");
				sprintf(sendline, "Fatal error, couldn't cd home\n");
    			write(controlfd, sendline, strlen(sendline));
			}
		}
		return -1;		
	}
	
	if(strcmp(filename,"..")==0){
		struct_data->var_dir[strlen(struct_data->var_dir)-(strlen(struct_data->last_dir)+2)] = '\0';
		struct_data->last_dir = "";

	}else{
		struct_data->last_dir = filename;
		strcat(struct_data->var_dir, "/");
		strcat(struct_data->var_dir, filename);
	}
	bzero(sendline, (int)sizeof(sendline));
	sprintf(sendline, "200");

	printf("Changed directory to: %s\n", struct_data->var_dir);
	write(controlfd, sendline, strlen(sendline));
	return 1;
}
//this fuction is equivalent to a "get" so it copies the archive in the actual server directory 
//to the actual client directory 
int do_retr(int controlfd, int datafd, char *input){
	struct dir_data *struct_data = pthread_getspecific(glob_var_key);
	if(!struct_data){
		printf("pthread_getspecific failed\n");
		pthread_exit((void *)13);
	}
	chdir(struct_data->var_dir);
	char filename[1024], sendline[MAXLINE+1], str[MAXLINE+1];
	bzero(filename, (int)sizeof(filename));
	bzero(sendline, (int)sizeof(sendline));
	bzero(str, (int)sizeof(str));
	if(get_filename(input, filename) > 0){
		sprintf(str, "cat %s", filename);

		if((access(filename, F_OK)) != 0){
			sprintf(sendline, "550 No Such File or Directory\n");
    		write(controlfd, sendline, strlen(sendline));
    		return -1;
		}
	}else{
		printf("Filename Not Detected\n");
		sprintf(sendline, "450 Requested file action not taken.\nFilename Not Detected\n");
    	write(controlfd, sendline, strlen(sendline));
		return -1;
	}
	FILE *in;
    extern FILE *popen();

    if (!(in = popen(str, "r"))) {
    	sprintf(sendline, "451 Requested action aborted. Local error in processing\n");
    	write(controlfd, sendline, strlen(sendline));
        return -1;
    }

    while (fgets(sendline, MAXLINE, in) != NULL) {
        write(controlfd, sendline, strlen(sendline));
        bzero(sendline, (int)sizeof(sendline));
    }
	sleep(0.3); //sleep is needed to send the code 200 in the right way
    sprintf(sendline, "200");
    write(controlfd, sendline, strlen(sendline));
    pclose(in);
    return 1;
}
//this fuction is equivalent to a "put" so it copies the archive in the actual client directory 
//to the actual server directory 
int do_stor(int controlfd, int datafd, char *input){
	struct dir_data *struct_data = pthread_getspecific(glob_var_key);
	if(!struct_data){
		printf("pthread_getspecific failed\n");
		pthread_exit((void *)13);
	}
	chdir(struct_data->var_dir);
	char filename[8192], sendline[MAXLINE+1], recvline[MAXLINE+1], str[MAXLINE+1], temp1[8192];
	bzero(filename, (int)sizeof(filename));
	bzero(sendline, (int)sizeof(sendline));
	bzero(recvline, (int)sizeof(recvline));
	bzero(str, (int)sizeof(str));

	int n = 0, p = 0;

	if(get_filename(input, filename) > 0){
		sprintf(str, "%s-out", filename);
	}else{
		printf("Filename Not Detected\n");
		sprintf(sendline, "450 Requested file action not taken.\n");
    	write(controlfd, sendline, strlen(sendline));
		return -1;
	}

	sprintf(temp1, "%s-out", filename);
	FILE *fp;
    if((fp = fopen(temp1, "w")) == NULL){
        perror("file error");
        return -1;
    }

	
    while((n = read(controlfd, recvline, MAXLINE)) > 0){
		char *temp = strtok(recvline, " "); 
		if(strcmp(temp,"200") == 0){
			break;
		}
        fseek(fp, p, SEEK_SET);
        fwrite(recvline, 1, n, fp);
        p = p + n;
        bzero(recvline, (int)sizeof(recvline)); 
    }

    fclose(fp);
    return 1;
}
//handle_conn handles the connection between the server and the client
void * handle_conn(void* input){
	int	listenfd, connfd, port;
	listenfd = ((struct args*)input)->listenfd;
	port = ((struct args*)input)->port;
	struct dir_data *s = malloc(sizeof(struct dir_data));
	memset(s, 0, sizeof(s));
	
	s->curr_dir = getcwd(NULL, 0);
	s->var_dir = getcwd(NULL, 0);
	s->curr_dir_len = strlen(curr_dir);
	
	pthread_setspecific(glob_var_key, s);
	while(1){
		connfd = accept(listenfd, (struct sockaddr*)NULL, NULL);
		
		printf("New Client Detected...\n");
		//child process---------------------------------------------------------------
		
		close(listenfd);
		int datafd, code, x = 0, client_port = 0;
		char recvline[MAXLINE+1];
		char client_ip[50], command[1024];			
		
		while(1){
			bzero(recvline, (int)sizeof(recvline));				
			bzero(command, (int)sizeof(command));
			
			//get client's data connection port
			if((x = read(connfd, recvline, MAXLINE)) < 0){
				break;
			}
			sleep(0.1);				
			printf("*****************\n%s \n", recvline);
			if(strcmp(recvline, "QUIT") == 0){
				printf("Quitting...\n");					
				char goodbye[1024];
				sleep(0.1);
				sprintf(goodbye,"221 Goodbye");			
				write(connfd, goodbye, strlen(goodbye));
				sleep(0.1); //needed to connect more clients
				close(connfd);
				break;
			}
				
			get_client_ip_port(recvline, client_ip, &client_port);
			(setup_data_connection(&datafd, client_ip, client_port, port));
			if((x = read(connfd, command, MAXLINE)) < 0){
				break;
			}
			printf("-----------------\n%s \n", command);
			code = get_command(command);
			if(code == 1){
				do_list(connfd, datafd, command);
			}else if(code == 2){
				do_retr(connfd, datafd, command);
			}else if(code == 3){
				do_stor(connfd, datafd, command);
			}else if(code == 4){
				char reply[1024];
				sprintf(reply, "550 Filename Does Not Exist");
				write(connfd, reply, strlen(reply));
				close(datafd);
				continue;
			}else if(code == 6){
				do_cd(connfd, datafd, command); 
			}else if(code == 7){
				do_pwd(connfd, datafd, command); 
			}               
			close(datafd);
		}
		printf("Exiting Child Process...\n");
		close(connfd);
		break;
	
		//end child process-------------------------------------------------------------
	}
	return NULL;
}

//this function is needed to decide between pre-threads and pre-forks
void* job (int port){
	while(1){
		int	listenfd;
		struct sockaddr_in	servaddr;
		pid_t pid;

		int flag = 1;
		listenfd = socket(AF_INET, SOCK_STREAM, 0);
		if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag)) < 0)
			error("setsockopt(SO_REUSEADDR) failed");

		bzero(&servaddr, sizeof(servaddr));
		servaddr.sin_family      = AF_INET;
		servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
		servaddr.sin_port        = htons(port);
		
		bind(listenfd, (struct sockaddr*) &servaddr, sizeof(servaddr));

		listen(listenfd, LISTENQ);

		struct args *hilo = (struct args *)malloc(sizeof(struct args));
		hilo->listenfd= listenfd;
		hilo->port=port;

		if(!pre_thread){	
			for (int i = 0; i < number_Process; ++i) {
				int p = fork();
				if (p == 0) break;
				if (p < 0) {printf("Couldn't initialize fork.\n");}
			}
		}else{
			for(int i = 0;i<number_Process;i++){
				pthread_t t;
				pthread_key_create(&glob_var_key,NULL);
				if(pthread_create(&t, NULL, handle_conn, (void*) hilo)){
					printf("\nError in creation of the thread #%d\n", i);
					
				}
				else printf("\nThread #%d created succesfully.\n", i);
				
			}
		}
		pthread_key_create (&glob_var_key, NULL);
		handle_conn((void*)hilo);
		pthread_key_delete(glob_var_key);
	}
	return NULL;
}

int main(int argc, char **argv){

	//To run this program you need the following arguments
	//port, amountOfProcess, (pt|pf)
	//if you choose 'pt', the server will create pre-threads to handle clients
	//otherwise, it'll use forks.

	int port;
	if(argc != 9){
		printf("Invalid Number of Arguments...\n");
		printf("Usage: ./ftpserver -n <number-process> -w <ftp-root> -p <port> -t <type>\n");
		exit(-1);
	}
	if(strcmp(argv[1], "-n") == 0 && strcmp(argv[3], "-w") == 0 && strcmp(argv[5],"-p") == 0 && strcmp(argv[7], "-t") == 0){
		sscanf(argv[6], "%d", &port);
		sscanf(argv[2], "%d", &number_Process);
		number_Process-=1;
		
		if (mkdir(argv[4], 0777)){
			perror("Unable to create directory");
		}else{
			printf("Directory made\n");
		}
		if (chdir(argv[4]))
		{
			perror("Unable to chdir into directory");
			return (1);
		}
		printf("Chdir into %s\n", argv[4]);
		curr_dir = getcwd(NULL, 0);
		var_dir = curr_dir;
		curr_dir_len = strlen(curr_dir);
	}else{
		printf("Wrong Arguments...\n");
		exit(-1);
	}
	if(strcmp(argv[8],"pt")==0){
		pre_thread = true;
		printf("PRE-THREAD\n");
	}else{
		printf("PRE-FORKED\n");
	}
	job(port);	
}

