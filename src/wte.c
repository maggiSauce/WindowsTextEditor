#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <termios.h>
#include <stdlib.h>
#include <errno.h>

struct termios orig_termios;

/*
    function to print error and exit program
    Input:
        const char *s - string of error
*/
void die(const char *s) {
    perror(s);
    exit(1);
}

/*
    Disables raw mode
    To be run before ending program to return terminal to original state
*/
void disableRawMode() {
    if (tcsetattr(STDERR_FILENO, TCSAFLUSH, &orig_termios) == -1) {
        die("tcsetattr");
    }
}

/*  enables raw mode.
    To be run at start of program   
    
    Turns off ECHO, Canonical mode and CTRL-C and CTRL-Z signals
*/
void enableRawMode() {
    if (tcgetattr(STDERR_FILENO, &orig_termios) == -1) {
        die("tcgetattr");
    }
    atexit(disableRawMode);     // ensures disableRawMode() is called before exit

    struct termios raw = orig_termios;      // copy original terminal settings
    tcgetattr(STDERR_FILENO, &raw);
    raw.c_cflag &= ~(CS8);
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);                     // diable ctrl-s and ctrl q
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);     // AND-ing compliment of binary bitflags
    raw.c_oflag &= ~(OPOST);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1;
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) {
        die("tcsetattr");
    }
}




int main() {
    enableRawMode();

    while (1) {
        char c = '\0';
        if (read(STDIN_FILENO, &c, 1) == -1 && errno != EAGAIN) {
            die("read");
        }
        // code to handle case where c is a control character such as Esc
        if (iscntrl(c)) {
            printf("%d\r\n", c);
        } else {
            printf("%d ('%c')\r\n", c, c);
        }
        if (c == 'q') {
            break;
        }
    }
    return 0;
}






