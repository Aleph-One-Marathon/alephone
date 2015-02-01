/*
 *  QuickSave.cpp - a manager for auto-named saved games
 
	Copyright (C) 2014 and beyond by Jeremiah Morris
	and the "Aleph One" developers.
 
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

#include "cseries.h"
#include "QuickSave.h"

#include <fstream>
#include <sstream>
#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/predicate.hpp>

#ifdef HAVE_SDL_IMAGE_H
#include <SDL_image.h>
#endif
#ifdef HAVE_PNG
#include "IMG_savepng.h"
#endif

#include "FileHandler.h"
#include "world.h"
#include "map.h"
#include "overhead_map.h"
#include "screen_drawing.h"
#include "interface.h"
#include "preferences.h"
#include "shell.h"
#include "player.h"
#include "game_wad.h"
#include "game_errors.h"
#include "sdl_dialogs.h"
#include "sdl_widgets.h"
#include "Logging.h"
#include "XML_ElementParser.h"
#include "XML_Configure.h"
#include "images.h"
#include "sdl_resize.h"

namespace algo = boost::algorithm;

const int RENDER_WIDTH = 1280;
const int RENDER_HEIGHT = 720;
const int RENDER_SCALE = OVERHEAD_MAP_MAXIMUM_SCALE;

const int PREVIEW_WIDTH = 128;
const int PREVIEW_HEIGHT = 72;

bool write_save_metadata(QuickSave& save);


class QuickSaveLoader : public XML_Configure {
public:
    QuickSaveLoader() { }
    ~QuickSaveLoader() { }
    
    bool ParseDirectory(FileSpecifier& dir);
    bool ParseQuickSave(FileSpecifier& file);
    
protected:
    virtual bool GetData();
    virtual void ReportReadError();
    virtual void ReportParseError(const char *ErrorString, int LineNumber);
    virtual void ReportInterpretError(const char* ErrorString);
    virtual bool RequestAbort();
    
private:
    std::string m_name;
    std::vector<char> m_data;
};

class QuickSaveImageCache {
public:
    typedef std::pair<std::string, SDL_Surface*> cache_pair_t;
    typedef std::list<cache_pair_t>::iterator cache_iter_t;
    
    static QuickSaveImageCache* instance();
    
    SDL_Surface* get(std::string image_name);
    void clear();

private:
    QuickSaveImageCache() {};
    static QuickSaveImageCache* m_instance;
    static const int k_max_items = 100;
    
    std::list<cache_pair_t> m_used;
    std::map<std::string, cache_iter_t> m_images;
};

QuickSaveImageCache* QuickSaveImageCache::m_instance = 0;
QuickSaveImageCache* QuickSaveImageCache::instance() {
    if (!m_instance) {
        m_instance = new QuickSaveImageCache;
    }
    
    return m_instance;
}

SDL_Surface* QuickSaveImageCache::get(std::string image_name) {
    std::map<std::string, cache_iter_t>::iterator it = m_images.find(image_name);
    if (it != m_images.end()) {
        // found it: move to front of list
        m_used.splice(m_used.begin(), m_used, it->second);
        return it->second->second;
    }
    
    // didn't find: load image
    DirectorySpecifier path;
    path.SetToQuickSavesDir();
    FileSpecifier f = path + image_name;
	OpenedFile of;
	if (!f.Open(of))
		return NULL;
	
#ifdef HAVE_SDL_IMAGE
	SDL_Surface *img = IMG_Load_RW(of.GetRWops(), 0);
#else
	SDL_Surface *img = SDL_LoadBMP_RW(of.GetRWops(), 0);
#endif
	if (img) {
		if (img->w != PREVIEW_WIDTH || img->h != PREVIEW_HEIGHT) {
			img = SDL_Resize(img, PREVIEW_WIDTH, PREVIEW_HEIGHT, true, 1);
		}
		
        m_used.push_front(cache_pair_t(image_name, img));
        m_images[image_name] = m_used.begin();
        
        // enforce maximum cache size
        if (m_used.size() > k_max_items) {
            cache_iter_t lru = m_used.end();
            --lru;
            m_images.erase(lru->first);
            SDL_FreeSurface(lru->second);
            m_used.pop_back();
        }
    }
    return img;
}

void QuickSaveImageCache::clear() {
    m_images.clear();
    for (cache_iter_t it = m_used.begin(); it != m_used.end(); ++it) {
        SDL_FreeSurface(it->second);
    }
    m_used.clear();
}


class w_saves : public w_list_base {
public:
    w_saves(std::vector<QuickSave>& saves, int width, int numRows) : w_list_base(width, numRows, 0), m_saves(saves)
    {
        saved_min_height = item_height() * static_cast<uint16>(shown_items) + get_theme_space(LIST_WIDGET, T_SPACE) + get_theme_space(LIST_WIDGET, B_SPACE);
        trough_rect.h = saved_min_height - get_theme_space(LIST_WIDGET, TROUGH_T_SPACE) - get_theme_space(LIST_WIDGET, TROUGH_B_SPACE);
        num_items = m_saves.size();
        new_items();
    }
    
    void mouse_move(int x, int y);
    void click(int x, int y);
    uint16 item_height() const { return PREVIEW_HEIGHT + 6; }
    QuickSave selected_save() { return m_saves[get_selection()]; }
    void remove_selected();
    void update_selected(QuickSave& save) { m_saves[get_selection()] = save; dirty = true; }
    bool has_selection() { return m_saves.size() > 0; }
    
protected:
    void draw_items(SDL_Surface* s) const;
    void item_selected();
    
private:
    std::vector<QuickSave>& m_saves;
    void draw_item(QuickSaves::iterator i, SDL_Surface* s, int16 x, int16 y, uint16 width, bool selected) const;
};

void w_saves::remove_selected()
{
    m_saves.erase(m_saves.begin()+get_selection());
    num_items = m_saves.size();
    new_items();
}

void w_saves::mouse_move(int x, int y)
{
    if (thumb_dragging) {
        w_list_base::mouse_move(x, y);
    }
}

void w_saves::click(int x, int y)
{
    if (x == 0 && y == 0 && active) {
        // almost certainly a simulated click
        if (num_items > 0)
            item_selected();
    }
    else if (x >= trough_rect.x && x < trough_rect.x + trough_rect.w
             && y >= thumb_y && y <= thumb_y + thumb_height) {
        thumb_dragging = dirty = true;
        thumb_drag_y = y - thumb_y;
    } else {
        if (x < get_theme_space(LIST_WIDGET, L_SPACE) || x >= rect.w - get_theme_space(LIST_WIDGET, R_SPACE)
            || y < get_theme_space(LIST_WIDGET, T_SPACE) || y >= rect.h - get_theme_space(LIST_WIDGET, B_SPACE))
            return;
        
        if ((y - get_theme_space(LIST_WIDGET, T_SPACE)) / item_height() + top_item < min(num_items, top_item + shown_items))
        {
            size_t old_sel = selection;
            set_selection((y - get_theme_space(LIST_WIDGET, T_SPACE)) / item_height() + top_item);
            if (selection == old_sel && num_items > 0 && is_item_selectable(selection))
                item_selected();
        }
    }
}

void w_saves::draw_items(SDL_Surface* s) const
{
    QuickSaves::iterator i = m_saves.begin();
    int16 x = rect.x + get_theme_space(LIST_WIDGET, L_SPACE);
    int16 y = rect.y + get_theme_space(LIST_WIDGET, T_SPACE);
    uint16 width = rect.w - get_theme_space(LIST_WIDGET, L_SPACE) - get_theme_space(LIST_WIDGET, R_SPACE);
    
    for (size_t n = 0; n < top_item; ++n)
    {
        ++i;
    }
    
    for (size_t n = top_item; n < top_item + MIN(shown_items, num_items); ++n, ++i, y = y + item_height())
        draw_item(i, s, x, y, width, n == selection);
}

void w_saves::item_selected()
{    
    get_owning_dialog()->quit(0);
}

void w_saves::draw_item(QuickSaves::iterator it, SDL_Surface* s, int16 x, int16 y, uint16 width, bool selected) const
{
    SDL_Surface *image = QuickSaveImageCache::instance()->get(it->preview_name);
    SDL_Rect r = {x + 3, y + 3, PREVIEW_WIDTH, PREVIEW_HEIGHT};
    SDL_BlitSurface(image, NULL, s, &r);
    x += PREVIEW_WIDTH + 12;
    width -= PREVIEW_WIDTH + 12;
    
    uint32 color;
    if (selected)
    {
        color = get_theme_color(ITEM_WIDGET, ACTIVE_STATE);
    }
    else
    {
        color = get_theme_color(ITEM_WIDGET, DEFAULT_STATE);
    }
    set_drawing_clip_rectangle(0, x, static_cast<short>(s->h), x + width);
    
    y += font->get_ascent();
    if (it->name.length())
    {
        draw_text(s, utf8_to_mac_roman(it->name).c_str(), x, y, color, font, style);
        y += font->get_ascent() + 1;
    }
    draw_text(s, it->formatted_time.c_str(), x, y, color, font, style);
    
    y += font->get_ascent() + 1;
    draw_text(s, utf8_to_mac_roman(it->level_name).c_str(), x, y, color, font, style);
    
    y += font->get_ascent() + 1;
    std::string game_time = it->formatted_ticks;
    if (it->players > 1)
    {
        game_time += " (Cooperative Play)";
    }
    draw_text(s, game_time.c_str(), x, y, color, font, style);
    
    set_drawing_clip_rectangle(SHRT_MIN, SHRT_MIN, SHRT_MAX, SHRT_MAX);
}

// Allow rename dialog to be closed by hitting Return in the text field
class w_save_name : public w_text_entry {
public:
    w_save_name(dialog *d, const char *initial_name = NULL) : w_text_entry(256, initial_name), parent(d) {}
    ~w_save_name() {}
    
    void event(SDL_Event & e)
    {
        // Return = close dialog
        if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_RETURN)
            parent->quit(0);
        w_text_entry::event(e);
    }
    
private:
    dialog *parent;
};


const int LOAD_DIALOG_OTHER = 4;
static void dialog_exit_other(void *arg)
{
    dialog *d = static_cast<dialog *>(arg);
    d->quit(LOAD_DIALOG_OTHER);
}

const int iDIALOG_SAVES_W = 42;
const int iDIALOG_RENAME_W = 43;
const int iDIALOG_DELETE_W = 44;
const int iDIALOG_EXPORT_W = 45;
const int iDIALOG_ACCEPT_W = 46;

static void dialog_rename(void *arg)
{
    dialog *d = static_cast<dialog *>(arg);
    w_saves *saves_w = static_cast<w_saves *>(d->get_widget_by_id(iDIALOG_SAVES_W));
    QuickSave sel = saves_w->selected_save();

    dialog rd;
    vertical_placer *placer = new vertical_placer;
    
    horizontal_placer* name_placer = new horizontal_placer;
    w_text_entry *rename_w = new w_save_name(&rd, utf8_to_mac_roman(sel.name).c_str());
    rename_w->enable_mac_roman_input();
    name_placer->dual_add(rename_w->label("Name: "), rd);
    name_placer->dual_add(rename_w, rd);
    placer->add(name_placer, true);
    
    placer->add(new w_spacer(), true);

    horizontal_placer* button_placer = new horizontal_placer;
    w_button* accept_w = new w_button("RENAME", dialog_ok, &rd);
    button_placer->dual_add(accept_w, rd);
    w_button* cancel_w = new w_button("CANCEL", dialog_cancel, &rd);
    button_placer->dual_add(cancel_w, rd);
    placer->add(button_placer, true);
    
    rd.set_widget_placer(placer);
    rd.activate_widget(rename_w);
    if (rd.run() == 0) {
        sel.name = mac_roman_to_utf8(rename_w->get_text());
        write_save_metadata(sel);
        saves_w->update_selected(sel);
    }
}
static void dialog_delete(void *arg)
{
    dialog *d = static_cast<dialog *>(arg);
    w_saves *saves_w = static_cast<w_saves *>(d->get_widget_by_id(iDIALOG_SAVES_W));
    QuickSave sel = saves_w->selected_save();
    if (delete_quick_save(sel)) {
        saves_w->remove_selected();
        if (!saves_w->has_selection()) {
			w_tiny_button* rename_w = static_cast<w_tiny_button *>(d->get_widget_by_id(iDIALOG_RENAME_W));
            rename_w->set_enabled(false);
            w_tiny_button* delete_w = static_cast<w_tiny_button *>(d->get_widget_by_id(iDIALOG_DELETE_W));
            delete_w->set_enabled(false);
            w_tiny_button* export_w = static_cast<w_tiny_button *>(d->get_widget_by_id(iDIALOG_EXPORT_W));
            export_w->set_enabled(false);
            w_button* accept_w = static_cast<w_button *>(d->get_widget_by_id(iDIALOG_ACCEPT_W));
            accept_w->set_enabled(false);
        }
    }
}
static void dialog_export(void *arg)
{
    dialog *d = static_cast<dialog *>(arg);
    w_saves *saves_w = static_cast<w_saves *>(d->get_widget_by_id(iDIALOG_SAVES_W));
    QuickSave sel = saves_w->selected_save();
    std::string name = sel.name;
    if (!name.length())
        name = sel.level_name;
    
    FileSpecifier srcFile;
    srcFile.SetToQuickSavesDir();
    srcFile += sel.save_file_name;
    
    FileSpecifier dstFile;
    dstFile.SetToSavedGamesDir();
    dstFile += "unused.sgaA";
    char prompt[256];
    if (dstFile.WriteDialog(_typecode_savegame, getcstr(prompt, strPROMPTS, _save_replay_prompt), utf8_to_mac_roman(name).c_str())) {
        dstFile.CopyContents(srcFile);
        int error = dstFile.GetError();
        if (error)
            alert_user(infoError, strERRORS, fileError, error);
    }
}


static FileSpecifier last_saved_game;
static size_t last_saved_networked = UNONE;
size_t saved_game_was_networked(FileSpecifier& saved_game)
{
    return (saved_game == last_saved_game) ? last_saved_networked : UNONE;
}

bool load_quick_save_dialog(FileSpecifier& saved_game)
{
    QuickSaves::instance()->enumerate();

    dialog d;
    vertical_placer *placer = new vertical_placer;
    w_title *w_header = new w_title("CONTINUE SAVED GAME");
    placer->dual_add(w_header, d);
    placer->add(new w_spacer, true);
    
    horizontal_placer *mini_button_placer = new horizontal_placer;
    w_tiny_button *rename_w = new w_tiny_button("RENAME", dialog_rename, &d);
    rename_w->set_identifier(iDIALOG_RENAME_W);
    mini_button_placer->dual_add(rename_w, d);
    w_tiny_button *export_w = new w_tiny_button("EXPORT", dialog_export, &d);
    export_w->set_identifier(iDIALOG_EXPORT_W);
    mini_button_placer->dual_add(export_w, d);
    w_tiny_button *delete_w = new w_tiny_button("DELETE", dialog_delete, &d);
    delete_w->set_identifier(iDIALOG_DELETE_W);
    mini_button_placer->dual_add(delete_w, d);
    
    placer->add(mini_button_placer, true);
    placer->add(new w_spacer, true);

    std::vector<QuickSave> saves(QuickSaves::instance()->begin(), QuickSaves::instance()->end());
    w_saves* saves_w = new w_saves(saves, 400, 4);
    saves_w->set_identifier(iDIALOG_SAVES_W);
    placer->dual_add(saves_w, d);
    placer->add(new w_spacer, true);

    horizontal_placer* button_placer = new horizontal_placer;
    w_button* other_w = new w_button("LOAD OTHER", dialog_exit_other, &d);
    button_placer->dual_add(other_w, d);
    w_button* accept_w = new w_button("LOAD", dialog_ok, &d);
    accept_w->set_identifier(iDIALOG_ACCEPT_W);
    button_placer->dual_add(accept_w, d);
    w_button* cancel_w = new w_button("CANCEL", dialog_cancel, &d);
    button_placer->dual_add(cancel_w, d);
    
    placer->add(button_placer, true);
    
    d.set_widget_placer(placer);
    d.activate_widget(saves_w);
    
    if (!saves_w->has_selection())
    {
        rename_w->set_enabled(false);
        delete_w->set_enabled(false);
        export_w->set_enabled(false);
        accept_w->set_enabled(false);
    }
    
    bool ret = false;
    QuickSave sel;
    switch (d.run()) {
        case 0:
            sel = saves_w->selected_save();
            saved_game.SetToQuickSavesDir();
            saved_game += sel.save_file_name;
            last_saved_game = saved_game;
            last_saved_networked = (sel.players > 1) ? 1 : 0;
            ret = true;
            break;
        case LOAD_DIALOG_OTHER:
            last_saved_networked = UNONE;
            ret = saved_game.ReadDialog(_typecode_savegame);
            break;
        default:
            break;
    }
    
    QuickSaves::instance()->clear();
    QuickSaveImageCache::instance()->clear();
    return ret;
}

extern SDL_Surface *draw_surface;
extern bool OGL_MapActive;

static bool save_map_preview(FileSpecifier& file)
{
    SDL_Rect r = {0, 0, RENDER_WIDTH, RENDER_HEIGHT};
    SDL_Surface *surface = SDL_CreateRGBSurface(SDL_SWSURFACE, r.w, r.h, 32, 0xff0000, 0x00ff00, 0x0000ff, 0);
    if (!surface)
        return false;
    
    SDL_FillRect(surface, &r, SDL_MapRGB(surface->format, 0, 0, 0));
    
    struct overhead_map_data overhead_data;
    overhead_data.half_width = r.w >> 1;
    overhead_data.half_height = r.h >> 1;
    overhead_data.width = r.w;
    overhead_data.height = r.h;
    overhead_data.top = overhead_data.left = 0;
    overhead_data.scale = RENDER_SCALE;
    overhead_data.mode = _rendering_saved_game_preview;
    overhead_data.origin.x = local_player->location.x;
    overhead_data.origin.y = local_player->location.y;
    
    bool old_OGL_MapActive = OGL_MapActive;
    _set_port_to_custom(surface);
    OGL_MapActive = false;
    _render_overhead_map(&overhead_data);
    OGL_MapActive = old_OGL_MapActive;
    _restore_port();

#if defined(HAVE_PNG) && defined(HAVE_SDL_IMAGE_H)
	int ret = IMG_SavePNG(file.GetPath(), surface, IMG_COMPRESS_DEFAULT, NULL, 0);
#else
	int ret = SDL_SaveBMP(surface, file.GetPath());
#endif
    SDL_FreeSurface(surface);
    return (ret == 0);
}

bool write_save_metadata(QuickSave& save)
{
    std::string lev_name = mac_roman_to_utf8(static_world->level_name);
    boost::replace_all(lev_name, "&",  "&amp;");
    boost::replace_all(lev_name, "\"", "&quot;");
    boost::replace_all(lev_name, "\'", "&apos;");
    boost::replace_all(lev_name, "<",  "&lt;");
    boost::replace_all(lev_name, ">",  "&gt;");
    
    std::ofstream xout;
    xout.open(save.metadata_file.GetPath());
    xout << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << std::endl
    << "<save_info"
    << " name=\"" << save.name << "\""
    << " level_name=\"" << save.level_name << "\""
    << " ticks=\"" << save.ticks << "\""
    << " ticks_formatted=\"" << save.formatted_ticks << "\""
    << " time=\"" << save.save_time << "\""
    << " time_formatted=\"" << save.formatted_time << "\""
    << " save_game=\"" << save.save_file_name << "\""
    << " preview=\"" << save.preview_name << "\""
    << " players=\"" << save.players << "\""
    << "/>" << std::endl;
    xout.close();
    
    return true;
}

bool create_quick_save(void)
{
    QuickSave save;

    time(&(save.save_time));
    char fmt_time[256];
    tm *time_info = localtime(&(save.save_time));
    strftime(fmt_time, 256, "%x %R", time_info);
    save.formatted_time = fmt_time;

    save.level_name = mac_roman_to_utf8(static_world->level_name);
    save.players = dynamic_world->player_count;
    save.ticks = dynamic_world->tick_count;
    char fmt_ticks[256];
    if (save.ticks < 60*TICKS_PER_MINUTE)
        sprintf(fmt_ticks, "%d:%02d",
                save.ticks/TICKS_PER_MINUTE,
                (save.ticks/TICKS_PER_SECOND) % 60);
    else
        sprintf(fmt_ticks, "%d:%02d:%02d",
                save.ticks/(60*TICKS_PER_MINUTE),
                (save.ticks/TICKS_PER_MINUTE) % 60,
                (save.ticks/TICKS_PER_SECOND) % 60);
    save.formatted_ticks = fmt_ticks;
    
    DirectorySpecifier metadata_dir;
    metadata_dir.SetToQuickSavesDir();
    std::ostringstream oss;
    oss << save.save_time;
    std::string base = oss.str();

    save.metadata_file.FromDirectory(metadata_dir);
    save.metadata_file.AddPart(base + ".xml");
    save.save_file_name = base + ".sgaA";
#if defined(HAVE_PNG) && defined(HAVE_SDL_IMAGE_H)
	save.preview_name = base + ".png";
#else
    save.preview_name = base + ".bmp";
#endif
	
    FileSpecifier save_game, save_image;
    save_game.FromDirectory(metadata_dir);
    save_game.AddPart(save.save_file_name);
    save_image.FromDirectory(metadata_dir);
    save_image.AddPart(save.preview_name);
    
    bool success = save_game_file(save_game);
    if (success) {
        success = save_map_preview(save_image);
        if (!success)
            save_game.Delete();
    }
    if (success) {
        success = write_save_metadata(save);
        if (!success) {
            save_game.Delete();
            save_image.Delete();
        }
    }
    
    if (success)
        QuickSaves::instance()->delete_surplus_saves(environment_preferences->maximum_quick_saves);
    return success;
}

bool delete_quick_save(QuickSave& save)
{
    DirectorySpecifier metadata_dir;
    save.metadata_file.ToDirectory(metadata_dir);
    
    if (save.preview_name.length())
    {
        FileSpecifier save_image;
        save_image.FromDirectory(metadata_dir);
        save_image.AddPart(save.preview_name);
        save_image.Delete();
    }
    if (save.save_file_name.length())
    {
        FileSpecifier save_game;
        save_game.FromDirectory(metadata_dir);
        save_game.AddPart(save.save_file_name);
        save_game.Delete();
    }
    return save.metadata_file.Delete();
}

static QuickSave Data;
static FileSpecifier DataFile;

class XML_QuickSaveParser : public XML_ElementParser
{
public:
    bool Start();
    bool HandleAttribute(const char* Tag, const char* Value);
    bool AttributesDone();
    bool End();
    
    XML_QuickSaveParser() : XML_ElementParser("save_info") {}
};

bool XML_QuickSaveParser::Start() {
    Data = QuickSave();
    Data.metadata_file = DataFile;
    return true;
}

bool XML_QuickSaveParser::HandleAttribute(const char* Tag, const char* Value)
{
    if (StringsEqual(Tag, "name")) {
        Data.name = Value;
        return true;
    } else if (StringsEqual(Tag, "level_name")) {
        Data.level_name = Value;
        return true;
    } else if (StringsEqual(Tag, "ticks")) {
        ReadInt32Value(Value, Data.ticks);
        return true;
    } else if (StringsEqual(Tag, "ticks_formatted")) {
        Data.formatted_ticks = Value;
        return true;
    } else if (StringsEqual(Tag, "time")) {
        std::istringstream stream(Value);
        stream >> Data.save_time;
        return true;
    } else if (StringsEqual(Tag, "time_formatted")) {
        Data.formatted_time = Value;
        return true;
    } else if (StringsEqual(Tag, "save_game")) {
        Data.save_file_name = Value;
        return true;
    } else if (StringsEqual(Tag, "preview")) {
        Data.preview_name = Value;
        return true;
    } else if (StringsEqual(Tag, "players")) {
        ReadBoundedInt16Value(Value, Data.players, 1, 8);
        return true;
    }

    UnrecognizedTag();
    return false;
}

bool XML_QuickSaveParser::AttributesDone()
{
    if (Data.save_file_name == "") {
        AttribsMissing();
        return false;
    }
    
    return true;
}

bool XML_QuickSaveParser::End() {
    QuickSaves::instance()->add(Data);
    return true;
}

XML_ElementParser QuickSaveRootParser("");
XML_QuickSaveParser QuickSaveParser;

bool QuickSaveLoader::GetData()
{
    if (m_data.size() == 0) {
        return false;
    }
    
    Buffer = &m_data[0];
    BufLen = m_data.size();
    LastOne = true;
    
    return true;
}

void QuickSaveLoader::ReportReadError()
{
    logError1("Error reading %s quick save", m_name.c_str());
}

void QuickSaveLoader::ReportParseError(const char* ErrorString, int LineNumber)
{
    logError3("XML parsing error: %s at line %d in %s", ErrorString, LineNumber, m_name.c_str());
}

const int MaxErrorsToShow = 7;

void QuickSaveLoader::ReportInterpretError(const char* ErrorString)
{
    if (GetNumInterpretErrors() < MaxErrorsToShow) {
        logError(ErrorString);
    }
}

bool QuickSaveLoader::RequestAbort()
{
    return (GetNumInterpretErrors() >= MaxErrorsToShow);
}

bool QuickSaveLoader::ParseQuickSave(FileSpecifier& file_name)
{
    OpenedFile file;
    if (file_name.Open(file))
    {
        
        int32 data_size;
        file.GetLength(data_size);
        m_data.resize(data_size);
        
        if (file.Read(data_size, &m_data[0]))
        {
            DataFile = file_name;
            m_name = file_name.GetPath();
            if (!DoParse())
            {
                logError1("There were parsing errors in %s\n", m_name.c_str());
            }
        }
        
        m_data.clear();
        return true;
    }
    return false;
}

bool QuickSaveLoader::ParseDirectory(FileSpecifier& dir)
{
    std::vector<dir_entry> de;
    if (!dir.ReadDirectory(de))
        return false;
    
    for (std::vector<dir_entry>::const_iterator it = de.begin(); it != de.end(); ++it) {
        FileSpecifier file = dir + it->name;
        if (algo::ends_with(it->name, ".xml"))
        {
            ParseQuickSave(file);
        }
    }
    
    return true;
}


QuickSaves* QuickSaves::m_instance = 0;
QuickSaves* QuickSaves::instance() {
    if (!m_instance) {
        m_instance = new QuickSaves;
    }
    
    return m_instance;
}

void QuickSaves::enumerate() {
    clear();
    
    QuickSaveRootParser.AddChild(&QuickSaveParser);
    
    logContext("parsing quick saves");
    QuickSaveLoader loader;
    loader.CurrentElement = &QuickSaveRootParser;
    
    DirectorySpecifier path;
    path.SetToQuickSavesDir();
    loader.ParseDirectory(path);
    clear_game_error();
    std::sort(m_saves.begin(), m_saves.end());
    std::reverse(m_saves.begin(), m_saves.end());
}

void QuickSaves::clear() {
    m_saves.clear();
}

bool most_recent_dir_entry(const dir_entry& a, const dir_entry& b)
{
    return a.date > b.date;
}

void QuickSaves::delete_surplus_saves(size_t max_saves)
{
    if (max_saves < 1)
        return;     // unlimited saves, no need to prune
    clear();
    
    // Check the directory to count the saves. If there
    // are fewer than the max, no need to go further.
    vector<dir_entry> entries;
    DirectorySpecifier path;
    path.SetToQuickSavesDir();
    if (path.ReadDirectory(entries)) {
        if (entries.size() <= max_saves)
            return;
    }
    
    // We might have too many unnamed saves; load and
    // count them, deleting any extras.
    enumerate();
    size_t unnamed_saves = 0;
    for (std::vector<QuickSave>::iterator it = begin(); it != end(); ++it) {
        if (it->name.length())
            continue;
        if (++unnamed_saves > max_saves)
            delete_quick_save(*it);
    }
    clear();
}

