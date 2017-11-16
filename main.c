#include <stdio.h>
#include <math.h>
#include <complex.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>

#define MAX_ITERATIONS 500

#define WIDTH 650
#define HEIGHT 450

//n = 0 -> not in set
//n = -1 -> no escapes
int get_iterations(complex long double c) {
	if (cabs(c) >= 2) return 0;

	complex long double z = 0;
	complex long double prevz = 0;
	
	int n;
	for (n = 1; n < MAX_ITERATIONS; n++) {
		//  z²  + c
		prevz = z;
		z = z*z + c;
		if (prevz == z) return -1;
		if (cabs(z) >= 2) {
//			printf("blowd up\n");
			return n;
		}
	}

//	printf("dindt blow up\n");

	return -1;
}

double scale = 100;
long double pos_x = -200.0;
long double pos_y = -200.0;
double drawpos_x = 0.0;
double drawpos_y = 0.0;

ALLEGRO_DISPLAY* disp;
ALLEGRO_EVENT_QUEUE *event_queue;

ALLEGRO_BITMAP* set = NULL;
ALLEGRO_FONT* font = NULL;

ALLEGRO_COLOR palette[0x100];

void draw(int redraw) {
	if (redraw) {
		drawpos_x = 0;
		drawpos_y = 0;
		al_clear_to_color(al_map_rgb(255,255,255));

		ALLEGRO_BITMAP* backbuf = al_get_backbuffer(disp);

		//NEEDED so al_put_pixel doesnt slow
		al_lock_bitmap(backbuf, ALLEGRO_PIXEL_FORMAT_ANY, ALLEGRO_LOCK_WRITEONLY);

		for (int px = 0; px < WIDTH; px++) {
			for (int py = 0; py < HEIGHT; py++) {
				//printf("%i, %i\n",px,py);
				long double a, b;
				a = ((double)px + pos_x)/(scale);
				b = ((double)py + pos_y)/(scale);
				//printf("%f, %f\n",a,b);
				int n = get_iterations(a + b*I);
				uint8_t col = 255 - (n > -1 ? n*(255/10) : 255); //gray scale value
				ALLEGRO_COLOR color = 
					//n <= 0 ? al_map_rgb(0,0,0) : palette[col];
					al_map_rgb(col,col,col);
				
				al_put_pixel(px,py, color);
			}
		}
		al_unlock_bitmap(backbuf);
		al_flip_display();
		if (set) al_destroy_bitmap(set);
		set = al_clone_bitmap(backbuf);
		//printf("drawn\n");
		return;
	}
	
	al_clear_to_color(al_map_rgb(255,255,255));

	if (set)
		al_draw_bitmap(set, drawpos_x, drawpos_y, 0);
	al_draw_multiline_textf(font, al_map_rgb(100,100,100), 2,2, 1000, 10, 0, "%llf + %llfi\nzoom: %lf", pos_x/scale, -pos_y/scale, scale);
	al_flip_display();
}

void process_event(ALLEGRO_EVENT* ev);

int ended = 0; //program end;
int scroll = 0; //mouse

void gen_palette(void);

int autozoom = 0;

int main() {
	al_init();
	al_install_keyboard();
	al_install_mouse();
	font = al_create_builtin_font();

	disp = al_create_display(WIDTH,HEIGHT);
	event_queue = al_create_event_queue();

    al_register_event_source(event_queue, al_get_keyboard_event_source());
    al_register_event_source(event_queue, al_get_mouse_event_source());
    al_register_event_source(event_queue, al_get_display_event_source(disp));
	
	gen_palette();
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
		if (wheel == scroll && (autozoom ? 1 : wheel)) {
			double prevscale = scale;
			scale = (scale+scroll*10)+log(scale)*log(scale);
			scale = scale < 0 ? 10/-scale : scale;
			scroll = 0;
			al_set_mouse_z(0);
			if (scale != prevscale) {
				double factor = prevscale/scale;
				pos_x -= (WIDTH/2 + pos_x*factor) * (factor-1);
				pos_y -= (HEIGHT/2 + pos_y*factor) * (factor-1);
				draw(1);
			}
		}
		scroll = wheel;
		if (abs(pos_x) == INFINITY)
			pos_x = 0;
		if (abs(pos_y) == INFINITY)
			pos_y = 0;
		if (scale < 0.1) scale = 0.1;
		draw(0);
	}

	al_destroy_event_queue(event_queue);
    al_destroy_display(disp);
}

struct {unsigned char r,g,b;} mapping[] = {
	{66,30,15},//brown3
	{25,7,26},//darkviolett
	{9,1,47},//darkestblue
	{4,4,73},//blue5
	{0,71,0},//blue4
	{12,44,138},//blue3
	{24,82,177},//blue2
	{57,125,209},//blue1
	{134,181,229},//blue0
	{211,236,248},//lightestblue
	{241,233,191},//lightestyellow
	{248,201,95},//lightyellow
	{255,170,0},//dirtyyellow
	{204,128,0},//brown0
	{153,87,0},//brown1
	{106,52,3}//brown2
};

void gen_palette(void) {
	for (int i = 0; i < 0x100; i++) {
		//TODO cool algrothitim
		palette[i] = al_map_rgb_f(mapping[i%16].r/255.0,mapping[i%16].g/255.0,mapping[i%16].b/255.0);
	}
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
					pos_x = -200.0;
					pos_y = -200.0;
					drawpos_x = 0.0;
					drawpos_y = 0.0;
					draw(1);
					break;
				case ALLEGRO_KEY_1: {
					double prevscale = scale;
					scale *= 10;
					scale = scale < 0 ? 10/-scale : scale;
					scroll = 0;
					double factor = prevscale/scale;
					pos_x -= (WIDTH/2 + pos_x) * (factor-1);
					pos_y -= (HEIGHT/2 + pos_y) * (factor-1);
					draw(1);
					} break;
				case ALLEGRO_KEY_2:
					scale/=10;
					draw(1);
					break;
				case ALLEGRO_KEY_A:
					autozoom = !autozoom;
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
