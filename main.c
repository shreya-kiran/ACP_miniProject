#include <ncurses.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "ncursers/ncursers.h"

static WINDOW *canvas_win = NULL;

static int promptCoords(int row, int col, const char *prompt,
                        int *x, int *y) {
    char input[64];
    mvprintw(row, col, "%s", prompt);
    clrtoeol();
    move(row, col + strlen(prompt));
    echo();
    refresh();
    int read = mvgetnstr(row, col + strlen(prompt), input, sizeof(input) - 1);
    noecho();
    if (read == ERR) {
        return 0;
    }
    if (sscanf(input, "%d %d", x, y) != 2) {
        return 0;
    }
    return 1;
}

static int promptInt(int row, int col, const char *prompt, int *value) {
    char input[64];
    mvprintw(row, col, "%s", prompt);
    clrtoeol();
    move(row, col + strlen(prompt));
    echo();
    refresh();
    int read = mvgetnstr(row, col + strlen(prompt), input, sizeof(input) - 1);
    noecho();
    if (read == ERR) {
        return 0;
    }
    if (sscanf(input, "%d", value) != 1) {
        return 0;
    }
    return 1;
}

static void showMessage(int row, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    move(row, 0);
    clrtoeol();
    vw_printw(stdscr, fmt, args);
    va_end(args);
    refresh();
}

static void drawMenu(void) {
    int rows = getCanvasRows();
    int cols = getCanvasCols();
    mvprintw(0, 0, "--- 2D Graphics Editor (Canvas: %dx%d, Origin: Top-Left) ---", rows, cols);
    mvprintw(1, 0, "Coordinate Limits - X: [0..%d], Y: [0..%d]", cols - 1, rows - 1);
    mvprintw(2, 0, "1. Add Rectangle");
    mvprintw(3, 0, "2. Add Circle");
    mvprintw(4, 0, "3. Add Line");
    mvprintw(5, 0, "4. Add Triangle");
    mvprintw(6, 0, "5. Delete Object by ID");
    mvprintw(7, 0, "6. Display Canvas");
    mvprintw(8, 0, "7. Exit");
    mvprintw(9, 0, "Enter your choice: ");
    clrtoeol();
}

static void renderCanvas(void) {
    werase(canvas_win);
    drawCanvasInWindow(canvas_win);
}

int main(void) {
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);

    if (LINES < 12 || COLS < 10) {
        endwin();
        fprintf(stderr, "Terminal must be at least 12 rows by 10 columns.\n");
        return 1;
    }

    int active_rows = LINES - 11;
    int active_cols = COLS;
    if (active_rows > CANVAS_ROWS) active_rows = CANVAS_ROWS;
    if (active_cols > CANVAS_COLS) active_cols = CANVAS_COLS;

    setActiveCanvasSize(active_rows, active_cols);

    canvas_win = newwin(active_rows, active_cols, 11, 0);
    if (canvas_win == NULL) {
        endwin();
        fprintf(stderr, "Failed to create canvas window.\n");
        return 1;
    }

    initCanvas();
    int choice;
    int id;
    bool redraw = true;

    while (1) {
        clear();
        drawMenu();
        if (redraw) {
            renderCanvas();
            redraw = false;
        }
        refresh();

        if (scanw("%d", &choice) != 1) {
            int c;
            while ((c = getch()) != ERR && c != '\n');
            showMessage(10, "Invalid input. Please enter a number.");
            continue;
        }

        Shape s;
        switch (choice) {
            case 1:
                strcpy(s.type, "rectangle");
                if (promptCoords(10, 0, "Enter x1 y1 for Point 1 (0..69, 0..49): ", &s.x1, &s.y1) != 2 ||
                    promptCoords(10, 0, "Enter x2 y2 for Point 2 (0..69, 0..49): ", &s.x2, &s.y2) != 2 ||
                    promptCoords(10, 0, "Enter x3 y3 for Point 3 (0..69, 0..49): ", &s.x3, &s.y3) != 2 ||
                    promptCoords(10, 0, "Enter x4 y4 for Point 4 (0..69, 0..49): ", &s.x4, &s.y4) != 2) {
                    showMessage(10, "Invalid input. Please enter valid coordinates.");
                    break;
                }
                if (!isAxisAlignedRectangle(s.x1, s.y1, s.x2, s.y2,
                                           s.x3, s.y3, s.x4, s.y4)) {
                    showMessage(10, "[ERROR] Coordinates do not form a valid axis-aligned rectangle.");
                    break;
                }
                if (addShape(s)) {
                    showMessage(10, "Rectangle added successfully!");
                    redraw = true;
                } else {
                    showMessage(10, "[ERROR] Placement failed due to collision or bounds.");
                }
                break;
            case 2:
                strcpy(s.type, "circle");
                if (promptCoords(10, 0, "Enter xc yc for Circle Center (0..69, 0..49): ", &s.x1, &s.y1) != 2 ||
                    promptInt(10, 0, "Enter radius for Circle (1..24): ", &s.radius) != 1) {
                    showMessage(10, "Invalid input. Please enter valid values.");
                    break;
                }
                if (s.radius <= 0 || s.radius > 24) {
                    showMessage(10, "[ERROR] Radius must be between 1 and 24.");
                    break;
                }
                if (s.x1 - s.radius < 0 || s.x1 + s.radius >= CANVAS_COLS ||
                    s.y1 - s.radius < 0 || s.y1 + s.radius >= CANVAS_ROWS) {
                    showMessage(10, "[ERROR] Circle goes out of canvas bounds.");
                    break;
                }
                if (addShape(s)) {
                    showMessage(10, "Circle added successfully!");
                    redraw = true;
                } else {
                    showMessage(10, "[ERROR] Placement failed due to collision or bounds.");
                }
                break;
            case 3:
                strcpy(s.type, "line");
                if (promptCoords(10, 0, "Enter x1 y1 for Start Point (0..69, 0..49): ", &s.x1, &s.y1) != 2 ||
                    promptCoords(10, 0, "Enter x2 y2 for End Point (0..69, 0..49): ", &s.x2, &s.y2) != 2) {
                    showMessage(10, "Invalid input. Please enter valid coordinates.");
                    break;
                }
                if (addShape(s)) {
                    showMessage(10, "Line added successfully!");
                    redraw = true;
                } else {
                    showMessage(10, "[ERROR] Placement failed due to collision or bounds.");
                }
                break;
            case 4:
                strcpy(s.type, "triangle");
                if (promptCoords(10, 0, "Enter x1 y1 for Base Point 1 (0..69, 0..49): ", &s.x1, &s.y1) != 2 ||
                    promptCoords(10, 0, "Enter x2 y2 for Base Point 2 (0..69, 0..49): ", &s.x2, &s.y2) != 2 ||
                    promptInt(10, 0, "Enter height for Triangle (1..49): ", &s.height) != 1) {
                    showMessage(10, "Invalid input. Please enter valid values.");
                    break;
                }
                if (s.height <= 0 || s.height >= CANVAS_ROWS) {
                    showMessage(10, "[ERROR] Height must be between 1 and 49.");
                    break;
                }
                if (addShape(s)) {
                    showMessage(10, "Triangle added successfully!");
                    redraw = true;
                } else {
                    showMessage(10, "[ERROR] Placement failed due to collision or bounds.");
                }
                break;
            case 5:
                if (promptInt(10, 0, "Enter ID of the object to delete: ", &id) != 1) {
                    showMessage(10, "Invalid input. Please enter a valid ID.");
                    break;
                }
                if (deleteShape(id)) {
                    showMessage(10, "Object deleted successfully.");
                    redraw = true;
                } else {
                    showMessage(10, "[ERROR] Object ID %d not found.", id);
                }
                break;
            case 6:
                showMessage(10, "Canvas refreshed.");
                redraw = true;
                break;
            case 7:
                cleanupShapes();
                endwin();
                return 0;
            default:
                showMessage(10, "Invalid choice. Enter 1-7.");
                break;
        }
    }

    cleanupShapes();
    endwin();
    return 0;
}
