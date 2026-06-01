#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ROWS 20
#define COLS 60

// Global canvas array
char canvas[ROWS][COLS];

// Shape definition
typedef struct {
    int id;
    char type[32];
    int x1;
    int y1;
    int x2;
    int y2;
} Shape;

// Global list of shapes (using dynamic array)
Shape* shapes = NULL;
int shape_count = 0;
int shape_capacity = 0;
int next_id = 0;

// Initializes the canvas with underscore character '_'
void initCanvas() {
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            canvas[i][j] = '_';
        }
    }
}

// Prints the canvas to the console row by row
void displayCanvas() {
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            putchar(canvas[i][j]);
        }
        putchar('\n');
    }
}

// Helper to draw a single point safely
void drawPoint(int x, int y, char c) {
    if (x >= 0 && x < COLS && y >= 0 && y < ROWS) {
        canvas[y][x] = c;
    }
}

// Helper to draw a shape onto the canvas
void drawShape(Shape s) {
    if (strcmp(s.type, "rectangle") == 0) {
        int min_x = s.x1 < s.x2 ? s.x1 : s.x2;
        int max_x = s.x1 > s.x2 ? s.x1 : s.x2;
        int min_y = s.y1 < s.y2 ? s.y1 : s.y2;
        int max_y = s.y1 > s.y2 ? s.y1 : s.y2;
        
        for (int x = min_x; x <= max_x; x++) {
            drawPoint(x, min_y, '#');
            drawPoint(x, max_y, '#');
        }
        for (int y = min_y; y <= max_y; y++) {
            drawPoint(min_x, y, '#');
            drawPoint(max_x, y, '#');
        }
    } else if (strcmp(s.type, "line") == 0) {
        // Bresenham's line algorithm
        int x1 = s.x1, y1 = s.y1, x2 = s.x2, y2 = s.y2;
        int dx = abs(x2 - x1), sx = x1 < x2 ? 1 : -1;
        int dy = -abs(y2 - y1), sy = y1 < y2 ? 1 : -1;
        int err = dx + dy, e2;
        
        while (1) {
            drawPoint(x1, y1, '*');
            if (x1 == x2 && y1 == y2) break;
            e2 = 2 * err;
            if (e2 >= dy) { err += dy; x1 += sx; }
            if (e2 <= dx) { err += dx; y1 += sy; }
        }
    } else {
        // Draw marker characters for other shapes (circle, triangle, etc.)
        char marker = s.type[0];
        drawPoint(s.x1, s.y1, marker);
        drawPoint(s.x2, s.y2, marker);
    }
}

// Redraws all shapes onto a clean canvas
void redrawCanvas() {
    initCanvas();
    for (int i = 0; i < shape_count; i++) {
        drawShape(shapes[i]);
    }
}

// Adds a shape to the global list, assigns unique ID, and draws it
void addShape(const char* type, int x1, int y1, int x2, int y2) {
    if (shape_count >= shape_capacity) {
        shape_capacity = shape_capacity == 0 ? 4 : shape_capacity * 2;
        Shape* temp = realloc(shapes, shape_capacity * sizeof(Shape));
        if (temp == NULL) {
            fprintf(stderr, "Error: Memory allocation failed!\n");
            return;
        }
        shapes = temp;
    }
    
    Shape s;
    s.id = ++next_id;
    strncpy(s.type, type, sizeof(s.type) - 1);
    s.type[sizeof(s.type) - 1] = '\0';
    s.x1 = x1;
    s.y1 = y1;
    s.x2 = x2;
    s.y2 = y2;
    
    shapes[shape_count++] = s;
    
    // Draw the newly added shape
    drawShape(s);
}

// Deletes a shape by ID, clears the canvas, and redraws the remaining shapes
void deleteShape(int id) {
    int index_to_remove = -1;
    for (int i = 0; i < shape_count; i++) {
        if (shapes[i].id == id) {
            index_to_remove = i;
            break;
        }
    }
    
    if (index_to_remove != -1) {
        // Shift remaining shapes to fill the gap
        for (int i = index_to_remove; i < shape_count - 1; i++) {
            shapes[i] = shapes[i + 1];
        }
        shape_count--;
        
        // Trigger clear and redraw
        redrawCanvas();
    }
}

int main() {
    initCanvas();
    
    printf("--- Canvas Initial State ---\n");
    displayCanvas();
    printf("\n");
    
    printf("--- Adding Rectangle (ID: 1) from (5, 2) to (25, 10) ---\n");
    addShape("rectangle", 5, 2, 25, 10);
    displayCanvas();
    printf("\n");
    
    printf("--- Adding Line (ID: 2) from (30, 15) to (55, 5) ---\n");
    addShape("line", 30, 15, 55, 5);
    displayCanvas();
    printf("\n");
    
    printf("--- Deleting Rectangle (ID: 1) (Triggers redraw) ---\n");
    deleteShape(1);
    displayCanvas();
    printf("\n");
    
    // Free allocated memory
    free(shapes);
    
    return 0;
}
