/*
 *  sdl_dialogs.cpp - SDL implementation of user dialogs
 *
 *  Written in 2000 by Christian Bauer
 */

#include "cseries.h"
#include "sdl_dialogs.h"
#include "sdl_widgets.h"

#include "shape_descriptors.h"
#include "screen_drawing.h"
#include "shell.h"
#include "screen.h"
#include "world.h"
#include "mysound.h"


// Dialog border width
const int DIALOG_BORDER = 8;

// Global variables
static SDL_Surface *dialog_surface = NULL;

static const sdl_font_info *dialog_font[NUM_DIALOG_FONTS];

static TextSpec dialog_font_spec[NUM_DIALOG_FONTS] = {
	{kFontIDMonaco, bold, 18}, // TITLE_FONT
	{kFontIDMonaco, bold, 18}, // BUTTON_FONT
	{kFontIDMonaco, bold, 12}, // NORMAL_FONT
	{kFontIDMonaco, bold, 12}, // MESSAGE_FONT
	{22, normal, 14}           // TEXT_ENTRY_FONT
};

static SDL_Color dialog_color[NUM_DIALOG_COLORS] = {
	{0x00, 0x00, 0x00}, // BLACK COLOR
	{0xc0, 0xc0, 0xc0}, // TITLE_COLOR
	{0xc0, 0x00, 0x00}, // BUTTON_COLOR
	{0xff, 0x80, 0x80}, // BUTTON_ACTIVE_COLOR
	{0x20, 0x20, 0xff}, // LABEL_COLOR
	{0x80, 0x80, 0xff}, // LABEL_ACTIVE_COLOR
	{0x40, 0xff, 0x40}, // ITEM_COLOR
	{0xff, 0xff, 0xff}, // ITEM_ACTIVE_COLOR
	{0x40, 0x40, 0x40}, // DISABLED_COLOR
	{0xff, 0xff, 0xff}  // MESSAGE_COLOR
};


/*
 *  Initialize dialog manager
 */

void initialize_dialogs(void)
{
	// Allocate surface for dialogs (this surface is needed because when
	// OpenGL is active, we can't write directly to the screen)
	dialog_surface = SDL_CreateRGBSurface(SDL_SWSURFACE, 640, 480, 16, 0x7c00, 0x03e0, 0x001f, 0);
	assert(dialog_surface);

	// Load fonts
	for (int i=0; i<NUM_DIALOG_FONTS; i++)
		dialog_font[i] = load_font(dialog_font_spec[i]);
}


/*
 *  Get dialog font/color
 */

const sdl_font_info *get_dialog_font(int which, uint16 &style)
{
	assert(which >= 0 && which < NUM_DIALOG_FONTS);
	style = dialog_font_spec[which].style;
	return dialog_font[which];
}

uint32 get_dialog_color(SDL_Surface *s, int which)
{
	assert(which >= 0 && which < NUM_DIALOG_COLORS);
	return SDL_MapRGB(s->format, dialog_color[which].r, dialog_color[which].g, dialog_color[which].b);
}


/*
 *  Dialog destructor
 */

dialog::~dialog()
{
	// Free all widgets
	vector<widget *>::const_iterator i = widgets.begin(), end = widgets.end();
	while (i != end) {
		delete *i;
		i++;
	}
}


/*
 *  Add widget
 */

void dialog::add(widget *w)
{
	widgets.push_back(w);
}


/*
 *  Layout dialog
 */

void dialog::layout()
{
	// Layout all widgets, calculate total width and height
	int y = DIALOG_BORDER;
	int left = 0;
	int right = 0;
	vector<widget *>::const_iterator i = widgets.begin(), end = widgets.end();
	while (i != end) {
		widget *w = *i;
		w->rect.y = y;
		y += w->layout();
		if (w->rect.x < left)
			left = w->rect.x;
		if (w->rect.x + w->rect.w > right)
			right = w->rect.x + w->rect.w;
		i++;
	}
	left = abs(left);
	rect.w = (left > right ? left : right) * 2 + DIALOG_BORDER * 2;
	rect.h = y + DIALOG_BORDER;

	// Center dialog on video surface
	SDL_Surface *video = SDL_GetVideoSurface();
	rect.x = (video->w - rect.w) / 2;
	rect.y = (video->h - rect.h) / 2;

	// Transform positions of all widgets to be relative to the dialog surface
	i = widgets.begin();
	while (i != end) {
		(*i)->rect.x += rect.w / 2;
		i++;
	}
}


/*
 *  Update part of dialog on screen
 */

void dialog::update(SDL_Rect r) const
{
	SDL_Surface *video = SDL_GetVideoSurface();
	SDL_Rect dst_rect = {r.x + rect.x, r.y + rect.y, r.w, r.h};
	SDL_BlitSurface(dialog_surface, &r, video, &dst_rect);
	SDL_UpdateRects(video, 1, &dst_rect);
#ifdef HAVE_OPENGL
	if (video->flags & SDL_OPENGL)
		SDL_GL_SwapBuffers();
#endif
}


/*
 *  Draw dialog
 */

void dialog::draw_widget(widget *w, bool do_update) const
{
	// Clear and redraw widget
	SDL_FillRect(dialog_surface, &w->rect, get_dialog_color(dialog_surface, BLACK_COLOR));
	w->draw(dialog_surface);
	w->dirty = false;

	// Blit to screen
	if (do_update)
		update(w->rect);
}

void dialog::draw(void) const
{
	// Draw border
	{
		uint32 light_pixel = SDL_MapRGB(dialog_surface->format, 0xe0, 0xe0, 0xe0);
		uint32 medium_pixel = SDL_MapRGB(dialog_surface->format, 0x80, 0x60, 0x60);
		uint32 dark_pixel = SDL_MapRGB(dialog_surface->format, 0x40, 0x40, 0x40);

		SDL_Rect r = {0, 0, rect.w, 1};
		SDL_FillRect(dialog_surface, &r, light_pixel);
		r.x++; r.y++; r.w -= 2;
		SDL_FillRect(dialog_surface, &r, medium_pixel);
		r.x++; r.y++; r.w -= 2;
		SDL_FillRect(dialog_surface, &r, dark_pixel);

		r.y = rect.h - 3;
		SDL_FillRect(dialog_surface, &r, light_pixel);
		r.x--; r.y++; r.w += 2;
		SDL_FillRect(dialog_surface, &r, medium_pixel);
		r.x--; r.y++; r.w += 2;
		SDL_FillRect(dialog_surface, &r, dark_pixel);

		r.x = 0; r.y = 0; r.w = 1; r.h = rect.h - 1;
		SDL_FillRect(dialog_surface, &r, light_pixel);
		r.x++; r.y++; r.h -= 2;
		SDL_FillRect(dialog_surface, &r, medium_pixel);
		r.x++; r.y++; r.h -= 2;
		SDL_FillRect(dialog_surface, &r, dark_pixel);

		r.x = rect.w - 3;
		SDL_FillRect(dialog_surface, &r, light_pixel);
		r.x++; r.y--; r.h += 2;
		SDL_FillRect(dialog_surface, &r, medium_pixel);
		r.x++; r.y--; r.h += 2;
		SDL_FillRect(dialog_surface, &r, dark_pixel);
	}

	// Draw all widgets
	vector<widget *>::const_iterator i = widgets.begin(), end = widgets.end();
	while (i != end) {
		draw_widget(*i, false);
		i++;
	}

	// Blit to screen
	SDL_Rect r = {0, 0, rect.w, rect.h};
	update(r);
}


/*
 *  Activate widget
 */

void dialog::activate_widget(int num, bool draw)
{
	if (num == active_widget_num)
		return;
	if (!widgets[num]->is_selectable())
		return;

	// Deactivate previously active widget
	if (active_widget) {
		active_widget->active = false;
		if (draw)
			draw_widget(active_widget);
	}

	// Activate new widget
	active_widget = widgets[num];
	active_widget_num = num;
	active_widget->active = true;
	if (draw) {
		draw_widget(active_widget);
//		play_sound(DIALOG_SELECT_SOUND, NULL, NONE);
	}
}


/*
 *  Activate first selectable widget (don't draw)
 */

void dialog::activate_first_widget(void)
{
	for (int i=0; i<widgets.size(); i++) {
		if (widgets[i]->is_selectable()) {
			activate_widget(i, false);
			break;
		}
	}
}


/*
 *  Activate next/previous selectable widget
 */

void dialog::activate_next_widget(void)
{
	int i = active_widget_num;
	do {
		i++;
		if (i >= widgets.size())
			i = 0;
	} while (!widgets[i]->is_selectable());
	activate_widget(i);
}

void dialog::activate_prev_widget(void)
{
	int i = active_widget_num;
	do {
		i--;
		if (i < 0)
			i = widgets.size() - 1;
	} while (!widgets[i]->is_selectable());
	activate_widget(i);
}


/*
 *  Find widget given video surface coordinates (<0 = none found)
 */

int dialog::find_widget(int x, int y)
{
	// Transform to dialog coordinates
	x -= rect.x;
	y -= rect.y;

	// Find widget
	vector<widget *>::const_iterator i = widgets.begin(), end = widgets.end();
	int num = 0;
	while (i != end) {
		widget *w = *i;
		if (x >= w->rect.x && y >= w->rect.y && x < w->rect.x + w->rect.w && y < w->rect.y + w->rect.h)
			return num;
		i++; num++;
	}
	return -1;
}


/*
 *  Handle event
 */

void dialog::event(SDL_Event &e)
{
	// First pass event to active widget (which may modify it)
	if (active_widget)
		active_widget->event(e);

	// Remaining events handled by dialog
	switch (e.type) {

		// Key pressed
		case SDL_KEYDOWN:
			switch (e.key.keysym.sym) {
				case SDLK_ESCAPE:		// ESC = Exit dialog
					quit(-1);
					break;
				case SDLK_UP:			// Up = Activate previous widget
				case SDLK_PAGEUP:
				case SDLK_LEFT:
					activate_prev_widget();
					break;
				case SDLK_DOWN:			// Down = Activate next widget
				case SDLK_PAGEDOWN:
				case SDLK_RIGHT:
					activate_next_widget();
					break;
				case SDLK_RETURN:		// Return = Action on widget
					active_widget->click(0, 0);
					break;
				case SDLK_F9:			// F9 = Screen dump
					dump_screen();
					break;
			}
			break;

		// Mouse button pressed
		case SDL_MOUSEBUTTONDOWN: {
			int x = e.button.x, y = e.button.y;
			int num = find_widget(x, y);
			if (num >= 0) {
				widget *w = widgets[num];
				w->click(x - rect.x - w->rect.x, y - rect.x - w->rect.y);
			}
			break;
		}

		// Mouse moved
		case SDL_MOUSEMOTION: {
			int x = e.motion.x, y = e.motion.y;
			int num = find_widget(x, y);
			if (num >= 0)
				activate_widget(num);
			break;
		}

		// Quit requested
		case SDL_QUIT:
			exit(0);
			break;
	}

	// Redraw dirty widgets
	for (int i=0; i<widgets.size(); i++)
		if (widgets[i]->dirty)
			draw_widget(widgets[i]);
}


/*
 *  Run dialog modally, returns result code (0 = ok, -1 = cancel)
 */

int dialog::run(bool intro_exit_sounds)
{
	// Clear dialog surface
	SDL_FillRect(dialog_surface, NULL, get_dialog_color(dialog_surface, BLACK_COLOR));

	// Activate first widget
	activate_first_widget();

	// Layout and draw dialog
	layout();
	draw();

	// Welcome sound
	if (intro_exit_sounds)
		play_sound(DIALOG_INTRO_SOUND, NULL, NONE);

	// Enable unicode key translation
	SDL_EnableUNICODE(true);

	// Dialog event loop
	result = 0;
	done = false;
	while (!done) {

		// Get next event
		SDL_Event e;
		e.type = SDL_NOEVENT;
		SDL_PollEvent(&e);

		// Handle event
		event(e);

		// Give time to system
		if (e.type == SDL_NOEVENT) {
			global_idle_proc();
			SDL_Delay(10);
		}
	}

	// Disable unicode key translation
	SDL_EnableUNICODE(false);

	// Farewell sound
	if (intro_exit_sounds)
		play_sound(result == 0 ? DIALOG_OK_SOUND : DIALOG_CANCEL_SOUND, NULL, NONE);

	// Clear dialog surface
	SDL_FillRect(dialog_surface, NULL, get_dialog_color(dialog_surface, BLACK_COLOR));

	// Erase dialog from screen
	SDL_Surface *video = SDL_GetVideoSurface();
	SDL_FillRect(video, &rect, get_dialog_color(video, BLACK_COLOR));
	SDL_UpdateRects(video, 1, &rect);
#ifdef HAVE_OPENGL
	if (video->flags & SDL_OPENGL)
		SDL_GL_SwapBuffers();
#endif

	return result;
}


/*
 *  Quit dialog, return result
 */

void dialog::quit(int r)
{
	result = r;
	done = true;
}


/*
 *  Standard callback functions
 */

void dialog_ok(void *arg)
{
	dialog *d = (dialog *)arg;
	d->quit(0);
}

void dialog_cancel(void *arg)
{
	dialog *d = (dialog *)arg;
	d->quit(-1);
}
