#include "MyButton.h"
#include <iostream>

#include "graphics.h"
#pragma comment(lib, "Msimg32.lib")

MyBtn controls[N_CONTROLS];

void create_control(int left, int top, int right, int bottom, int type) {

	//Создаем кнопку
	MyBtn control;
	control.left = left;
	control.top = top;
	control.right = right;
	control.bottom = bottom;

	//type - именованная константа, она же индекс для массива controls
	controls[type] = control;
}

void draw_buttons() {
	setfillstyle(SOLID_FILL, WHITE);
	setcolor(BLUE);
	for (int i = 0; i < N_CONTROLS; ++i) {
		bar(controls[i].left, controls[i].top, controls[i].right, controls[i].bottom);
		rectangle(controls[i].left, controls[i].top, controls[i].right, controls[i].bottom);
	}
}

int select_control() {
	int x = mousex();
	int y = mousey();

	for (int i = 0; i < N_CONTROLS; ++i) {
		if (x >= controls[i].left && x <= controls[i].right
			&& y >= controls[i].top && y <= controls[i].bottom) {
			return i;
		}
	}
	return NONE;
}