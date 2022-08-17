#ifndef _CONTROL_H
#define _CONTROL_H

//��������� "������"
struct MyBtn {
	int left;
	int top;
	int right;
	int bottom;
};

//����������� ��������� ��� ������
enum control_values {
	NONE = -1, CHANGE_COLOR, ARRAY_SHUFFLE, REFILL_ARRAY, N_CONTROLS
};

void create_control(int left, int top, int right, int bottom, int type);
void draw_buttons();
int select_control();

#endif