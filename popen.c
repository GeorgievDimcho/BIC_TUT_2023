
#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)popen.c	8.3 (Berkeley) 5/3/95";
#endif /* LIBC_SCCS and not lint */
#include <sys/cdefs.h>

#include <sys/param.h>
#include <sys/queue.h>
#include <sys/wait.h>

#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <paths.h>
#include <pthread.h>

extern char **environ;

// benutzt für bestimmung von dr größe von dem Speicher Raum
struct pid {
	SLIST_ENTRY(pid) next;
	FILE *fp;
	pid_t pid; // signed integer
};


static SLIST_HEAD(, pid) pidlist = SLIST_HEAD_INITIALIZER(pidlist);
static pthread_mutex_t pidlist_mutex = PTHREAD_MUTEX_INITIALIZER;

FILE *
popen(command, type)
	const char *command, *type;
{
	printf("popen is called! \n");
	struct pid *cur;
	FILE *iop;
	int pdes[2], pid,twoway, cloexec;
	char *argv[4];
	struct pid *p;
	
	cloexec = strchr(type, 'e') != NULL;
	
	pipe(pdes);

	if ((cur = malloc(sizeof(struct pid))) == NULL) { // dynamischen speichern
		(void)close(pdes[0]);
		(void)close(pdes[1]);
		return (NULL);
	}
 

	argv[0] = "sh";
	argv[1] = "-c";
	argv[2] = (char *)command; // "ls"
	argv[3] = NULL;

	//THREAD_LOCK();
	switch (pid = fork()) {
	case -1:			/* Error. */
		//THREAD_UNLOCK();
		(void)close(pdes[0]);
		(void)close(pdes[1]);
		free(cur); // speicher frei machen
		return (NULL);
		/* NOTREACHED */
	case 0:				/* Child. */ //
		if (*type == 'r') {
			/*
			 * The _dup2() to STDIN_FILENO is repeated to avoid
			 * writing to pdes[1], which might corrupt the
			 * parent's copy.  This isn't good enough in
			 * general, since the _exit() is no return, so
			 * the compiler is free to corrupt all the local
			 * variables.
			 */
			if (!cloexec)
				(void)close(pdes[0]);
				
			if (pdes[1] != STDOUT_FILENO) {
				printf("dsfdsasd\n");
				(void)dup2(pdes[1], STDOUT_FILENO);
				if (!cloexec)
				printf("sd\n");
					(void)close(pdes[1]);
			} 
			if (cloexec)
				(void)fcntl(pdes[1], F_SETFD, 0);
		} else {
			if (pdes[0] != STDIN_FILENO) {
				(void)dup2(pdes[0], STDIN_FILENO);
				if (!cloexec)
					(void)close(pdes[0]);
			} else if (cloexec)
				(void)fcntl(pdes[0], F_SETFD, 0);
			if (!cloexec)
				(void)close(pdes[1]);
		}
		SLIST_FOREACH(p, &pidlist, next)
			(void)close(fileno(p->fp));
		execve(_PATH_BSHELL, argv, environ); // liefert nichts zurück, falls alles gepasst hat
		exit(127);
		/* NOTREACHED */
	}
	//THREAD_UNLOCK();

	/* Parent; assume fdopen can't fail. */
	if (*type == 'r') {
		iop = fdopen(pdes[0], type);
		(void)close(pdes[1]);
	} else {
		iop = fdopen(pdes[1], type);
		(void)close(pdes[0]);
	}

	/* Link into list of file descriptors. */
	cur->fp = iop;
	cur->pid = pid;
	//THREAD_LOCK();
	SLIST_INSERT_HEAD(&pidlist, cur, next);
	//THREAD_UNLOCK();

	return (iop);
}

/*
 * pclose --
 *	Pclose returns -1 if stream is not associated with a `popened' command,
 *	if already `pclosed', or waitpid returns an error.
 */
int
pclose(iop)
	FILE *iop;
{
	struct pid *cur, *last = NULL;
	int pstat;
	pid_t pid;

	/*
	 * Find the appropriate file pointer and remove it from the list.
	 */
	//THREAD_LOCK();
	SLIST_FOREACH(cur, &pidlist, next) {
		if (cur->fp == iop)
			break;
		last = cur;
	}
	if (cur == NULL) {
		//THREAD_UNLOCK();
		return (-1);
	}
	if (last == NULL)
		SLIST_REMOVE_HEAD(&pidlist, next);
	//else
//		SLIST_REMOVE_AFTER(last, next);
	//THREAD_UNLOCK();

	(void)fclose(iop);

	do {
		pid = wait4(cur->pid, &pstat, 0, (struct rusage *)0);
	} while (pid == -1 && errno == EINTR);

	free(cur);

	return (pid == -1 ? -1 : pstat);
}

int main()
{
// we create processes for both `ls` and `wc`
// `popen` returns a FILE* that represents the process' stdout (`r`) or stdin (`w`)
FILE *ls = popen("ls", "r");
FILE *wc = popen("wc", "w");
// we consume the output of `ls` and feed it to `wc`
char buf[1024];
while (fgets(buf, sizeof(buf), ls) != NULL)
fputs(buf, wc);
// once we're done, we close the streams
pclose(ls);

pclose(wc);
}