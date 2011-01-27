/********************************************************************
 *
 *  File    : ident.c
 *  Location: /home/asterix/pt94/pt94jpi/mud/raven/src/ident.c
 *
 *  Created : 97/03/31
 *  Author  : Joachim Pileborg
 *  E-Mail  : pt94jpi@student.hk-r.se
 *
 *  Comments:
 *
 *  Copyright (C) 1997 by Joachim Pileborg
 *  All rights reserved.
 *
 * $Id: ident.c,v 1.1 2002/07/17 15:10:26 dibrova Exp $
 */

#include "conf.h"
#include "sysdep.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <signal.h>

#include "structs.h"
#include "utils.h"
#include "db.h"
#include "ident.h"

/*******************************************************************/

int id_serv_socket  = -1;

/*******************************************************************/

extern int lookup_hostname;
extern int lookup_username;
extern int use_external_lookup_process;

void nonblock(int);

/*******************************************************************/

#define IDENTD_PORT  113

static pid_t id_pid          = -1;
static int   id_child_socket = -1;

static void  id_mainloop(void);
static char *id_lookup_hostname(struct sockaddr_in sa);
static char *id_lookup_username(struct sockaddr_in sa);

/********************************************************************
 * Function : id_init
 * Created  : 97/03/31
 * Author   : Joachim Pileborg
 * E-Mail   : pt94jpi@student.hk-r.se
 *
 */
int id_init(void)
{
  int fds[2];

  extern int no_ext_processes;

#ifdef CIRCLE_WINDOWS
  fprintf(stderr, "The hostname/username lookup service only works on " \
	  "UNIX systems!\n");
  log("   Ident service not started!");
  return 0;
#endif /* CIRCLE_WINDOWS */

  if (!use_external_lookup_process || no_ext_processes)
    /* the admin don't want to use a separate process */
    return 0;

  log("Starting Ident process...");

  if (socketpair(AF_UNIX, SOCK_DGRAM, 0, fds) < 0)
  {
    perror("id_init(): socketpair");
    return -1;
  }
  id_serv_socket  = fds[0];
  id_child_socket = fds[1];

  /* nonblock(id_serv_socket, 0); */
  /* nonblock(id_child_socket, 0); */

  if ((id_pid = fork()) < 0)
  {
    perror("Error creating Ident process");
    return -2;
  }
  else if (id_pid > 0)
  {
    /* we are in the mother process */
    close(id_child_socket);
    log("   Ident service started.");
    return id_pid;
  }
  else
  {
    /* we are in the child */
    close(id_serv_socket);

    id_mainloop();

    exit(0);
  }

  /*NOTREACHED*/
  return -1;
}

/********************************************************************
 * Function : id_kill
 * Created  : 97/03/31
 * Author   : Joachim Pileborg
 * E-Mail   : pt94jpi@student.hk-r.se
 *
 */
void id_kill(void)
{
  if (id_pid > 0)
  {
    log("Terminating Ident service process...");
    kill(id_pid, SIGKILL);
    close(id_serv_socket);
    log("   Ident service terminated.");
  }
}

/********************************************************************
 * Function : id_lookup
 * Created  : 97/03/31
 * Author   : Joachim Pileborg
 * E-Mail   : pt94jpi@student.hk-r.se
 *
 */
void id_lookup(struct descriptor_data *d)
{
  struct message msg;
  char          *str;

  if (id_pid > 0)
  {
    STATE(d) = CON_IDENT;
    msg.type = MSG_IDENT;
    msg.addr = d->saddr;
    msg.fd   = d->descriptor;
    write(id_serv_socket, &msg, sizeof(msg));
  }
  else
  {
    /* get the hostname */
    if (lookup_hostname && (str = id_lookup_hostname(d->saddr)) != NULL)
      strcpy(d->host_name, str);
    /* get the username */
    if (lookup_username && (str = id_lookup_username(d->saddr)) != NULL)
      strcpy(d->user_name, str);

    *d->host = 0;
    if (*d->user_name)
    {
      strcat(d->host, d->user_name);
      strcat(d->host, "@");
    }
    if (*d->host_name)
      strcat(d->host, d->host_name);
    else
      strcat(d->host, d->host_ip);
  }
}

/********************************************************************
 * Function : id_mainloop
 * Created  : 97/03/31
 * Author   : Joachim Pileborg
 * E-Mail   : pt94jpi@student.hk-r.se
 *
 */
static void id_mainloop(void)
{
  int  n;
  fd_set inp_set;
  struct message msg;
  char *str;

  for (;;)
  {
    FD_ZERO(&inp_set);
    FD_SET(id_child_socket, &inp_set);

    if (select(id_child_socket + 1,
               &inp_set, (fd_set *) NULL, (fd_set *) NULL, NULL) < 0)
    {
      perror("id_mainloop(): select");
      continue;
    }

    if (FD_ISSET(id_child_socket, &inp_set))
    {
      n = read(id_child_socket, &msg, sizeof(msg));
      memset(msg.host, 0, 256);
      memset(msg.user, 0, 256);

      if (msg.type == MSG_NOP)
	;
      else if (msg.type == MSG_QUIT)
      {
	write(id_child_socket, &msg, sizeof(msg));
	close(id_child_socket);
	exit(0);
      }
      else if (msg.type == MSG_IDENT)
      {
	/* get the hostname */
	if ((str = id_lookup_hostname(msg.addr)) != NULL)
	  strcpy(msg.host, str);
	/* get the username */
	if ((str = id_lookup_username(msg.addr)) != NULL)
	  strcpy(msg.user, str);

	msg.type = MSG_IDREP;
      }
      else
      {
        sprintf(buf, "SYSERR: id_mainloop(): msg.type = %d\n", msg.type);
	log(buf);
	msg.type = MSG_ERROR;
      }

      /* now reply on the message */
      write(id_child_socket, &msg, sizeof(msg));
    }
  }
}

/********************************************************************
 * Function : id_lookup_hostname
 * Created  : 97/03/31
 * Author   : Joachim Pileborg
 * E-Mail   : pt94jpi@student.hk-r.se
 *
 * Lookup a specific hostname.
 */
static char *id_lookup_hostname(struct sockaddr_in sa)
{
  static char     hostname[256];
  struct hostent *hent = NULL;

  if (!(hent = gethostbyaddr((char *) &sa.sin_addr,
                             sizeof(sa.sin_addr), AF_INET)))
  {
    perror("id_lookup_hostname(): gethostbyaddr");
    return NULL;
  }

  strcpy(hostname, hent->h_name);

  return hostname;
}

/********************************************************************
 * Function : id_lookup_username
 * Created  : 97/03/31
 * Author   : Joachim Pileborg
 * E-Mail   : pt94jpi@student.hk-r.se
 *
 * Try to get a username from a user.
 */
static char *id_lookup_username(struct sockaddr_in sa)
{
  static char        username[256];
  char               string[256];
  struct hostent    *hent;
  struct sockaddr_in saddr;
  int                sock;

  extern int port;

  sprintf(string, "%d, %d\r\n", ntohs(sa.sin_port), port);

  if (!(hent = gethostbyaddr((char *) &sa.sin_addr,
                             sizeof(sa.sin_addr), AF_INET)))
    perror("id_lookup_username(): gethostbyaddr");
  else
  {
    /* connect to the inetd super server */

    saddr.sin_family = hent->h_addrtype;
    saddr.sin_port   = htons(IDENTD_PORT);
    memcpy(&saddr.sin_addr, hent->h_addr, hent->h_length);

    if ((sock = socket(hent->h_addrtype, SOCK_STREAM, 0)) < 0)
    {
      perror("id_lookup_username(): socket");
      return NULL;
    }

    if (connect(sock, (struct sockaddr *) &saddr, sizeof(saddr)) < 0)
    {
      if (errno != ECONNREFUSED)
        perror("id_lookup_username(): connect");
      close(sock);
      return NULL;
    }

    /* write our message to the inetd super server */
    write(sock, &string, strlen(string));

    /* get the reply */
    read(sock, string, 256);

    /* close the socket */
    close(sock);

    /* extract what we need */
    {
      int sport, cport;
      char mtype[256], otype[256];

      sscanf(string, " %d , %d : %s : %s : %s ", &sport, &cport,
             mtype, otype, username);

      if (!strcmp(mtype, "USERID"))
	return username;
      else
        return NULL;
    }
  }

  return NULL;
}



/*******************************************************************/

/*******************************************************************/

/*******************************************************************/
