#include "server.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <string.h>

int sv_webconsoleSocket;
cvar_t *sv_webconsolePassword;

/* Connects the websocket */
qboolean sv_webconsole_connect( int *sockfd ) {
  int                 port = 5624;
  char                *host = "50.21.181.81";
  struct sockaddr_in  server;
  struct hostent      *he;

  //get password
  sv_webconsolePassword = Cvar_Get("sv_webconsolePassword", "", CVAR_ARCHIVE  );

  //create socket
  *sockfd = socket(AF_INET,SOCK_STREAM,0);
  if (*sockfd == -1) {
    return qfalse;
  }

  //resolve host
  if ((he = gethostbyname(host)) == NULL) {
    return qfalse;
  }

  //build struct
  memcpy(&server.sin_addr, he->h_addr_list[0], he->h_length);
  server.sin_family = AF_INET;
  server.sin_port = htons(port);

  //attempt to connect the socket
  if (connect(*sockfd, (struct sockaddr *)&server, sizeof(server))) {
    return qfalse;
  }

  //non-blocking
  fcntl(*sockfd, F_SETFL, O_NONBLOCK);

  //send initial message
  char *message = "server:x:a@og72gH8!*Fhga#\n";

  if(send(*sockfd,message,strlen(message),MSG_NOSIGNAL) == -1) {
    close(*sockfd);
    return qfalse;
  }

  return qtrue;
}

/* Sends a message over webconsole socket.
 * Attempts to create a socket if necessary.
 */
void sv_webconsole_send( int *sockfd, char *message, qboolean *connected ) {
Com_Printf("Attempting Send: %s", message);
  //attempt to connect socket if disconnected
  if(*connected == qfalse) {
    //first try to close it, then connect it
    close(*sockfd);
    *connected = sv_webconsole_connect(sockfd);
  }

  //if now connected
  if(*connected == qtrue) {
    //make sure message sends successfully
    if(send(*sockfd, message, strlen(message), MSG_NOSIGNAL) == -1) {
      *connected = qfalse;
      close(*sockfd);
    }
  }
}

/* Closes webconsole socket */
void sv_webconsole_close( int *sockfd ) {
  close(*sockfd);
}

/* Attempts to read a message from the webconsole
 * Returns NULL if nothing was successfully read
 */
char* sv_webconsole_read( int *sockfd, qboolean *connected) {
  //attempt to connect socket if disconnected
  if(*connected == qfalse) {
    //first try to close it, then connect it
    close(*sockfd);
    *connected = sv_webconsole_connect(sockfd);
  }

  //if now connected
  if(*connected == qtrue) {
    //we can receive 1kb packets
    int bufsize=1024;
    char *receive = malloc(bufsize);

    if(recv(sockfd,receive,bufsize,MSG_NOSIGNAL) == -1) {
//      Com_Printf("Error receiving data\n");
      return NULL;
    } else {
Com_Printf("1");
      Com_Printf("Received: %s\n", receive);
      return receive;
    }
  }

  //case where the call failed
  return NULL;
}
