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


/*
 *  Template for list selection widgets
 */

template <class T>
class w_list : public widget {
protected:
	static const int BORDER_SIZE = 2;
	static const int SCROLL_BAR_WIDTH = 16;

public:
	w_list(const vector<T> &it, int width, int lines, int sel) : widget(NORMAL_FONT), items(it), shown_items(lines), thumb_dragging(false)
	{
		font_height = font_line_height(font);
		rect.w = width;
		rect.h = font_height * shown_items + BORDER_SIZE * 2;
		new_items();
		set_selection(selection);
		center_item(selection);
	}
	~w_list() {}

	int layout(void)
	{
		rect.x = -rect.w / 2;
		return rect.h;
	}

	void draw(SDL_Surface *s) const
	{
		// Border
		draw_rectangle(s, &rect, get_dialog_color(s, BORDER_COLOR));

		// Scroll bar
		uint32 white = get_dialog_color(s, WHITE_COLOR);
		SDL_Rect r = {rect.x + rect.w - SCROLL_BAR_WIDTH, rect.y, SCROLL_BAR_WIDTH, rect.h};
		draw_rectangle(s, &r, white);
		r.y += thumb_y; r.h = thumb_height;
		SDL_FillRect(s, &r, get_dialog_color(s, thumb_dragging ? THUMB_ACTIVE_COLOR : THUMB_COLOR));
		draw_rectangle(s, &r, white);

		// Items
		vector<T>::const_iterator i = items.begin() + top_item;
		int y = rect.y + BORDER_SIZE;
		for (int n=top_item; n<top_item + min(shown_items, num_items); n++, i++, y+=font_height)
			draw_item(i, s, rect.x + BORDER_SIZE, y, rect.w - BORDER_SIZE * 2, n == selection && active);
	}

	void mouse_move(int x, int y)
	{
		if (thumb_dragging) {
			int delta_y = y - thumb_drag_y;
			set_top_item(delta_y * num_items / rect.h);
		} else {
			if (x < BORDER_SIZE || x >= rect.w - SCROLL_BAR_WIDTH)
				return;
			if (y < BORDER_SIZE || y >= rect.h - BORDER_SIZE)
				return;
			set_selection((y - BORDER_SIZE) / font_height + top_item);
		}
	}

	void click(int x, int y)
	{
		if (x >= rect.w - SCROLL_BAR_WIDTH) {
			thumb_dragging = dirty = true;
			thumb_drag_y = y - thumb_y;
		} else
			item_selected();
	}

	void event(SDL_Event &e)
	{
		if (e.type == SDL_KEYDOWN) {
			switch (e.key.keysym.sym) {
				case SDLK_UP:
					set_selection(selection - 1);
					e.type = SDL_NOEVENT;	// Prevent selection of previous widget
					break;
				case SDLK_DOWN:
					set_selection(selection + 1);
					e.type = SDL_NOEVENT;	// Prevent selection of next widget
					break;
				case SDLK_PAGEUP:
					set_selection(selection - shown_items);
					break;
				case SDLK_PAGEDOWN:
					set_selection(selection + shown_items);
					break;
				case SDLK_HOME:
					set_selection(0);
					break;
				case SDLK_END:
					set_selection(num_items - 1);
					break;
				default:
					break;
			}
		} else if (e.type == SDL_MOUSEBUTTONUP) {
			if (thumb_dragging) {
				thumb_dragging = false;
				dirty = true;
			}
		}
	}

	int get_selection(void) {return selection;}

	virtual void item_selected(void) = 0;

protected:
	void set_selection(int s)
	{
		if (s < 0)
			s = 0;
		else if (s >= num_items)
			s = num_items - 1;
		if (s != selection)
			dirty = true;
		selection = s;
		if (s < top_item)
			set_top_item(s);
		else if (s >= top_item + shown_items)
			set_top_item(s - shown_items + 1);
	}

	void new_items(void)
	{
		num_items = items.size();
		top_item = selection = 0;
		dirty = true;
		if (num_items <= shown_items)
			thumb_height = rect.h;
		else
			thumb_height = shown_items * rect.h / num_items;
	}

	void center_item(int i)
	{
		set_top_item(selection - shown_items / 2);
	}

	int selection;			// Currently selected item
	int font_height;		// Height of font

private:
	void set_top_item(int i)
	{
		if (i > num_items - shown_items)
			i = num_items - shown_items;
		if (i < 0)
			i = 0;
		if (i != top_item)
			dirty = true;
		top_item = i;
		thumb_y = top_item * rect.h / num_items;
	}

	virtual void draw_item(vector<T>::const_iterator i, SDL_Surface *s, int x, int y, int width, bool selected) const = 0;

	const vector<T> &items;	// List items
	int num_items;			// Total number of items
	int top_item;			// Number of first visible item
	int shown_items;		// Number of shown items

	bool thumb_dragging;	// Flag: currently dragging scroll bar thumb
	int thumb_height;		// Height of scroll bar
	int thumb_y;			// Y position of scroll bar
	int thumb_drag_y;		// Y start position when dragging
};

#endif
