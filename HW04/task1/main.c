#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <limits.h>
#include <signal.h>
#include <unistd.h>

void sig_stp(int);
void sig_int(int);

char* massage;
char* reverse;
int max_counter;
char* current_text;
int counter = 0;

char *reverse_text(char *massage);

int main(int argc, char* argv[]){
    if(argc != 3){
        printf("Wrong number of arguments\n");
        return -1;
    }

    char* to_int_error;
    long n_repeat = strtol(argv[1], &to_int_error , 10);
    if(errno == ERANGE || 0!=strcmp(to_int_error,"\0") || n_repeat <= 0 || n_repeat > INT_MAX){
        printf("Expected number: %s\n", argv[2]);
        return 1;
    }
    max_counter = (int)n_repeat;
    massage = argv[2];
    current_text = argv[2];
    reverse = reverse_text(massage);

    struct sigaction exit_action;
    exit_action.sa_handler = sig_int;
    exit_action.sa_flags = 0;

    if(signal(SIGTSTP, sig_stp) == SIG_ERR)
        printf("can't catch SIGTSTP");
    if(sigaction(SIGINT, &exit_action, NULL) == -1)
        printf("can't catch SIGTERM");


    while(1){

        for(int i = 0; i< (counter/2)+1; i++)
            printf("%s\n", current_text);
        printf("\n\n");

        sleep(1);
    }

    return 0;
}

char *reverse_text(char *massage) {
    size_t len = strlen(massage) + 1;
    char* reversed  = malloc(len);
    for(int i =0; i< len-1; i++)
        reversed[i] = massage[len - 2 - i];
    reversed[len - 1] = '\0';
    return reversed;
}


void sig_stp(int sig){
/*    if (signal(SIGTSTP, sig_stp) == SIG_ERR)
        printf("can't catch SIGTSTP");*/
    counter = (counter + 1) % (2*max_counter);
    if(counter%2 == 1){
        current_text = reverse;
        printf("reverse\n");
    } else {
        printf("normall\n");
        current_text = massage;
    }
}

void sig_int(int sig){
    free(reverse);
    printf("received SIGINT\nexit\n");
    exit(0);
}
