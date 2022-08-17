#include <iostream>
#include <string>
#include <algorithm>

#include "MyButton.h"

#include "graphics.h"

#pragma comment(lib, "Msimg32.lib")

const int MAX_RECTANGLES = 20;
const int x_start = 100;
const int y_start = 400;
const int RECT_WIDTH = 20;
const int MAX_HEIGHT = 200;

const int WIDTH = 800;
const int HEIGHT = 700;

int arr[MAX_RECTANGLES];


void draw_rectangles(int color);            
bool is_sorted();                           
void bubble_sort_iteration(int j);          
void random_fill_array();                  


void random_shuffle();                      
int random_colour();                         


void draw_rectangles(int color) {
    setfillstyle(SOLID_FILL, color);
    int x = x_start;
    for (int i = 0; i < MAX_RECTANGLES; ++i) {
        bar(x, y_start - arr[i], x + RECT_WIDTH, y_start);
        x += RECT_WIDTH;
    }
}

void random_fill_array() {
    for (int i = 0; i < MAX_RECTANGLES; ++i) {
        arr[i] = rand() % MAX_HEIGHT;
    }
}

bool is_sorted() {
    for (int i = 0; i < MAX_RECTANGLES - 1; ++i) {
        if (arr[i] > arr[i + 1]) {
            return false;
        }
    }
    return true;
}

void bubble_sort_iteration(int j) {
    if (arr[j] > arr[j + 1]) {
        std::swap(arr[j], arr[j + 1]);
    }
}

int random_colour() {
    int r = rand() % 256;
    int g = rand() % 256;
    int b = rand() % 256;
    return COLOR(r, g, b);
}

void random_shuffle() {
    for (int i = 0; i < MAX_RECTANGLES; ++i) {
        std::swap(arr[i], arr[std::rand() % MAX_RECTANGLES]);
    }
}



int main() {
    srand(time(0));
    random_fill_array();

    initwindow(WIDTH, HEIGHT, "Bubble sort  visualization", 0, 0, true);
    create_control(0, HEIGHT - 100, 100, HEIGHT, CHANGE_COLOR);
    create_control(100, HEIGHT - 100, 200, HEIGHT, ARRAY_SHUFFLE);
    create_control(200, HEIGHT - 100, 300, HEIGHT, REFILL_ARRAY);

    int pos = 0;
    int colour = RGB(57, 47, 80);

    while (true) {
        setbkcolor(BLACK);
        clearviewport();

        draw_rectangles(colour);
        draw_buttons();


        if (!is_sorted()) {
            bubble_sort_iteration(pos);
            pos = (pos + 1) % (MAX_RECTANGLES - 1);
        }


        if (mousebuttons() == 1) {
            int button = select_control();

            if (button == CHANGE_COLOR) {            
                colour = random_colour();
            }
            else if (button == ARRAY_SHUFFLE) {     
                random_shuffle();
            }
            else if (button == REFILL_ARRAY) {       
                random_fill_array();
            }
        }

        swapbuffers();
        delay(10);


        if (kbhit()) {
            int key = getch();
            if (key == KEY_ESC) {
                break;
            }
        }
    }
}
