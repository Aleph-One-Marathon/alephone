/*
 *  computer_interface_sdl.cpp - Terminal handling, SDL specific stuff
 *
 *  Written in 2000 by Christian Bauer
 */


// Global variables
static sdl_font_info *terminal_font = NULL;
static uint32 current_pixel;			// Current color pixel value
static uint16 current_style = normal;	// Current style flags

// From images_sdl.cpp
extern SDL_Surface *picture_to_surface(void *picture, uint32 size);


// Terminal key definitions
static struct terminal_key terminal_keys[]= {
	{SDLK_UP, 0, 0, _terminal_page_up},				// arrow up
	{SDLK_DOWN, 0, 0, _terminal_page_down},			// arrow down
	{SDLK_PAGEUP, 0, 0, _terminal_page_up},			// page up
	{SDLK_PAGEDOWN, 0, 0, _terminal_page_down},		// page down
	{SDLK_TAB, 0, 0, _terminal_next_state},			// tab
	{SDLK_KP_ENTER, 0, 0, _terminal_next_state},	// enter
	{SDLK_RETURN, 0, 0, _terminal_next_state},		// return
	{SDLK_SPACE, 0, 0, _terminal_next_state},		// space
	{SDLK_ESCAPE, 0, 0, _any_abort_key_mask}		// escape
};


// Emulation of MacOS functions
static void SetRect(Rect *r, int left, int top, int right, int bottom)
{
	r->top = top;
	r->left = left;
	r->bottom = bottom;
	r->right = right;
}

static void InsetRect(Rect *r, int dx, int dy)
{
	r->top += dy;
	r->left += dx;
	r->bottom -= dy;
	r->right -= dx;
}

static void OffsetRect(Rect *r, int dx, int dy)
{
	r->top += dy;
	r->left += dx;
	r->bottom += dy;
	r->right += dx;
}


static void	set_text_face(struct text_face_data *text_face)
{
	current_style = normal;

	// Set style
	if (text_face->face & _bold_text)
		current_style |= bold;
	if (text_face->face & _italic_text)
		current_style |= italic;
	if (text_face->face & _underline_text)
		current_style |= underline;

	// Set color
	SDL_Color color;
	_get_interface_color(text_face->color + _computer_interface_text_color, &color);
	current_pixel = SDL_MapRGB(world_pixels->format, color.r, color.g, color.b);
}
