#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
// TODO: Implement your signal handling code here!

  void segfault_handler(int signal, siginfo_t* info, void* ctx);

__attribute__((constructor)) void init() {

  struct sigaction sa;
  memset(&sa, 0, sizeof(struct sigaction));
  sa.sa_sigaction = segfault_handler;
  sa.sa_flags = SA_SIGINFO;

  // Set the signal handler, checking for errors
  if(sigaction(SIGSEGV, &sa, NULL) != 0) {
    perror("sigaction failed");
    exit(2);
  }

  printf("This code runs at program startup\n");
}
  void segfault_handler(int signal, siginfo_t* info, void* ctx){

    char *phrases[] = {"Hmmm, maybe you're close. Let's try the debugger :)", "Better luck next time, maybe take a lil break.", "Errors are proof that you're trying; keep that momentum!","Errors are just stepping stones to success.","Code errors are your secret sauce for future success.","Stay positive, debug diligently, and conquer your code!"};

    srand(time(NULL));
    int lucky_winner = (rand() % 6);
    printf("%s\n", phrases[lucky_winner]);


  exit(2);


  }