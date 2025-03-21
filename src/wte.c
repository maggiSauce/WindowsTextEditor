#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <termios.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/ioctl.h>

#define CTRL_KEY(k) ((k) & 0x1F)

/*** data ***/
struct termios orig_termios;
struct editorConfig {
    int screenrows;
    int screencols;
    struct termios orig_termios;
};
struct editorConfig E;
/*** terminal ***/

/*
    function to print error and exit program
    Input:
        const char *s - string of error
*/
void die(const char *s) {
    // clear screen an reset cursor on exit
    write(STDOUT_FILENO, "\x1b[2J", 4);
    write(STDOUT_FILENO, "\x1b[H", 3);

    perror(s);
    exit(1);
}

/*
    Disables raw mode
    To be run before ending program to return terminal to original state
*/
void disableRawMode() {
    if (tcsetattr(STDERR_FILENO, TCSAFLUSH, &E.orig_termios) == -1) {
        die("tcsetattr");
    }
}

/*  enables raw mode.
    To be run at start of program   
    
    Turns off ECHO, Canonical mode and CTRL-C and CTRL-Z signals
*/
void enableRawMode() {
    if (tcgetattr(STDERR_FILENO, &E.orig_termios) == -1) {
        die("tcgetattr");
    }
    atexit(disableRawMode);     // ensures disableRawMode() is called before exit

    struct termios raw = E.orig_termios;      // copy original terminal settings
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

char editorReadKey() {
    int nread;
    char c;
    while ((nread = read(STDIN_FILENO, &c, 1)) != 1) {
        if (nread == -1 && errno != EAGAIN) {
            die("read");
        }
    }
    return c;
}

/*
    places the rows and cols in ws struct and returns 0 on success
*/
int getWindowSize(int *rows, int *cols) {
    struct winsize ws;

    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
        return -1;
    } else {
        *cols = ws.ws_col;
        *rows = ws.ws_row;
        return 0;
    }
}

/*** Output ****/
void editorDrawRows() {
    int y;
    for (y = 0; y < 24; y++) {
        write(STDOUT_FILENO, "~\r\n", 3);
    }
}


void editorProcessKeyPress() {
    char c = editorReadKey();
    switch (c) {
        case CTRL_KEY('q'):     // exit program of CTRL-q pressed
            exit(0);
            break;
    }
}

void editorRefreshScreen() {
    // write 4 bytes to the terminal 1: \x1b is escape char, [2:4]: [2J
    write(STDOUT_FILENO, "\x1b[2J", 4);
    write(STDOUT_FILENO, "\xb1[H", 3);      // reposition cursor

    editorDrawRows();

    write(STDOUT_FILENO, "\x1b[H", 3);
}

/*** init ***/
// initialises all fields in editorConfig struct E
void initEditor() {
    if (getWindowSize(&E.screenrows, &E.screencols) == -1) die("getWindowSize");
}


int main() {
    enableRawMode();
    initEditor();

    while (1) {
        editorRefreshScreen();
        editorProcessKeyPress();
    }
    return 0;
}






