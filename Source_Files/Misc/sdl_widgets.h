/*
 *  sdl_widgets.h - Widgets for SDL dialogs
 *
 *  Written in 2000 by Christian Bauer
 */

#ifndef SDL_WIDGETS_H
#define SDL_WIDGETS_H

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
	virtual void mouse_move(int x, int y) {}
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
	w_spacer(int height = get_dialog_space(SPACER_HEIGHT)) {rect.w = 0; rect.h = height;}

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

typedef void (&action_proc)(void *);

class w_button : public widget {
public:
	w_button(const char *text, action_proc proc, void *arg);
	~w_button();

	void draw(SDL_Surface *s) const;
	void click(int x, int y);

protected:
	const char *text;
	action_proc proc;
	void *arg;

	SDL_Surface *button_l, *button_c, *button_r;
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
 *  Selection button
 */

class w_select_button : public widget {
public:
	w_select_button(const char *name, const char *selection, action_proc proc, void *arg);

	int layout(void);
	void draw(SDL_Surface *s) const;
	void click(int x, int y);

	void set_selection(const char *selection);

private:
	const char *name, *selection;
	action_proc proc;
	void *arg;

	int selection_x;			// X offset of selection display
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
	static const char *onoff_labels[3];

	w_toggle(const char *name, bool selection, const char **labels = onoff_labels);
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

	virtual void set_key(SDLKey key);
	SDLKey get_key(void) {return key;}

private:
	const char *name;

	int key_x;			// X offset of key name

	SDLKey key;
	bool binding;		// Flag: next key press will bind key
};


/*
 *  Slider
 */

class w_slider : public widget {
public:
	w_slider(const char *name, int num_items, int sel);
	~w_slider();

	int layout(void);
	void draw(SDL_Surface *s) const;
	void mouse_move(int x, int y);
	void click(int x, int y);
	void event(SDL_Event &e);

	int get_selection(void) {return selection;}
	void set_selection(int selection);

	virtual void item_selected(void) {}

protected:
	const char *name;

	int slider_x;			// X offset of slider image

	int selection;			// Currently selected item
	int num_items;			// Total number of items

	bool thumb_dragging;	// Flag: currently dragging thumb
	int thumb_x;			// X position of thumb
	int trough_width;		// Width of trough

	int thumb_drag_x;		// X start position when dragging

	SDL_Surface *slider_l, *slider_c, *slider_r, *thumb;
};


/*
 *  Template for list selection widgets
 */

class w_list_base : public widget {
public:
	w_list_base(int width, int lines, int sel);
	~w_list_base();

	int layout(void);
	void draw(SDL_Surface *s) const;
	void mouse_move(int x, int y);
	void click(int x, int y);
	void event(SDL_Event &e);

	int get_selection(void) {return selection;}

	virtual bool is_item_selectable(int i) {return true;}
	virtual void item_selected(void) = 0;

protected:
	virtual void draw_items(SDL_Surface *s) const = 0;
	void draw_image(SDL_Surface *dst, SDL_Surface *s, int x, int y) const;
	void set_selection(int s);
	void new_items(void);
	void center_item(int i);
	void set_top_item(int i);

	int selection;			// Currently selected item
	int font_height;		// Height of font

	int num_items;			// Total number of items
	int shown_items;		// Number of shown items
	int top_item;			// Number of first visible item

	bool thumb_dragging;	// Flag: currently dragging scroll bar thumb
	SDL_Rect trough_rect;	// Dimensions of trough
	int thumb_height;		// Height of thumb
	int min_thumb_height;	// Minimal height of thumb
	int thumb_y;			// Y position of thumb

	int thumb_drag_y;		// Y start position when dragging

	SDL_Surface *frame_tl, *frame_t, *frame_tr, *frame_l, *frame_r, *frame_bl, *frame_b, *frame_br;
	SDL_Surface *thumb_t, *thumb_tc, *thumb_c, *thumb_bc, *thumb_b;
};

template <class T>
class w_list : public w_list_base {
public:
	w_list(const vector<T> &it, int width, int lines, int sel) : w_list_base(width, lines, sel), items(it)
	{
		num_items = items.size();
		new_items();
		set_selection(sel);
		center_item(selection);
	}

	~w_list() {}

protected:
	void draw_items(SDL_Surface *s) const
	{
		vector<T>::const_iterator i = items.begin() + top_item;
		int x = rect.x + get_dialog_space(LIST_L_SPACE);
		int y = rect.y + get_dialog_space(LIST_T_SPACE);
		int width = rect.w - get_dialog_space(LIST_L_SPACE) - get_dialog_space(LIST_R_SPACE);
		for (int n=top_item; n<top_item + min(shown_items, num_items); n++, i++, y+=font_height)
			draw_item(i, s, x, y, width, n == selection && active);
	}

	const vector<T> &items;	// List of items

private:
	virtual void draw_item(vector<T>::const_iterator i, SDL_Surface *s, int x, int y, int width, bool selected) const = 0;
};

#endif
