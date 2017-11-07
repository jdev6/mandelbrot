#include <stdio.h>
#include <math.h>
#include <complex.h>
#include <allegro5/allegro.h>

#define MAX_ITERATIONS 100

#define WIDTH 600
#define HEIGHT 400

//returns -1 wghen doesnt escape;
int get_iterations(complex double c) {
	complex double z = 0;
	
	int n;
	for (n = 1; n < MAX_ITERATIONS; n++) {
		//  z²  + c
		z = z*z + c;
		if (cabs(z) >= 2) {
//			printf("blowd up\n");
			return n;
		}
	}

//	printf("dindt blow up\n");

	return -1;
}

double scale = 100;
double pos_x = -100.0;
double pos_y = -100.0;
double drawpos_x = 0.0;
double drawpos_y = 0.0;

ALLEGRO_DISPLAY* disp;
ALLEGRO_EVENT_QUEUE *event_queue;

ALLEGRO_BITMAP* set = NULL;

void draw(int redraw) {
	if (redraw) {
		drawpos_x = 0;
		drawpos_y = 0;
		al_clear_to_color(al_map_rgb(255,255,255));

		for (int px = 0; px < WIDTH; px++) {
			for (int py = 0; py < HEIGHT; py++) {
				//printf("%i, %i\n",px,py);
				double a, b;
				a = 1/scale * ((double)px + pos_x);
				b = 1/scale * ((double)py + pos_y);
				//printf("%f, %f\n",a,b);
				int n = get_iterations(a + b*I);
				int col = 255 - (n > -1 ? n*(255/20) : 255);
				al_put_pixel(px,py, al_map_rgb(col,col,col));
			}
		}
		al_flip_display();
		if (set) al_destroy_bitmap(set);
		set = al_clone_bitmap(al_get_backbuffer(disp));
		printf("drawn\n");
	}
	
	al_clear_to_color(al_map_rgb(255,255,255));

	if (set) {
		al_draw_bitmap(set, drawpos_x, drawpos_y, 0);
	}

	al_flip_display();

}

void process_event(ALLEGRO_EVENT* ev);

int ended = 0;
int scroll = 0;

int main() {
	al_init();
	al_install_keyboard();
	al_install_mouse();

	disp = al_create_display(600,400);
	event_queue = al_create_event_queue();

    al_register_event_source(event_queue, al_get_keyboard_event_source());
    al_register_event_source(event_queue, al_get_mouse_event_source());
    al_register_event_source(event_queue, al_get_display_event_source(disp));
	draw(1);

	while (!ended) {
        ALLEGRO_EVENT ev;
		int get_event = al_wait_for_event_timed(event_queue, &ev, 1.0/30.0);
		if (get_event) process_event(&ev);
		while ((get_event = al_get_next_event(event_queue, &ev))) {
			if (get_event) process_event(&ev);
		}
		ALLEGRO_MOUSE_STATE st;
		al_get_mouse_state(&st);
		int wheel = al_get_mouse_state_axis(&st, 2);
		if (wheel == scroll) {
			double prevscale = scale;
			scale += scroll*10;
			scale = scale < 0 ? 10/-scale : scale;
			scroll = 0;
			al_set_mouse_z(0);
			if (scale != prevscale) draw(1);
		}
		scroll = wheel;

		draw(0);
	}

	al_destroy_event_queue(event_queue);
    al_destroy_display(disp);
}

void process_event(ALLEGRO_EVENT* ev) {
	switch (ev->type) {
        case ALLEGRO_EVENT_KEY_DOWN:
			switch (ev->keyboard.keycode) {
				case ALLEGRO_KEY_ESCAPE: 
					ended = 1;
					break;
				case ALLEGRO_KEY_R:
					scale = 100;
					pos_x = -100.0;
					pos_y = -100.0;
					drawpos_x = 0.0;
					drawpos_y = 0.0;
					draw(1);
					break;
			}
			break;

		case ALLEGRO_EVENT_MOUSE_BUTTON_DOWN:
			switch (ev->mouse.button) {
				case 2:
					draw(1);
					break;
			}
		
		case ALLEGRO_EVENT_MOUSE_AXES: {
			ALLEGRO_MOUSE_STATE st;
			al_get_mouse_state(&st);
			if (al_mouse_button_down(&st,1)) {
				pos_x -= ev->mouse.dx;
				drawpos_x += ev->mouse.dx;
				pos_y -= ev->mouse.dy;	
				drawpos_y += ev->mouse.dy;	
			}
			} break;
	}
}
