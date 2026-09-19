#include "signal.h"

extern int tw;
extern int th;
extern volatile sig_atomic_t terminalResized;

void draw();
void drawWithFilesys();

void handleResize(int signal);
