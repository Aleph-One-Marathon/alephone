/*
 *  sdl_widgets.h - Widgets for SDL dialogs
 *
 *  Written in 2000 by Christian Bauer
 */

#ifndef SDL_WIDGETS_H
#define SDL_WIDGETS_H

union SDL_Event;
struct SDL_Surface;
struct sdl_font_info;


/*
 *  Widget base class
 */

class widget {
	friend class dialog;

public:
	widget() : active(false), dirty(false), font(NULL) {}
	widget(int font);
	virtual ~widget() {}

	// Layout widget: calculate x position relative to dialog center (-> rect.x)
	// and return effective height of widget
	virtual int layout(void);

	// Draw widget
	virtual void draw(SDL_Surface *s) const = 0;

	// Handle event
	virtual void click(int x, int y) {}
	virtual void event(SDL_Event &e) {}

	// Widget selectable?
	virtual bool is_selectable(void) const {return true;}

protected:
	SDL_Rect rect;	// Position relative to dialog surface, and dimensions

	bool active;	// Flag: widget active
	bool dirty;		// Flag: widget needs redraw

	const sdl_font_info *font;	// Widget font
	uint16 style;				// Widget font style
};


/*
 *  Vertical space
 */

class w_spacer : public widget {
public:
	w_spacer(int height = 8) {rect.w = 0; rect.h = height;}

	void draw(SDL_Surface *s) const {}
	bool is_selectable(void) const {return false;}
};


/*
 *  Static text
 */

class w_static_text : public widget {
public:
	w_static_text(const char *text, int font = MESSAGE_FONT, int color = MESSAGE_COLOR);

	void draw(SDL_Surface *s) const;
	bool is_selectable(void) const {return false;}

private:
	const char *text;
	int color;
};


/*
 *  Picture (PICT resource)
 */

class w_pict : public widget {
public:
	w_pict(int id);
	~w_pict();

	void draw(SDL_Surface *s) const;
	bool is_selectable(void) const {return false;}

private:
	SDL_Surface *picture;
};


/*
 *  Button
 */

typedef void (*action_proc)(void *);

class w_button : public widget {
public:
	w_button(const char *text, action_proc proc, void *arg);

	void draw(SDL_Surface *s) const;
	void click(int x, int y);

protected:
	const char *text;
	action_proc proc;
	void *arg;
};


/*
 *  Button on left/right side of dialog box (always use in pairs, left button first)
 */

class w_left_button : public w_button {
public:
	w_left_button(const char *text, action_proc proc, void *arg) : w_button(text, proc, arg) {}

	int layout(void);
};

class w_right_button : public w_button {
public:
	w_right_button(const char *text, action_proc proc, void *arg) : w_button(text, proc, arg) {}

	int layout(void);
};


/*
 *  Selection widget (base class)
 */

class w_select : public widget {
public:
	w_select(const char *name, int selection, const char **labels);

	int layout(void);
	void draw(SDL_Surface *s) const;
	void click(int x, int y);
	void event(SDL_Event &e);

	int get_selection(void) const {return selection;}
	void set_selection(int selection);

protected:
	virtual void selection_changed(void);

	const char *name;

	const char **labels;
	int num_labels;

	int selection;			// -1 means unknown selection
	int label_x;			// X offset of label display
};


/*
 *  On-off toggle
 */

class w_toggle : public w_select {
public:
	w_toggle(const char *name, bool selection, const char **labels = onoff_labels);

	static const char *onoff_labels[3];
};


/*
 *  Player color selection
 */

class w_player_color : public w_select {
public:
	w_player_color(const char *name, int selection) : w_select(name, selection, color_labels) {}

	void draw(SDL_Surface *s) const;

	static const char *color_labels[9];
};


/*
 *  Text entry widget
 */

class w_text_entry : public widget {
public:
	w_text_entry(const char *name, int max_chars, const char *initial_text = NULL);
	~w_text_entry();

	int layout(void);
	void draw(SDL_Surface *s) const;
	void event(SDL_Event &e);

	void set_text(const char *text);
	const char *get_text(void) {return buf;}

protected:
	char *buf;		// Text entry buffer

private:
	void modified_text(void);

	const char *name;

	const sdl_font_info *text_font;	// Font for text
	uint16 text_style;

	int num_chars;		// Length of text in buffer
	int max_chars;		// Maximum number of chars in buffer
	int text_x;			// X offset of text display
	int max_text_width;	// Maximum width of text display
};


/*
 *  Number entry widget
 */

class w_number_entry : public w_text_entry {
public:
	w_number_entry(const char *name, int initial_number = 0);

	void event(SDL_Event &e);

	void set_number(int number);
	int get_number(void) {return atoi(get_text());}
};


/*
 *  Key name widget
 */

class w_key : public widget {
public:
	w_key(const char *name, SDLKey key);

	int layout(void);
	void draw(SDL_Surface *s) const;
	void click(int x, int y);
	void event(SDL_Event &e);

	void set_key(SDLKey key);
	SDLKey get_key(void) {return key;}

private:
	const char *name;

	int key_x;			// X offset of key name

	SDLKey key;
	bool binding;		// Flag: next key press will bind key
};

#endif
