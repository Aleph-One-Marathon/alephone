
/*

	Copyright (C) 2005 and beyond by the "Aleph One" developers.
 
	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	This license is contained in the file "COPYING",
	which is included with this source code; it is available online at
	http://www.gnu.org/licenses/gpl.html

*/

/*
 *  carbon_widgets.cpp - Widgets for carbon dialogs
 *
 */

#include "cseries.h"
#include "screen_drawing.h"
#include "carbon_widgets.h"

void SelectorWidget::set_labels (const std::vector<std::string>& labels)
{
	// Possibly should extend to operate on radio groups too?

	// Get the menu
	MenuRef Menu = GetControlPopupMenuHandle(m_ctrl);
	if (!Menu)
		return;
	
	// Get rid of old contents
	while(CountMenuItems(Menu)) DeleteMenuItem(Menu, 1);
	
	// Add in new contents
	for (std::vector<std::string>::const_iterator it = labels.begin (); it != labels.end (); ++it) {
		CFStringRef cfstring = CFStringCreateWithCString(NULL, (*it).c_str (), NULL);
		AppendMenuItemTextWithCFString(Menu, cfstring, 0, 0, NULL);
		CFRelease(cfstring);
	}
	
	SetControl32BitMaximum(m_ctrl, CountMenuItems(Menu));
}

void StaticTextWidget::set_text (const std::string& s)
{
	SetControlData(m_ctrl, kControlLabelPart, kControlStaticTextTextTag, s.length (), s.c_str ());
	Draw1Control (m_ctrl);
}

void EditTextOrNumberWidget::hide ()
{
	ControlRef focusCtrl;
	GetKeyboardFocus (GetControlOwner (m_ctrl), &focusCtrl);

	if (m_ctrl == focusCtrl)
		ClearKeyboardFocus (GetControlOwner (m_ctrl));
	
	if (m_label_widget)
		m_label_widget->hide ();
		
	NIBsControlWidget::hide ();
}

void EditTextOrNumberWidget::show ()
{	
	if (m_label_widget)
		m_label_widget->show ();
		
	NIBsControlWidget::show ();
}

void EditTextOrNumberWidget::set_label (const std::string& s)
{
	if (m_label_widget)
		m_label_widget->set_text (s);
}

void EditTextOrNumberWidget::set_text (const std::string& s)
{
	SetControlData(m_ctrl, kControlEditTextPart, kControlEditTextTextTag, s.length (), s.c_str ());
	Draw1Control (m_ctrl);
}

const string EditTextOrNumberWidget::get_text ()
{
	Size size = 0;
	GetControlDataSize(m_ctrl, kControlEditTextPart, kControlEditTextTextTag, &size);

	char* buffer = new char[size];
	GetControlData(m_ctrl, kControlEditTextPart, kControlEditTextTextTag, size, buffer, NULL);
	return std::string(buffer, size);
}

void EditNumberWidget::set_value (int value)
{
	NumToString (value, ptemporary);
	set_text (pstring_to_string (ptemporary));
}

int EditNumberWidget::get_value ()
{
	long result;

	copy_string_to_pstring(get_text (), ptemporary);
	StringToNum(ptemporary, &result);
	return result;
}

void FileChooserWidget::set_file (const FileSpecifier& file)
{
	m_file = file;
	
	char buffer[256];
	m_file.GetName (buffer);
	m_text->set_text (string (buffer));
}

void FileChooserWidget::choose_file ()
{
	if (m_file.ReadDialog (m_type, m_prompt.c_str ())) {
		char dummy[256];
		m_file.GetName (dummy);
		m_text->set_text (string (dummy));
		if (m_callback)
			m_callback ();
	}
}

const string ListWidgetValueForItem(const MetaserverPlayerInfo* element)
	{ return element == NULL ? string() : element->name(); }

const string ListWidgetValueForItem(const prospective_joiner_info* element)
	{ return pstring_to_string (element->name); }

const string ListWidgetValueForItem(const GameListMessage::GameListEntry* element)
	{ return element == NULL ? string() : element->name(); }

extern void _get_player_color(size_t color_index, RGBColor *color);

// helper for PlayersInGameWidget::pigDrawer
static void calculate_box_colors(
	short color_index,
	RGBColor *highlight_color,
	RGBColor *bar_color,
	RGBColor *shadow_color)
{
	_get_player_color(color_index, highlight_color);

	bar_color->red = (highlight_color->red * 7) / 10;
	bar_color->blue = (highlight_color->blue * 7) / 10;
	bar_color->green = (highlight_color->green * 7) / 10;
	
	shadow_color->red = (highlight_color->red * 2) / 10;
	shadow_color->blue = (highlight_color->blue * 2) / 10;
	shadow_color->green = (highlight_color->green * 2) / 10;
}

// helper for PlayersInGameWidget::pigDrawer
static void draw_player_box_with_team(
	Rect *rectangle, 
	short player_index)
{
	const int TEAM_BADGE_WIDTH =  16;
	const int NAME_BEVEL_SIZE = 4;

	player_info *player= (player_info *) NetGetPlayerData(player_index);
	RGBColor highlight_color, bar_color, shadow_color;
	Rect team_badge, color_badge, text_box;
	RGBColor old_color;
	short index;

	/* Save the color */
	GetForeColor(&old_color);
			
	/* Setup the rectangles.. */
	team_badge= color_badge= *rectangle;
	team_badge.right= team_badge.left+TEAM_BADGE_WIDTH;
	color_badge.left= team_badge.right;

	/* Determine the colors */
	calculate_box_colors(player->team, &highlight_color,
		&bar_color, &shadow_color);

	/* Erase the team badge area. */
	RGBForeColor(&bar_color);
	PaintRect(&team_badge);
	
	/* Draw the highlight for this one. */
	RGBForeColor(&highlight_color);
	for (index = 0; index < NAME_BEVEL_SIZE; index++)
	{
		MoveTo(team_badge.left+index, team_badge.bottom-index);
		LineTo(team_badge.left+index, team_badge.top+index);
		LineTo(team_badge.right, team_badge.top+index);
	}
	
	/* Draw the drop shadow.. */
	RGBForeColor(&shadow_color);
	for (index = 0; index < NAME_BEVEL_SIZE; index++)
	{
		MoveTo(team_badge.left+index, team_badge.bottom-index);
		LineTo(team_badge.right, team_badge.bottom-index);
	}

	/* Now draw the player color. */
	calculate_box_colors(player->color, &highlight_color,
		&bar_color, &shadow_color);

	/* Erase the team badge area. */
	RGBForeColor(&bar_color);
	PaintRect(&color_badge);
	
	/* Draw the highlight for this one. */
	RGBForeColor(&highlight_color);
	for (index = 0; index < NAME_BEVEL_SIZE; index++)
	{
		MoveTo(color_badge.left, color_badge.top+index);
		LineTo(color_badge.right-index, color_badge.top+index);
	}
	
	/* Draw the drop shadow.. */
	RGBForeColor(&shadow_color);
	for (index = 0; index < NAME_BEVEL_SIZE; index++)
	{
		MoveTo(color_badge.left, color_badge.bottom-index);
		LineTo(color_badge.right-index, color_badge.bottom-index);
		LineTo(color_badge.right-index, color_badge.top+index);
	}

	/* Finally, draw the name. */
	text_box= *rectangle;
	InsetRect(&text_box, NAME_BEVEL_SIZE, NAME_BEVEL_SIZE);
	CopyPascalStringToC(player->name, temporary);
	_draw_screen_text(temporary, (screen_rectangle *) &text_box, 
		_center_horizontal|_center_vertical, _net_stats_font, _white_color);		

	/* Restore the color */
	RGBForeColor(&old_color);
}

void PlayersInGameWidget::pigDrawer (ControlRef Ctrl, void* ignored)
{
	const int NAME_BOX_HEIGHT = 28;
	const int NAME_BOX_WIDTH = 114;

	const int BOX_SPACING = 8;

	// No need for the window context -- it's assumed
	Rect Bounds = {0,0,0,0};
	
	GetControlBounds(Ctrl, &Bounds);
	
	// Draw background and boundary
	ForeColor(whiteColor);
	PaintRect(&Bounds);
	ForeColor(blackColor);
	FrameRect(&Bounds);

	// Cribbed from update_player_list_item()
	FontInfo finfo;
	GetFontInfo(&finfo);
	short height = finfo.ascent + finfo.descent + finfo.leading;
	MoveTo(Bounds.left + 3, Bounds.top+height);
	short num_players = NetNumberOfPlayerIsValid() ? NetGetNumberOfPlayers() : 0;
	
	Rect name_rect;
	SetRect(&name_rect, Bounds.left, Bounds.top, Bounds.left+NAME_BOX_WIDTH, Bounds.top+NAME_BOX_HEIGHT);
	for (short i = 0; i < num_players; i++)
	{
		draw_player_box_with_team(&name_rect, i);
		if (!(i % 2))
		{
			OffsetRect(&name_rect, NAME_BOX_WIDTH+BOX_SPACING, 0);
		}
		else
		{
			OffsetRect(&name_rect, -(NAME_BOX_WIDTH+BOX_SPACING), NAME_BOX_HEIGHT + BOX_SPACING);
		}
	}
}

ColourPickerWidget::ColourPickerWidget (ControlRef ctrl)
	: NIBsControlWidget (ctrl)
	, m_control_watcher (ctrl)
{
	m_colourDrawer (ctrl, SwatchDrawer, &m_colour);
	m_colourHitter (ctrl);
	m_control_watcher.set_callback (boost::bind (&ColourPickerWidget::chooseColour, this));
}

void ColourPickerWidget::chooseColour ()
{
	copy_string_to_pstring ("You Must Choose:", ptemporary);
	PickControlColor (m_ctrl, &m_colour, ptemporary);
}
