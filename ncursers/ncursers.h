#ifndef NCURSERS_H
#define NCURSERS_H

#include <ncurses.h>

#define CANVAS_ROWS 50
#define CANVAS_COLS 70

typedef struct {
    int id;
    char type[32];
    int x1, y1;
    int x2, y2;
    int x3, y3;
    int x4, y4;
    int radius;
    int height;
} Shape;

void initCanvas(void);
void displayCanvas(void);
void drawCanvasInWindow(WINDOW *win);
void setActiveCanvasSize(int rows, int cols);
int getCanvasRows(void);
int getCanvasCols(void);
int addShape(Shape s);
int deleteShape(int id);
void redrawCanvas(void);
void cleanupShapes(void);
int isAxisAlignedRectangle(int x1, int y1, int x2, int y2,
                           int x3, int y3, int x4, int y4);

#endif // NCURSERS_H
