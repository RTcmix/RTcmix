#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>

#define DEFAULT_SOCKNUM 9999

int setnetplay(char *hname, char *snum)
{
	int s;
	struct sockaddr_in sss;
	struct hostent *hp;
	int sockno;
	char hostname[60];

	strcpy(hostname, hname);

	if (hostname[0] == '\0') {
		return(-1);
		perror("hostname");
	}

	if (snum[0] == '\0')
		sockno = DEFAULT_SOCKNUM;
	else
		sockno = atoi(snum);

	if( (s = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket");
		return(-1);
	}
	hp = gethostbyname(hostname);
	if (hp == NULL) return(-1);

	sss.sin_family = AF_INET;
	bcopy(hp->h_addr, &(sss.sin_addr.s_addr), hp->h_length);
	sss.sin_port = htons(sockno);

	if (connect(s, (struct sockaddr *)&sss, sizeof(sss)) < 0) {
		perror("connect");
		return(-1);
	}

	return s;
}

