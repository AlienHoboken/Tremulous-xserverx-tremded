#include "server.h"
#include <mysql/mysql.h>

MYSQL *connection = NULL;
MYSQL_RES *results = NULL;
MYSQL_ROW row = NULL;

cvar_t *sv_mysql;
cvar_t *sv_mysqlhost;
cvar_t *sv_mysqldatabase;
cvar_t *sv_mysqlusername;
cvar_t *sv_mysqlpassword;

void sv_mysql_init( void ) {

  int timeout = 2;
  sv_mysql = Cvar_Get("sv_mysql", "0", CVAR_SERVERINFO );
  sv_mysqlhost = Cvar_Get("sv_mysqlhost", "localhost", CVAR_ARCHIVE  );
  sv_mysqldatabase = Cvar_Get("sv_mysqldatabase", "tremulous", CVAR_ARCHIVE  );
  sv_mysqlusername = Cvar_Get("sv_mysqlusername", "root", CVAR_ARCHIVE );
  sv_mysqlpassword = Cvar_Get("sv_mysqlpassword", "", CVAR_ARCHIVE  );

  connection = mysql_init( NULL );

  mysql_options(connection, MYSQL_OPT_CONNECT_TIMEOUT, (char*)&(timeout));

  Com_Printf( "Connecting to server %s timeout in %d seconds\n", sv_mysqlhost->string, timeout );
  
  if( !mysql_real_connect( connection, sv_mysqlhost->string, sv_mysqlusername->string, sv_mysqlpassword->string, sv_mysqldatabase->string, 0, NULL, 0) ) {
    Com_Printf( "^3WARNING:^7 MySQL loading failed: %s\n", mysql_error( connection ) );
    Cvar_Set( "sv_mysql", "0" );
  }
  Com_Printf( "MySQL loaded version: %s\n", mysql_get_client_info() );
  Cvar_Set( "sv_mysql", "1" );
}


void sv_mysql_shutdown( void ) {
  mysql_close( connection );
  Com_Printf( "MySQL Closed\n" );
  Cvar_Set( "sv_mysql", "0" );
}

void sv_mysql_reconnect( void ){
    sv_mysql_shutdown();
    sv_mysql_init();
}

qboolean sv_mysql_runquery( char *query ) {
  if( sv_mysql->integer == 1 ) {
    if( mysql_query( connection, query ) ) {
      Com_Printf( "^3WARNING:^7 MySQL Query failed: %s. Attempting to reconnect.\n", mysql_error( connection ) );
      //attempt to reconnect mysql if it failed
      sv_mysql_reconnect();
      if( sv_mysql->integer == 1 ) {
        if( mysql_query( connection, query ) ) {
          Com_Printf( "^3WARNING:^7 MySQL Query failed: %s\n", mysql_error( connection ) );
          return qfalse;
        }
        results = mysql_store_result( connection );
        return qtrue;
      }
      return qfalse;
    }
    results = mysql_store_result( connection );
    return qtrue;
  }
  return qfalse;
}

void sv_mysql_finishquery( void ) {
  if( sv_mysql->integer == 1 && results) {
    mysql_free_result( results );
    results = NULL;
  }
}

qboolean sv_mysql_fetchrow( void ) {
  if( sv_mysql->integer == 1 ) {
    row = mysql_fetch_row( results );
    if( !row ) {
      return qfalse;
    }
    return qtrue;
  }
  return qfalse;
}

void sv_mysql_fetchfieldbyID( int id, char *buffer, int len ) {
  if( sv_mysql->integer == 1 ) {
    if( row[ id ] ) {
      Q_strncpyz( buffer, row[ id ], len );
    } else {
      //do nothing
    }
  }
}

void sv_mysql_fetchfieldbyName( const char *name, char *buffer, int len ) {
  MYSQL_FIELD *fields;
  int num_fields;
  int i;

  if( sv_mysql->integer == 1 ) {
    num_fields = mysql_num_fields( results );
    fields = mysql_fetch_fields( results );

    for( i = 0; i < num_fields; i++ ) {
      if( !strcmp( fields[i].name, name ) ) {
        Q_strncpyz( buffer, row[ i ], len );
        return;
      }
    }
  }
}
