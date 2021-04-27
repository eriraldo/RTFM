#include 	<sys/types.h>
#include 	<sys/socket.h>
#include 	<strings.h>
#include	<string.h>
#include 	<arpa/inet.h>
#include 	<unistd.h>
#include 	<stdio.h>
#include    <ctype.h>
#include 	<stdlib.h>
#include 	<netinet/in.h>
#include	<stdbool.h>
#include 	<fcntl.h>
#include	<pthread.h>
#include    <sys/select.h>
#include    <sys/time.h>


#define 	MAXLINE 4096
#define     LISTENQ 1024
#define		TRUE	1
#define		FALSE	0

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


int get_user_input(char *buffer){
	//clear buffer
    memset(buffer, 0, (int)sizeof(buffer));

    //print prompt
    printf("> ");

    //get user input
    if(fgets(buffer, 1024, stdin) == NULL){
            return -1;
    }

    return 1;
}
//gets the port and the ip from the server and prints it
int get_port_string(char *str, char *ip, int n5, int n6){
    int i = 0;
    char ip_temp[1024];
    strcpy(ip_temp, ip);

    for(i = 0; i < strlen(ip); i++){
        if(ip_temp[i] == '.'){
            ip_temp[i] = ',';
        }
    }

    sprintf(str, "PORT %s,%d,%d", ip_temp, n5, n6);
    return 1;
}
//validates the command
int check_command(char *command){
	int i = 0, len = strlen(command), space = FALSE, count = 0;
	for(i = 0; i < len; i++){
		if(isspace(command[i]) == 0){
			space = FALSE;
			continue;
		}else{
			if(space == FALSE){
				count++;
			}
		}
	}

	if(count <= 1){return 1;}
	else{return -1;}
}
//check what command is going to be sent to the server
int get_command(char *command, char* args){    
	int value, check = -1;
	char copy[1024];	
	while(check < 0){
    	char *str;
        printf("ARGS: %s\n", args);
        if(strcmp(args,"")==0){
            if(get_user_input(command) < 0){
                printf("Cannot Read Command...\nPlease Try Again...\n");
                bzero(command, (int)sizeof(command));
                continue;
            }

            if(strlen(command) < 2){
                printf("No Input Detected...\nPlease Try Again\n");
                bzero(command, (int)sizeof(command));
                continue;
            }
            printf("Command1: %s\n", command);
            trim(command);
            strcpy(copy, command);
            printf("Command2: %s\n", command);

            if(check_command(copy) < 0){
                    printf("Invalid Format...\nPlease Try Again...\n");
                    bzero(command, (int)sizeof(command));
                    bzero(copy, (int)sizeof(copy));
                    continue;
            }
        }
    	
    	char delimit[]=" \t\r\n\v\f";
        if(strcmp(args,"")==0){
    	    str = strtok(copy, delimit);
        }else{
            char delimit2[]=" \t\r\n\v\f";
            str = strtok(args,delimit2);
            sleep(3);
        }
    	if((strcmp(str, "ls") == 0) || (strcmp(str, "get") == 0) || (strcmp(str, "put") == 0) || (strcmp(str, "quit") == 0) || (strcmp(str, "cd") == 0) || (strcmp(str, "pwd") == 0)){
    		check = 1;


            //populated value variable to indicate back to main which input was entered
            if(strcmp(str, "ls") == 0){value = 1;}
            else if(strcmp(str, "get") == 0){value = 2;}
            else if(strcmp(str, "put") == 0){value = 3;}
            else if(strcmp(str, "quit") == 0){value = 4;}
            else if(strcmp(str, "cd") == 0){value = 5;}
            else if(strcmp(str, "pwd") == 0){value =6;}
    	}else{
    		printf("Incorrect Command Entered...\nPlease Try Again...\n");
            bzero(command, strlen(command));
           	bzero(copy, sizeof(copy));
    		continue;
    	}
    }
	return value;
}
//convert is needed so that the server know which is the client port
int convert(uint16_t port, int *n5, int *n6){
    int i = 0;
    int x = 1;
    *n5 = 0;
    *n6 = 0;
    int temp = 0;
    for(i = 0; i< 8; i++){
        temp = port & x;
        *n6 = (*n6)|(temp);
        x = x << 1; 
    }

    port = port >> 8;
    x = 1;

    for(i = 8; i< 16; i++){
        temp = port & x;
        *n5 = ((*n5)|(temp));
        x = x << 1; 
    }
    return 1;
}
// takes the ip and the port
int get_ip_port(int fd, char *ip, int *port){
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);

    getsockname(fd, (struct sockaddr*) &addr, &len);
    sprintf(ip, inet_ntoa(addr.sin_addr));
    *port = (uint16_t)ntohs(addr.sin_port);
    return 1;
}
//takes the file that is going to be used for get or put commands
int get_filename(char *input, char *fileptr){
    char cpy[1024];
    char *filename = NULL;
    strcpy(cpy, input);
    trim(cpy);
    filename = strtok(cpy, " ");
    filename = strtok(NULL, " ");

    if(filename == NULL){
        fileptr = "\0";
        return -1;
    }else{
        strncpy(fileptr, filename, strlen(filename));
        return 1;
    }
}
//This function checks the codes received from the server
bool check_recvline(char* pch){
    bool check = false;
    char * recvline = strtok(pch, "\r\n");
    while (recvline != NULL)
        {
            //printf("%s\n", recvline);
            if (strcmp(recvline,"200")==0 || strcmp(recvline,"450")==0 || strcmp(recvline,"550")==0 || strcmp(recvline,"451")==0){
                check = true;
                break;
            }
            recvline = strtok(NULL, "\r\n");
        }
    return check;
}
// this function sends the ls command to the server and lists all the files from the current directory 
//of the server
int do_ls(int controlfd, int datafd, char *input){
    
    char filelist[256], str[MAXLINE+1], recvline[MAXLINE+1], *temp;
    bzero(filelist, (int)sizeof(filelist));
    bzero(recvline, (int)sizeof(recvline));
    bzero(str, (int)sizeof(str));

    fd_set rdset;
    int maxfdp1, data_finished = FALSE, control_finished = FALSE;

    if(get_filename(input, filelist) < 0){
        printf("No Filelist Detected...\n");
        sprintf(str, "LIST");
    }else{
        sprintf(str, "LIST %s", filelist);
    }

    bzero(filelist, (int)sizeof(filelist));

    FD_ZERO(&rdset);
    FD_SET(controlfd, &rdset);
    FD_SET(datafd, &rdset);

    if(controlfd > datafd){
        maxfdp1 = controlfd + 1;
    }else{
        maxfdp1 = datafd + 1;
    }

    write(controlfd, str, strlen(str));
    while(1){
        if(control_finished == FALSE){FD_SET(controlfd, &rdset);}
        if(data_finished == FALSE){FD_SET(datafd, &rdset);}
        select(maxfdp1, &rdset, NULL, NULL, NULL);
        if(FD_ISSET(controlfd, &rdset)){             
            printf("Server Data Response:\n");
            while(read(controlfd, recvline, MAXLINE) > 0){                
                temp = strtok(recvline, " ");                             
                printf("%s", recvline); 
                if(check_recvline(temp)){
                    printf("Exiting...\n");
                    break;
                }   
                bzero(recvline, (int)sizeof(recvline)); 
            }
            data_finished = TRUE;
            control_finished = TRUE;
            FD_CLR(datafd, &rdset);
            FD_CLR(controlfd, &rdset);            
        }
        if((control_finished == TRUE) && (data_finished == TRUE)){
            break;
        }
    }
    bzero(filelist, (int)sizeof(filelist));
    bzero(recvline, (int)sizeof(recvline));
    bzero(str, (int)sizeof(str));
    return 1;
}
// this function sends the pwd command to the server and gets the current servers directory path
int do_pwd(int controlfd, int datafd, char *input){
    
    char str[MAXLINE+1], recvline[MAXLINE+1];
    bzero(recvline, (int)sizeof(recvline));
    bzero(str, (int)sizeof(str));
    fd_set rdset;
    sprintf(str, "PWD");
    FD_ZERO(&rdset);
    FD_SET(controlfd, &rdset);

    
    write(controlfd, str, strlen(str));
    read(controlfd, recvline, MAXLINE);
    printf("Server Data Response: %s \n", recvline); 
    FD_CLR(controlfd, &rdset);

    bzero(recvline, (int)sizeof(recvline));
    bzero(str, (int)sizeof(str));

    return 1;
}

//this fuction copies the archive in the server client directory 
//to the actual client directory 

int do_get(int controlfd, int datafd, char *input){
    char filename[256], str[MAXLINE+1], recvline[MAXLINE+1], *temp, temp1[1024];
    bzero(filename, (int)sizeof(filename));
    bzero(recvline, (int)sizeof(recvline));
    bzero(str, (int)sizeof(str));
    int n = 0, p = 0;

    fd_set rdset;
    int maxfdp1, data_finished = FALSE, control_finished = FALSE;

    

    if(get_filename(input, filename) < 0){
        printf("No filename Detected...\n");
        char send[1024];
        sprintf(send, "SKIP");
        write(controlfd, send, strlen(send));
        bzero(send, (int)sizeof(send));
        read(controlfd, send, 1000);
        printf("Server Response: %s\n", send);
        return -1;
    }else{
        sprintf(str, "RETR %s", filename);
    }   
    printf("File: %s\n", filename);
    sprintf(temp1, "%s-out", filename);
    bzero(filename, (int)sizeof(filename));


    FD_ZERO(&rdset);
    FD_SET(controlfd, &rdset);
    FD_SET(datafd, &rdset);


    if(controlfd > datafd){
        maxfdp1 = controlfd + 1;
    }else{
        maxfdp1 = datafd + 1;
    }

    
    FILE *fp;
    if((fp = fopen(temp1, "w")) == NULL){
        perror("file error");
        return -1;
    }

    write(controlfd, str, strlen(str));
    while(1){
        if(control_finished == FALSE){FD_SET(controlfd, &rdset);}
        if(data_finished == FALSE){FD_SET(datafd, &rdset);}
        select(maxfdp1, &rdset, NULL, NULL, NULL);

        if(FD_ISSET(controlfd, &rdset)){
            bzero(recvline, (int)sizeof(recvline));
            while((n = read(controlfd, recvline, MAXLINE)) > 0){
                fseek(fp, p, SEEK_SET);
                fwrite(recvline, 1, n, fp);
                p = p + n;
                temp = strtok(recvline, " ");
                if(check_recvline(temp)){
                    break;
                }
                bzero(recvline, (int)sizeof(recvline)); 
            }
            data_finished = TRUE;
            FD_CLR(datafd, &rdset);
            control_finished = TRUE;
            bzero(recvline, (int)sizeof(recvline));
            FD_CLR(controlfd, &rdset);
        }
        if((control_finished == TRUE) && (data_finished == TRUE)){
            break;
        }
    }
    bzero(filename, (int)sizeof(filename));
    bzero(recvline, (int)sizeof(recvline));
    bzero(str, (int)sizeof(str));
    fclose(fp);
    return 1;
}
//this function sends the cd command to the server and changes the actual server directory to a new one
//specified
int do_cd(int controlfd, int datafd, char *input){
    
    char filename[256], str[MAXLINE+1], recvline[MAXLINE+1], *temp, temp1[1024];
    bzero(filename, (int)sizeof(filename));
    bzero(recvline, (int)sizeof(recvline));
    bzero(str, (int)sizeof(str));
    int n = 0, p = 0;

    fd_set rdset;
    int maxfdp1, data_finished = FALSE, control_finished = FALSE;

    get_filename(input, filename);
    sprintf(str, "CD %s", filename);
    bzero(filename, (int)sizeof(filename));
    FD_ZERO(&rdset);
    FD_SET(controlfd, &rdset);
    FD_SET(datafd, &rdset);
    if(controlfd > datafd){
        maxfdp1 = controlfd + 1;
    }else{
        maxfdp1 = datafd + 1;
    }

    write(controlfd, str, strlen(str));
    while(1){
        if(control_finished == FALSE){FD_SET(controlfd, &rdset);}
        if(data_finished == FALSE){FD_SET(datafd, &rdset);}
        select(maxfdp1, &rdset, NULL, NULL, NULL);

        if(FD_ISSET(controlfd, &rdset)){           
            printf("Server Data Response:\n");
            while(read(controlfd, recvline, MAXLINE) > 0){                
                temp = strtok(recvline, " ");                             
                printf("%s", recvline); 
                if(check_recvline(temp)){
                    printf("Exiting...\n");
                    break;
                }   
                bzero(recvline, (int)sizeof(recvline)); 
            }
            data_finished = TRUE;
            control_finished = TRUE;
            FD_CLR(datafd, &rdset);
            FD_CLR(controlfd, &rdset);            
        }
        if((control_finished == TRUE) && (data_finished == TRUE)){
            break;
        }

    }
    bzero(filename, (int)sizeof(filename));
    bzero(recvline, (int)sizeof(recvline));
    bzero(str, (int)sizeof(str));
    return 1;
}
//this fuction copies the archive in the actual client directory 
//to the actual server directory 
int do_put(int controlfd, int datafd, char *input){
    char filename[256], str[MAXLINE+1], recvline[MAXLINE+1], sendline[MAXLINE+1], *temp, temp1[1024];
    bzero(filename, (int)sizeof(filename));
    bzero(recvline, (int)sizeof(recvline));
    bzero(str, (int)sizeof(str));
    fd_set wrset, rdset;
    int maxfdp1, data_finished = FALSE, control_finished = FALSE;
    if(get_filename(input, filename) < 0){
        printf("No filename Detected...\n");
        char send[1024];
        sprintf(send, "SKIP");
        write(controlfd, send, strlen(send));
        bzero(send, (int)sizeof(send));
        read(controlfd, send, 1000);
        printf("Server Control Response: %s\n", send);
        return -1;
    }else{
        sprintf(str, "STOR %s", filename);
    }   
    sprintf(temp1, "cat %s", filename);
    bzero(filename, (int)sizeof(filename));


    FD_ZERO(&wrset);
    FD_ZERO(&rdset);
    FD_SET(controlfd, &rdset);
    FD_SET(datafd, &wrset);


    if(controlfd > datafd){
        maxfdp1 = controlfd + 1;
    }else{
        maxfdp1 = datafd + 1;
    }
    FILE *in;
    extern FILE *popen();

    if (!(in = popen(temp1, "r"))) {
        printf("Cannot Run Command\nExiting...\n");
        return -1;
    }
    write(controlfd, str, strlen(str));
    while(1){
        if(control_finished == FALSE){FD_SET(controlfd, &wrset);}
        if(data_finished == FALSE){FD_SET(datafd, &wrset);}
        select(maxfdp1, &rdset, &wrset, NULL, NULL);
        FD_SET(controlfd, &wrset);
        if(FD_ISSET(controlfd, &wrset)){           
            bzero(sendline, (int)sizeof(sendline));
            while(fgets(sendline, MAXLINE, in) != NULL){
                write(controlfd, sendline, strlen(sendline));
                bzero(sendline, (int)sizeof(sendline));
            }
            data_finished = TRUE;
            control_finished = TRUE;
            FD_CLR(datafd, &wrset);
            FD_CLR(controlfd, &wrset);   
            close(datafd);         
        }
        sleep(0.2);
        sprintf(sendline, "200"); //ends sending a 200 to the server
        write(controlfd, sendline, strlen(sendline));
        if((control_finished == TRUE) && (data_finished == TRUE)){
            break;
        }
    }
    return 1;
}

int main(int argc, char **argv){
    //./ftpclient -h 8080 [command]
	int server_port, controlfd, listenfd, datafd, code, n5, n6, x;
    uint16_t port;
	struct sockaddr_in servaddr, data_addr;
	char command[1024], ip[50], str[MAXLINE+1];
    char args[100] = "";
    if(!(argc>=3 && argc <5)){        
        printf("Invalid Number of Arguments...\n");
        printf("Usage: ./ftpclient -h <host_port> [command]\n");
        exit(-1);
    }
	

    if(argc==4){
       strncpy(args,argv[3],99);
    }
	//get server port
	sscanf(argv[2], "%d", &server_port);

    //set up control connection--------------------------------------------------
    if ( (controlfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
    	perror("socket error");
    	exit(-1);
    }
                
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port   = htons(server_port);
    if (inet_pton(AF_INET, "127.0.0.1", &servaddr.sin_addr) <= 0){
    	perror("inet_pton error");
    	exit(-1);
    }
        
    if (connect(controlfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0){
    	perror("connect error");
    	exit(-1);
    }


    //set up data connection------------------------------------------------------
    listenfd = socket(AF_INET, SOCK_STREAM, 0);

    bzero(&data_addr, sizeof(data_addr));
    data_addr.sin_family      = AF_INET;
    data_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    data_addr.sin_port        = htons(0);



    bind(listenfd, (struct sockaddr*) &data_addr, sizeof(data_addr));

    listen(listenfd, LISTENQ);
    
    //get ip address from control port
    get_ip_port(controlfd, ip, (int *)&x);
    //x = 0;
    printf("x: %d\n", x);
    printf("ip: %s\n", ip);
    //get data connection port from listenfd
    get_ip_port(listenfd, str, (int *)&port);
    
    printf("Port: %d, str: %s\n",  port, str);
    convert(port, &n5, &n6);

    while(1){

        bzero(command, strlen(command));
        //get command from user
        code = get_command(command, args);
        
        
        //user has entered quit
        if(code == 4){
            char quit[1024];
            sprintf(quit, "QUIT");
            write(controlfd, quit, strlen(quit));
            bzero(quit, (int)sizeof(quit));
            read(controlfd,quit, 1024);
            printf("Server Response: %s\n", quit);
            break;
        }
	if(argc == 4){
            strcpy(command, argv[3]);
        }
        printf("command: %s\n", command);

        //send PORT n1,n2,n3,n4,n5,n6
        bzero(str, (int)sizeof(str));       
        get_port_string(str, ip, n5, n6);

        write(controlfd, str, strlen(str));
        bzero(str, (int)sizeof(str));

        datafd = accept(listenfd, (struct sockaddr*)NULL, NULL);
        int test = data_addr.sin_port;

        printf("Data connection Established...\n");

        if(code == 1){
            if(do_ls(controlfd, datafd, command) < 0){
                close(datafd);
                continue;
            }
        }else if(code == 2){
            if(do_get(controlfd, datafd, command) < 0){
                close(datafd);
                continue;
            }
        }else if(code == 3){
            if(do_put(controlfd, datafd, command) < 0){
                close(datafd);
                continue;
            }
        }else if(code == 5){
            if(do_cd(controlfd, datafd, command) < 0){
                close(datafd);
                continue;
            }
        }else if(code == 6){
            if(do_pwd(controlfd, datafd, command) < 0){
                close(datafd);
                continue;
            }
        }           
        close(datafd);
    }
    close(controlfd);	
	return TRUE;
}
