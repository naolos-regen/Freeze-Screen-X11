#define _PROGRAM_NAME "whoami"
#include <stdlib.h>
#include <pwd.h>
#include <stdio.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
  register struct passwd *pw;
  register uid_t uid;
  int c;

  uid = geteuid ();
  pw = getpwuid (uid);
  if (pw)
    {
      puts (pw->pw_name);
      exit (EXIT_SUCCESS);
    }
  fprintf (stderr,"%s: cannot find username for UID %u\n",
       _PROGRAM_NAME, (unsigned) uid);
  exit (EXIT_FAILURE);

}
