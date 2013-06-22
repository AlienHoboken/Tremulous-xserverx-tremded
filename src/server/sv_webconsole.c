#include "server.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

int sv_webconsoleSocket;
qboolean sv_webconsoleConnected;
cvar_t *sv_webconsolePassword;
cvar_t  *sv_webconsoleHost;
cvar_t  *sv_webconsolePort;
cvar_t  *sv_webconsoleServer;

/* Closes webconsole socket */
void sv_webconsole_close( int *sockfd ) {
  close(*sockfd);
}

/* Connects the websocket */
qboolean sv_webconsole_connect( int *sockfd ) {
  int                 port;
  char                *host;
  char                message[32];
  struct sockaddr_in  server;
  struct hostent      *he;


  //get webconsole vars
  sv_webconsolePassword = Cvar_Get("sv_webconsolePassword", "", CVAR_ARCHIVE  );
  sv_webconsoleHost = Cvar_Get("sv_webconsoleHost", "127.0.0.1", CVAR_ARCHIVE );
  sv_webconsolePort = Cvar_Get("sv_webconsolePort", "5624", CVAR_ARCHIVE );
  sv_webconsoleServer = Cvar_Get("sv_webconsoleServer", "", CVAR_ARCHIVE );

  host = sv_webconsoleHost->string;
  port = sv_webconsolePort->integer;

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
  Com_sprintf( message, sizeof(message), "server:%s:%s\n", sv_webconsoleServer->string, sv_webconsolePassword->string );

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
    char *receive = malloc(bufsize+1);
    int response;

    response = recv(*sockfd,receive,bufsize,MSG_NOSIGNAL);
    if(response == -1) { //no response ready on non-blocking socket (hopefully)
      return NULL;
    } else if(response == 0) { //socket closed remotely
      close(*sockfd);
      *connected = qfalse;
    } else {
      Com_Printf("Received(%d): %s\n", (*connected == qtrue) ? 1 : 0, receive);
      return receive;
    }
  }

  //case where the call failed
  return NULL;
}
