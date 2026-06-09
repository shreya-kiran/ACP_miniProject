#include "ncursers.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char canvas[CANVAS_ROWS][CANVAS_COLS];

static int canvas_rows = CANVAS_ROWS;
static int canvas_cols = CANVAS_COLS;
static Shape *shapes = NULL;
static int shape_count = 0;
static int shape_capacity = 0;
static int next_id = 0;
static int dry_run = 0;
static int collision_detected = 0;

static void drawPoint(int x, int y, char c) {
    if (x >= 0 && x < canvas_cols && y >= 0 && y < canvas_rows) {
        if (dry_run) {
            if (canvas[y][x] != '_') {
                collision_detected = 1;
            }
        } else {
            canvas[y][x] = c;
        }
    }
}

static void drawLine(int x1, int y1, int x2, int y2, char marker) {
    int dx = abs(x2 - x1), sx = x1 < x2 ? 1 : -1;
    int dy = -abs(y2 - y1), sy = y1 < y2 ? 1 : -1;
    int err = dx + dy, e2;

    while (1) {
        drawPoint(x1, y1, marker);
        if (x1 == x2 && y1 == y2) break;
        e2 = 2 * err;
        if (e2 >= dy) { err += dy; x1 += sx; }
        if (e2 <= dx) { err += dx; y1 += sy; }
    }
}

static void drawCirclePoints(int xc, int yc, int x, int y, char marker) {
    drawPoint(xc + x, yc + y, marker);
    drawPoint(xc - x, yc + y, marker);
    drawPoint(xc + x, yc - y, marker);
    drawPoint(xc - x, yc - y, marker);
    drawPoint(xc + y, yc + x, marker);
    drawPoint(xc - y, yc + x, marker);
    drawPoint(xc + y, yc - x, marker);
    drawPoint(xc - y, yc - x, marker);
}

static void drawCircle(int xc, int yc, int r, char marker) {
    int x = 0;
    int y = r;
    int d = 3 - 2 * r;

    drawCirclePoints(xc, yc, x, y, marker);
    while (y >= x) {
        x++;
        if (d > 0) {
            y--;
            d = d + 4 * (x - y) + 10;
        } else {
            d = d + 4 * x + 6;
        }
        drawCirclePoints(xc, yc, x, y, marker);
    }
}

static int arePointsUnique(int x1, int y1, int x2, int y2,
                           int x3, int y3, int x4, int y4) {
    if (x1 == x2 && y1 == y2) return 0;
    if (x1 == x3 && y1 == y3) return 0;
    if (x1 == x4 && y1 == y4) return 0;
    if (x2 == x3 && y2 == y3) return 0;
    if (x2 == x4 && y2 == y4) return 0;
    if (x3 == x4 && y3 == y4) return 0;
    return 1;
}

int isAxisAlignedRectangle(int x1, int y1, int x2, int y2,
                           int x3, int y3, int x4, int y4) {
    if (!arePointsUnique(x1, y1, x2, y2, x3, y3, x4, y4)) {
        return 0;
    }

    int xs[4] = {x1, x2, x3, x4};
    int ys[4] = {y1, y2, y3, y4};
    for (int i = 0; i < 3; i++) {
        for (int j = i + 1; j < 4; j++) {
            if (xs[i] > xs[j]) {
                int temp = xs[i]; xs[i] = xs[j]; xs[j] = temp;
            }
            if (ys[i] > ys[j]) {
                int temp = ys[i]; ys[i] = ys[j]; ys[j] = temp;
            }
        }
    }

    if (xs[0] == xs[1] && xs[2] == xs[3] && xs[0] != xs[2] &&
        ys[0] == ys[1] && ys[2] == ys[3] && ys[0] != ys[2]) {
        return 1;
    }
    return 0;
}

void initCanvas(void) {
    for (int i = 0; i < canvas_rows; i++) {
        for (int j = 0; j < canvas_cols; j++) {
            canvas[i][j] = '_';
        }
    }
}

void setActiveCanvasSize(int rows, int cols) {
    if (rows < 1) rows = 1;
    if (cols < 1) cols = 1;
    if (rows > CANVAS_ROWS) rows = CANVAS_ROWS;
    if (cols > CANVAS_COLS) cols = CANVAS_COLS;
    canvas_rows = rows;
    canvas_cols = cols;
}

int getCanvasRows(void) {
    return canvas_rows;
}

int getCanvasCols(void) {
    return canvas_cols;
}

static void drawShape(Shape s) {
    if (strcmp(s.type, "rectangle") == 0) {
        drawLine(s.x1, s.y1, s.x2, s.y2, '#');
        drawLine(s.x2, s.y2, s.x3, s.y3, '#');
        drawLine(s.x3, s.y3, s.x4, s.y4, '#');
        drawLine(s.x4, s.y4, s.x1, s.y1, '#');
    } else if (strcmp(s.type, "circle") == 0) {
        drawCircle(s.x1, s.y1, s.radius, 'O');
    } else if (strcmp(s.type, "line") == 0) {
        drawLine(s.x1, s.y1, s.x2, s.y2, '*');
    } else if (strcmp(s.type, "triangle") == 0) {
        int mx = (s.x1 + s.x2) / 2;
        int my = (s.y1 + s.y2) / 2;
        int tx = mx;
        int ty = my - s.height;
        drawLine(s.x1, s.y1, s.x2, s.y2, '^');
        drawLine(s.x2, s.y2, tx, ty, '^');
        drawLine(tx, ty, s.x1, s.y1, '^');
    }
}

void redrawCanvas(void) {
    initCanvas();
    for (int i = 0; i < shape_count; i++) {
        drawShape(shapes[i]);
    }
}

int addShape(Shape s) {
    dry_run = 1;
    collision_detected = 0;
    drawShape(s);
    dry_run = 0;

    if (collision_detected) {
        return 0;
    }

    if (shape_count >= shape_capacity) {
        shape_capacity = shape_capacity == 0 ? 4 : shape_capacity * 2;
        Shape *temp = realloc(shapes, shape_capacity * sizeof(Shape));
        if (temp == NULL) {
            fprintf(stderr, "Error: Memory allocation failed!\n");
            return 0;
        }
        shapes = temp;
    }

    s.id = ++next_id;
    shapes[shape_count++] = s;
    drawShape(s);
    return 1;
}

int deleteShape(int id) {
    int index_to_remove = -1;
    for (int i = 0; i < shape_count; i++) {
        if (shapes[i].id == id) {
            index_to_remove = i;
            break;
        }
    }
    if (index_to_remove != -1) {
        for (int i = index_to_remove; i < shape_count - 1; i++) {
            shapes[i] = shapes[i + 1];
        }
        shape_count--;
        redrawCanvas();
        return 1;
    }
    return 0;
}

void displayCanvas(void) {
    for (int i = 0; i < canvas_rows; i++) {
        for (int j = 0; j < canvas_cols; j++) {
            putchar(canvas[i][j]);
        }
        putchar('\n');
    }
}

void drawCanvasInWindow(WINDOW *win) {
    for (int y = 0; y < canvas_rows; y++) {
        for (int x = 0; x < canvas_cols; x++) {
            mvwaddch(win, y, x, canvas[y][x]);
        }
    }
    wrefresh(win);
}

void cleanupShapes(void) {
    free(shapes);
    shapes = NULL;
    shape_count = 0;
    shape_capacity = 0;
}
