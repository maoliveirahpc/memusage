/*=================================================================*/
/*= Utility to print VmSize, VmPeak, VMRSS and VmHWM of a program =*/
/*=================================================================*/

/*=================================================================*/
/*= Adapted from:                                                 =*/
/*= http://locklessinc.com/downloads/tmem.c                       =*/ 
/*=================================================================*/

#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define PATH_MAX 2048

int child_pid;

/*=================================================================*/

static int usage(char *me)
{
   fprintf(stderr, "%s: filename args\n", me);
   fprintf(stderr, "Run program, and print VmSize, VmPeak, VmRSS and VmHWM (in KiB) to stderr\n");
   return 0;
}

static int child(int argc, char **argv)
{
   int i;

   char **newargs = malloc(sizeof(char *) * argc);

   /* We can't be certain that argv is NULL-terminated, so do that now */
   for (i = 0 ; i < argc - 1 ; i++)
   {
      newargs[i] = argv[i+1];
   }
   newargs[argc - 1] = NULL;

   /* Launch the child */
   execvp(argv[1], newargs);

   return 0;
}

/*=================================================================*/

static void sig_chld(int dummy)
{
   (void) dummy;

   int status, child_val, pid;

   pid = waitpid(-1, &status, WNOHANG);
   if (pid < 0)
   {
      fprintf(stderr, "waitpid failed\n");
      return;
   }

   /* Only worry about direct child */
   if (pid != child_pid) return;

   /* Get child status value */
   if (WIFEXITED(status))
   {
      child_val = WEXITSTATUS(status);
      exit(child_val);
   }
}

/*=================================================================*/

static int main_loop(char *buf1, char *buf2)
{
   char *line;
   char *vmsize;
   char *vmpeak;
   char *vmrss;
   char *vmhwm;	

   size_t len;	

   FILE *f1, *f2;

   vmsize = NULL;
   vmpeak = NULL;
   vmrss = NULL;
   vmhwm = NULL;
   line = malloc(128);
   len = 128;

   f1 = fopen(buf1, "r");
   if (!f1) return 1;

   f2 = fopen(buf2, "a");
   if (!f2) return 1;

   /* Read memory size data from /proc/pid/status */
   while (!vmsize || !vmpeak || !vmrss || !vmhwm)
   {
      if (getline(&line, &len, f1) == -1)
      {
         /* Some of the information isn't there, die */
         return 1;
      }
      /* Find VmPeak */
      if (!strncmp(line, "VmPeak:", 7))
      {
            vmpeak = strdup(&line[7]);
      }
      /* Find VmSize */
      else if (!strncmp(line, "VmSize:", 7))
      {
            vmsize = strdup(&line[7]);
      }	
      /* Find VmRSS */
      else if (!strncmp(line, "VmRSS:", 6))
      {
            vmrss = strdup(&line[7]);
      }	
      /* Find VmHWM */
      else if (!strncmp(line, "VmHWM:", 6))
      {
            vmhwm = strdup(&line[7]);
      }
   }
   free(line);	
   fclose(f1);
   /* Get rid of " kB\n"*/
   len = strlen(vmsize);
   vmsize[len - 4] = 0;
   len = strlen(vmpeak);
   vmpeak[len - 4] = 0;
   len = strlen(vmrss);
   vmrss[len - 4] = 0;
   len = strlen(vmhwm);
   vmhwm[len - 4] = 0;

   /* Output results to file */
   fprintf(f2, "%s\t%s\t%s\t%s\n", vmsize, vmpeak, vmrss, vmhwm);
   fclose(f2);

   free(vmpeak);
   free(vmsize);
   free(vmrss);
   free(vmhwm);

   /* Success */
   return 0;
}

/*=================================================================*/

int main(int argc, char **argv)
{
   char buf1[PATH_MAX], buf2[PATH_MAX];

   struct sigaction act;

   if (argc < 2) return usage(argv[0]);

   act.sa_handler = sig_chld;

   /* We don't want to block any other signals */
   sigemptyset(&act.sa_mask);

   act.sa_flags = SA_NOCLDSTOP;

   if (sigaction(SIGCHLD, &act, NULL) < 0)
   {
      fprintf(stderr, "sigaction failed\n");
      return 1;
   }

   child_pid = fork();

   if (!child_pid) return child(argc, argv);

   snprintf(buf1, PATH_MAX, "/proc/%d/status", child_pid);

   snprintf(buf2, PATH_MAX, "memusage.%d.out", child_pid);

   /* Continual scan of proc */
   while (!main_loop(buf1,buf2))
   {
      /* Wait for 0.1 sec */
      usleep(100000);
   }

   return 1;
}

/*=================================================================*/
/*= END                                                           =*/
/*=================================================================*/
