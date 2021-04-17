#ifndef UTILITIES_H
#define UTILITIES_H

#include <errno.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h> //get file size
#include <sys/ipc.h>
#include <sys/shm.h>

#define ERROR -1

#define GET 1
#define POST 2
#define DELETE 3
#define PUT 4
#define EQUALS 0
#define MB 1000000
#define RESPONSE_BUFFER_SIZE 11*MB
#define FALSE 0
#define TRUE 1

// Maps the HTTP status code to the name in string
char *phrases[600];

// Auxiliar structure for receiving an HTTP request
typedef struct http_request{
    int method;
    char uri[256];
    char protocol[20];
} http_request;


// Auxiliar structure for building an HTTP response and sending it later
typedef struct http_response{
    int status_code;
    int method;
    char *body;
    int content_length;
    int content_type;
} http_response;



// Auxiliar structure for accessing the command line parameters of the program
typedef struct arguments{
    char path[100];
    int port;
    int processes;
}arguments;


/*
	Creates a socket and return the socket file descriptor
	*If there is any error then exits the program with corresponging message
*/
int create_socket();



/*
	Prints error followed by the name of errno and finally
	exits the program with code 1
*/
void exit_on_error(const char* error);



/*
	If there is any error then exits the program with
	corresponging message
*/
void bind_socket(int socket_fd, int port);



/*
    Set to null the buffer of stdout
*/
void disable_buffers();



/*
   *If there is any error then exits the program with corresponging message
*/
void start_listening(int socket_fd, int max_clients);



/*
    It connects to localhost at received port
    *If there is any error then exits the program with corresponging message
*/
void connect_to_server(int socked_fd, int port);



/*
    It fills the fields of the struct based on the string
    DOES NOTHING BY NOW
*/
void parse_http_response(char *string, http_response *response);



/*
    It fills the fields of the struct based on the string
*/
void parse_http_request(char *string, http_request *request);



/*
    It pints the current error held by errno
*/
void print_error_status();



/*
	Builds the final path to the requested uri and writes it
	to dest
	-folder must be only the name (without "/" at end)
	-uri can begin with "/" or not
	-dest is where the final result will be written to
*/
void build_filename(char *folder, char *uri, char *dest);


/*
    It reads all bytes from file and writes them to buffer
*/
int copy_file(FILE *file, char *buffer);



/*
    Fills SOME names of HTTP status codes
*/
void fill_phrases();



/*
    Fills the fields of the struct for easier access
*/
void parse_arguments(int argc, char *argv[], arguments *arguments);


/*
    Returns the size in bytes of the given file
*/
long int get_file_size(const char *file_name);


/*
    Returns the shared memory pointer associated to the key
 */
char *get_shared_memory_segment(int size, key_t key);



/*
    Fills the array with the given value
 */
void fill_array(int *array, int n, int value);



/*
    Writes the given file to the socket
*/
void write_file_to_socket(FILE *file, int socket_fd, int filesize);


/*
    Deletes the file at temp/temp+pid_file
*/
void delete_temp_file(int pid_file);


/*
    It serializes the struct and sends it through the socket
*/
void send_response(int socket_fd, http_response response);

#endif
