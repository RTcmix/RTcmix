#include <stdio.h>		/*						SFNAME.C	*/
#include "soundfile.h"	/* SFDIR defined here */
#include "sysdep.h"

#ifdef SFREMOTE			/* include net definitions if needed */
#include <netdb.h>
#endif

int	remotein = 0, remoteout = 0;

extern	char	errmsg[];

 char *
buildsfname(name)			/*  build a soundfile name	*/
 register char *name;			/*  <mod of carl getsfname()>	*/
{
register char	*sfdir;
	char	*getenv();
static	char	fullname[60];
						/* if path already given */
#ifdef SFREMOTE
	if (*name == '/' || *name == '.' || index(name,':') != NULL)
#else
	if (*name == '/' || *name == '.')
#endif
		return(name);			/*	ok as is	*/
	else {					/* else build right here  */
		if ((sfdir = getenv(SFDIR)) == NULL)
			dies("buildsfname: environment variable %s undefined",SFDIR);
		sprintf(fullname,"%s/%s",sfdir,name);
		return(fullname);
	}
}

rsfopen(name, direction)	/* open a soundfile remotely (across a net) */
 char	*name, direction;		/* see rexec(3N) for explanations */
{
#ifdef SFREMOTE
	struct	servent *sp;
	int	rsfd;
	char	fullname[60], *hostname, *wherecolon, command[70];
	
	if ((sp = getservbyname("exec", "tcp")) == NULL)
		die("remote-sf-open: can't find a server");
	strcpy(fullname,name);			/* copy complete name  */
	wherecolon = index(fullname,':');
	hostname = fullname;			/* isolate host & filename  */
	*wherecolon = '\0';
	sprintf(command,"cat %c %s", direction, wherecolon+1);
	if ((rsfd = rexec(&hostname, sp->s_port, NULL, NULL, command,
			(int *)NULL)) <= 0) {
		sprintf(errmsg,"rexec cannot rsfopen %s for %s",
			name, (direction == '<')? "input" : "output");
		die(errmsg);
	}
printf("rexec %s on %s opened with rfd %d\n",command,hostname,rsfd);
	if (direction == '<')		/* mark this direction remote	*/
		remotein = 1;		/* 	(i.e. no seeks)		*/
	else remoteout = 1;
	return(rsfd);
#else
	dies("remote access to %s not supported", name);
#endif
}

readin(infd,inbuf,nbytes)	/* special handling of sound input	*/
 register int	infd, nbytes;	/* to accomodate reads thru pipes & net	*/
 register char	*inbuf;		/* where nbytes rcvd can be < n requested */
{
register int	n, ntot=0;

	do if ((n = read(infd, inbuf+ntot, nbytes-ntot)) < 0)
		die("soundfile read error");
	while (n > 0 && (ntot += n) < nbytes);
	return(ntot);
}
