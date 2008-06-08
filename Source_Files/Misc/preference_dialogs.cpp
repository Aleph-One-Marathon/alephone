/*

	Copyright (C) 2006 and beyond by Bungie Studios, Inc.
	and the "Aleph One" developers.
 
	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	This license is contained in the file "COPYING",
	which is included with this source code; it is available online at
	http://www.gnu.org/licenses/gpl.html
	
*/

#include "preference_dialogs.h"
#include "preferences.h"
#include "binders.h"
#include "OGL_Setup.h"

class TexQualityPref : public Bindable<int>
{
public:
	TexQualityPref (int16& pref, int16 normal) : m_pref (pref), m_normal (normal) {}
	
	virtual int bind_export ()
	{
		int result = 0;
		int temp = m_pref;
		while (temp >= m_normal) {
			temp = temp >> 1;
			++result;
		}
		return result;
	}
	
	virtual void bind_import (int value)
	{
		m_pref = (value == 0) ? 0 : m_normal << (value - 1);
	}
	
protected:
	int16& m_pref;
	int16 m_normal;
};

class ColourPref : public Bindable<RGBColor>
{
public:
	ColourPref (RGBColor& pref) : m_pref (pref) {}
	
	virtual RGBColor bind_export () { return m_pref; }
	virtual void bind_import (RGBColor value) { m_pref = value; }
	
protected:
	RGBColor& m_pref;
};

class FilterPref : public Bindable<int>
{
public:
	FilterPref (int16& pref) : m_pref(pref) { }

	int bind_export() {
		return (m_pref - 1) / 2;
	}

	void bind_import(int value) {
		m_pref = value * 2 + 1;
	}

protected:
	int16& m_pref;
};

class TimesTwoPref : public Bindable<int>
{
public:
	TimesTwoPref (int16& pref) : m_pref (pref) {}
	
	virtual int bind_export ()
	{
		return (m_pref / 2);
	}
	
	virtual void bind_import (int value)
	{
		m_pref = value * 2;
	}

protected:
	int16& m_pref;
};

class AnisotropyPref : public Bindable<int>
{
public:
	AnisotropyPref (float& pref) : m_pref (pref) {}
	
	virtual int bind_export ()
	{
		int result = 0;
		int temp = static_cast<int> (m_pref);
		while (temp >= 1) {
			temp = temp >> 1;
			++result;
		}
		return result;
	}
	
	virtual void bind_import (int value)
	{
		m_pref = (value == 0) ? 0.0 : 1 << (value - 1);
	}

protected:
	float& m_pref;
};

OpenGLDialog::OpenGLDialog() {  }

OpenGLDialog::~OpenGLDialog()
{
	delete m_cancelWidget;
	delete m_okWidget;
	delete m_zBufferWidget;
	delete m_fogWidget;
	delete m_staticEffectWidget;
	delete m_colourEffectsWidget;
	delete m_transparentLiquidsWidget;
	delete m_3DmodelsWidget;
	delete m_colourTheVoidWidget;
	delete m_voidColourWidget;
	delete m_fsaaWidget;
	delete m_anisotropicWidget;
	delete m_geForceFixWidget;
	delete m_wallsFilterWidget;
	delete m_spritesFilterWidget;

	for (int i=0; i<OGL_NUMBER_OF_TEXTURE_TYPES; ++i) {
		delete m_textureQualityWidget [i];
		delete m_textureResolutionWidget [i];
		delete m_textureDepthWidget[i];
	}

}

void OpenGLDialog::OpenGLPrefsByRunning ()
{
	m_cancelWidget->set_callback (boost::bind (&OpenGLDialog::Stop, this, false));
	m_okWidget->set_callback (boost::bind (&OpenGLDialog::Stop, this, true));
	
	BinderSet binders;
	
	BitPref zBufferPref (graphics_preferences->OGL_Configure.Flags, OGL_Flag_ZBuffer);
	binders.insert<bool> (m_zBufferWidget, &zBufferPref);
	BitPref fogPref (graphics_preferences->OGL_Configure.Flags, OGL_Flag_Fog);
	binders.insert<bool> (m_fogWidget, &fogPref);
	BitPref staticEffectsPref (graphics_preferences->OGL_Configure.Flags, OGL_Flag_FlatStatic, true);
	binders.insert<bool> (m_staticEffectWidget, &staticEffectsPref);
	BitPref colourEffectsPref (graphics_preferences->OGL_Configure.Flags, OGL_Flag_Fader);
	binders.insert<bool> (m_colourEffectsWidget, &colourEffectsPref);
	BitPref transparentLiquidsPref (graphics_preferences->OGL_Configure.Flags, OGL_Flag_LiqSeeThru);
	binders.insert<bool> (m_transparentLiquidsWidget, &transparentLiquidsPref);
	BitPref modelsPref (graphics_preferences->OGL_Configure.Flags, OGL_Flag_3D_Models);
	binders.insert<bool> (m_3DmodelsWidget, &modelsPref);
	
	BitPref colourTheVoidPref (graphics_preferences->OGL_Configure.Flags, OGL_Flag_VoidColor);
	binders.insert<bool> (m_colourTheVoidWidget, &colourTheVoidPref);
	ColourPref voidColourPref (graphics_preferences->OGL_Configure.VoidColor);
	binders.insert<RGBColor> (m_voidColourWidget, &voidColourPref);
	
	TimesTwoPref fsaaPref (graphics_preferences->OGL_Configure.Multisamples);
	binders.insert<int> (m_fsaaWidget, &fsaaPref);
	
	AnisotropyPref anisotropyPref (graphics_preferences->OGL_Configure.AnisotropyLevel);
	binders.insert<int> (m_anisotropicWidget, &anisotropyPref);
	
	BoolPref geForceFixPref (graphics_preferences->OGL_Configure.GeForceFix);
	binders.insert<bool> (m_geForceFixWidget, &geForceFixPref);
	
	FilterPref wallsFilterPref (graphics_preferences->OGL_Configure.TxtrConfigList [OGL_Txtr_Wall].FarFilter);
	binders.insert<int> (m_wallsFilterWidget, &wallsFilterPref);
	FilterPref spritesFilterPref (graphics_preferences->OGL_Configure.TxtrConfigList [OGL_Txtr_Inhabitant].FarFilter);
	binders.insert<int> (m_spritesFilterWidget, &spritesFilterPref);
	
	TexQualityPref wallQualityPref (graphics_preferences->OGL_Configure.TxtrConfigList [0].MaxSize, 128);
	binders.insert<int> (m_textureQualityWidget [0], &wallQualityPref);
	TexQualityPref landscapeQualityPref (graphics_preferences->OGL_Configure.TxtrConfigList [1].MaxSize, 256);
	binders.insert<int> (m_textureQualityWidget [1], &landscapeQualityPref);
	TexQualityPref spriteQualityPref (graphics_preferences->OGL_Configure.TxtrConfigList [2].MaxSize, 256);
	binders.insert<int> (m_textureQualityWidget [2], &spriteQualityPref);
	TexQualityPref weaponQualityPref (graphics_preferences->OGL_Configure.TxtrConfigList [3].MaxSize, 256);
	binders.insert<int> (m_textureQualityWidget [3], &weaponQualityPref);
	TexQualityPref modelQualityPref (graphics_preferences->OGL_Configure.ModelConfig.MaxSize, 256);
	binders.insert<int> (m_modelQualityWidget, &modelQualityPref);
	
	Int16Pref wallResoPref (graphics_preferences->OGL_Configure.TxtrConfigList [0].Resolution);
	binders.insert<int> (m_textureResolutionWidget [0], &wallResoPref);
	Int16Pref landscapeResoPref (graphics_preferences->OGL_Configure.TxtrConfigList [1].Resolution);
	binders.insert<int> (m_textureResolutionWidget [1], &landscapeResoPref);
	Int16Pref spriteResoPref (graphics_preferences->OGL_Configure.TxtrConfigList [2].Resolution);
	binders.insert<int> (m_textureResolutionWidget [2], &spriteResoPref);
	Int16Pref weaponResoPref (graphics_preferences->OGL_Configure.TxtrConfigList [3].Resolution);
	binders.insert<int> (m_textureResolutionWidget [3], &weaponResoPref);

	Int16Pref wallDepthPref(graphics_preferences->OGL_Configure.TxtrConfigList[0].ColorFormat);
	binders.insert<int> (m_textureDepthWidget[0], &wallDepthPref);
	Int16Pref landscapeDepthPref(graphics_preferences->OGL_Configure.TxtrConfigList[1].ColorFormat);
	binders.insert<int> (m_textureDepthWidget[1], &landscapeDepthPref);
	Int16Pref spriteDepthPref(graphics_preferences->OGL_Configure.TxtrConfigList[2].ColorFormat);
	binders.insert<int> (m_textureDepthWidget[2], &spriteDepthPref);
	Int16Pref weaponDepthPref(graphics_preferences->OGL_Configure.TxtrConfigList[3].ColorFormat);
	binders.insert<int> (m_textureDepthWidget[3], &weaponDepthPref);
	
	
	// Set initial values from prefs
	binders.migrate_all_second_to_first ();
	
	bool result = Run ();
	
	if (result) {
		// migrate prefs and save
		binders.migrate_all_first_to_second ();
		write_preferences ();
	}
}

static const char *filter_labels[4] = {
	"Linear", "Bilinear", "Trilinear", NULL
};

class SdlOpenGLDialog : public OpenGLDialog
{
public:
	SdlOpenGLDialog ()
	{

		vertical_placer *placer = new vertical_placer;
		placer->dual_add(new w_title("OPENGL OPTIONS"), m_dialog);
		placer->add(new w_spacer(), true);
		
		// horizontal_placer *tabs_placer = new horizontal_placer;
		// w_button *w_general_tab = new w_button("GENERAL");
		// w_general_tab->set_callback(choose_generic_tab, static_cast<void *>(this));
		// tabs_placer->dual_add(w_general_tab, m_dialog);
		// w_button *w_advanced_tab = new w_button("ADVANCED");
		// w_advanced_tab->set_callback(choose_advanced_tab, static_cast<void *>(this));
		// tabs_placer->dual_add(w_advanced_tab, m_dialog);
		// placer->add(tabs_placer, true);

		m_tabs = new tab_placer();

		std::vector<std::string> labels;
		labels.push_back("GENERAL");
		labels.push_back("ADVANCED");
		w_tab *tabs = new w_tab(labels, m_tabs);
		placer->dual_add(tabs, m_dialog);
		
		placer->add(new w_spacer(), true);

		table_placer *general_table = new table_placer(2, get_theme_space(ITEM_WIDGET), true);
		general_table->col_flags(0, placeable::kAlignRight);
		
		w_toggle *zbuffer_w = new w_toggle(false);
		general_table->dual_add(zbuffer_w->label("Z Buffer"), m_dialog);
		general_table->dual_add(zbuffer_w, m_dialog);

		w_toggle *fog_w = new w_toggle(false);
		general_table->dual_add(fog_w->label("Fog"), m_dialog);
		general_table->dual_add(fog_w, m_dialog);

		w_toggle *static_w = new w_toggle(false);
		general_table->dual_add(static_w->label("Static Effect"), m_dialog);
		general_table->dual_add(static_w, m_dialog);

		w_toggle *fader_w = new w_toggle(false);
		general_table->dual_add(fader_w->label("Color Effects"), m_dialog);
		general_table->dual_add(fader_w, m_dialog);

		w_toggle *liq_w = new w_toggle(false);
		general_table->dual_add(liq_w->label("Transparent Liquids"), m_dialog);
		general_table->dual_add(liq_w, m_dialog);

		w_toggle *models_w = new w_toggle(false);
		general_table->dual_add(models_w->label("3D Models"), m_dialog);
		general_table->dual_add(models_w, m_dialog);

		general_table->add_row(new w_spacer(), true);

		w_select_popup *fsaa_w = new w_select_popup ();
		general_table->dual_add(fsaa_w->label("Full Scene Antialiasing"), m_dialog);
		general_table->dual_add(fsaa_w, m_dialog);
		vector<string> fsaa_strings;
		fsaa_strings.push_back ("Off");
		fsaa_strings.push_back ("2x");
		fsaa_strings.push_back ("4x");
		fsaa_w->set_labels (fsaa_strings);
		
		w_slider* aniso_w = new w_slider(6, 1);
		general_table->dual_add(aniso_w->label("Anisotropic Filtering"),m_dialog);
		general_table->dual_add(aniso_w, m_dialog);

		general_table->add_row(new w_spacer(), true);

		general_table->dual_add_row(new w_static_text("Replacement Texture Quality"), m_dialog);
	
		w_select_popup *texture_quality_wa[OGL_NUMBER_OF_TEXTURE_TYPES];
		for (int i = 0; i < OGL_NUMBER_OF_TEXTURE_TYPES; i++) texture_quality_wa[i] = NULL;
		
		texture_quality_wa[OGL_Txtr_Wall] =  new w_select_popup ();
		general_table->dual_add(texture_quality_wa[OGL_Txtr_Wall]->label("Walls"), m_dialog);
		general_table->dual_add(texture_quality_wa[OGL_Txtr_Wall], m_dialog);
		
		texture_quality_wa[OGL_Txtr_Landscape] = new w_select_popup ();
		general_table->dual_add(texture_quality_wa[OGL_Txtr_Landscape]->label("Landscapes"), m_dialog);
		general_table->dual_add(texture_quality_wa[OGL_Txtr_Landscape], m_dialog);

		texture_quality_wa[OGL_Txtr_Inhabitant] = new w_select_popup ();
		general_table->dual_add(texture_quality_wa[OGL_Txtr_Inhabitant]->label("Sprites"), m_dialog);
		general_table->dual_add(texture_quality_wa[OGL_Txtr_Inhabitant], m_dialog);

		texture_quality_wa[OGL_Txtr_WeaponsInHand] = new w_select_popup ();
		general_table->dual_add(texture_quality_wa[OGL_Txtr_WeaponsInHand]->label("Weapons in Hand"), m_dialog);
		general_table->dual_add(texture_quality_wa[OGL_Txtr_WeaponsInHand], m_dialog);

		w_select_popup *model_quality_w = new w_select_popup();
		general_table->dual_add(model_quality_w->label("3D Model Skins"), m_dialog);
		general_table->dual_add(model_quality_w, m_dialog);
	
		vector<string> tex_quality_strings;
		tex_quality_strings.push_back ("Unlimited");
		tex_quality_strings.push_back ("Normal");
		tex_quality_strings.push_back ("High");
		tex_quality_strings.push_back ("Higher");
		tex_quality_strings.push_back ("Highest");
	
		for (int i = 0; i < OGL_NUMBER_OF_TEXTURE_TYPES; i++) {
			if (texture_quality_wa[i]) {
				texture_quality_wa[i]->set_labels (tex_quality_strings);
			}
		}
		model_quality_w->set_labels(tex_quality_strings);

		vertical_placer *advanced_placer = new vertical_placer;

		table_placer *advanced_table = new table_placer(2, get_theme_space(ITEM_WIDGET), true);
		advanced_table->col_flags(0, placeable::kAlignRight);
	
		w_toggle *geforce_fix_w = new w_toggle(false);
		advanced_table->dual_add(geforce_fix_w->label("GeForce 1-4 Texture Fix"), m_dialog);
		advanced_table->dual_add(geforce_fix_w, m_dialog);
		
		advanced_table->add_row(new w_spacer(), true);
		advanced_table->dual_add_row(new w_static_text("Distant Texture Filtering"), m_dialog);

		w_select *wall_filter_w = new w_select(0, filter_labels);
		advanced_table->dual_add(wall_filter_w->label("Walls"), m_dialog);
		advanced_table->dual_add(wall_filter_w, m_dialog);

		w_select *sprite_filter_w = new w_select(0, filter_labels);
		advanced_table->dual_add(sprite_filter_w->label("Sprites"), m_dialog);
		advanced_table->dual_add(sprite_filter_w, m_dialog);

		advanced_placer->add(advanced_table, true);

		advanced_placer->add(new w_spacer(), true);
		w_select_popup *texture_resolution_wa[OGL_NUMBER_OF_TEXTURE_TYPES];
		w_select_popup *texture_depth_wa[OGL_NUMBER_OF_TEXTURE_TYPES];
		for (int i = 0; i < OGL_NUMBER_OF_TEXTURE_TYPES; i++) 
		{
			texture_resolution_wa[i] = new w_select_popup();
			texture_depth_wa[i] = new w_select_popup();
		}

		w_label *texture_labels[OGL_NUMBER_OF_TEXTURE_TYPES];
		texture_labels[OGL_Txtr_Wall] = new w_label("Walls");
		texture_labels[OGL_Txtr_Landscape] = new w_label("Landscapes");
		texture_labels[OGL_Txtr_Inhabitant] = new w_label("Sprites");
		texture_labels[OGL_Txtr_WeaponsInHand] = new w_label("Weapons in Hand / HUD");

		advanced_placer->dual_add(new w_static_text("Built-in Texture Size and Depth"), m_dialog);
		advanced_placer->dual_add(new w_static_text("(reduce for machines with low VRAM)"), m_dialog);

		vector<string> tex_reso_strings;
		tex_reso_strings.push_back ("Full");
		tex_reso_strings.push_back ("1/2");
		tex_reso_strings.push_back ("1/4");

		vector<string> tex_depth_strings;
		tex_depth_strings.push_back ("32-bit");
		tex_depth_strings.push_back ("16-bit");
		tex_depth_strings.push_back ("8-bit");

		table_placer *table = new table_placer(3, get_theme_space(ITEM_WIDGET));

		table->col_flags(0, placeable::kAlignRight);
		table->col_flags(1, placeable::kAlignLeft);
		table->col_flags(2, placeable::kAlignLeft);

		table->add(new w_spacer(), true);
		table->dual_add(new w_label("Size"), m_dialog);
		table->dual_add(new w_label("Depth"), m_dialog);

		for (int i = 0; i < OGL_NUMBER_OF_TEXTURE_TYPES; i++)
		{
			table->dual_add(texture_labels[i], m_dialog);
			table->dual_add(texture_resolution_wa[i], m_dialog);
			table->dual_add(texture_depth_wa[i], m_dialog);

			texture_resolution_wa[i]->associate_label(texture_labels[i]);
			texture_resolution_wa[i]->set_labels(tex_reso_strings);

			texture_depth_wa[i]->associate_label(texture_labels[i]);
			texture_depth_wa[i]->set_labels(tex_depth_strings);
		}

		table->col_min_width(1, (table->col_width(0) - get_theme_space(ITEM_WIDGET)) / 2);
		table->col_min_width(2, (table->col_width(0) - get_theme_space(ITEM_WIDGET)) / 2);

		advanced_placer->add(table, true);

		m_tabs->add(general_table, true);
		m_tabs->add(advanced_placer, true);
		placer->add(m_tabs, false);
	
		placer->add(new w_spacer(), true);

		horizontal_placer *button_placer = new horizontal_placer;
		w_button* ok_w = new w_button("ACCEPT");
		button_placer->dual_add(ok_w, m_dialog);
		
		w_button* cancel_w = new w_button("CANCEL");
		button_placer->dual_add(cancel_w, m_dialog);
		placer->add(button_placer, true);

		m_dialog.set_widget_placer(placer);

		m_cancelWidget = new ButtonWidget (cancel_w);
		m_okWidget = new ButtonWidget (ok_w);
		
		m_zBufferWidget = new ToggleWidget (zbuffer_w);
		m_fogWidget = new ToggleWidget (fog_w);
		m_staticEffectWidget = new ToggleWidget (static_w);
		m_colourEffectsWidget = new ToggleWidget (fader_w);
		m_transparentLiquidsWidget = new ToggleWidget (liq_w);
		m_3DmodelsWidget = new ToggleWidget (models_w);

		m_colourTheVoidWidget = 0;
		m_voidColourWidget = 0;

		m_fsaaWidget = new PopupSelectorWidget (fsaa_w);

		m_anisotropicWidget = new SliderSelectorWidget (aniso_w);

		m_geForceFixWidget = new ToggleWidget (geforce_fix_w);
		
		m_wallsFilterWidget = new SelectSelectorWidget (wall_filter_w);
		m_spritesFilterWidget = new SelectSelectorWidget (sprite_filter_w);

		for (int i = 0; i < OGL_NUMBER_OF_TEXTURE_TYPES; ++i) {
			m_textureQualityWidget [i] = new PopupSelectorWidget (texture_quality_wa[i]);
			m_textureResolutionWidget [i] = new PopupSelectorWidget (texture_resolution_wa[i]);
			m_textureDepthWidget [i] = new PopupSelectorWidget(texture_depth_wa[i]);
		}
		m_modelQualityWidget = new PopupSelectorWidget(model_quality_w);
	}

	~SdlOpenGLDialog() {
		delete m_tabs;
	}

	virtual bool Run ()
	{	
		return (m_dialog.run () == 0);
	}

	virtual void Stop (bool result)
	{
		m_dialog.quit (result ? 0 : -1);
	}

	static void choose_generic_tab(void *arg);
	static void choose_advanced_tab(void *arg);

private:
	enum {
		TAB_WIDGET = 400,
		BASIC_TAB,
		ADVANCED_TAB
	};
	
	tab_placer* m_tabs;
	dialog m_dialog;
};

void SdlOpenGLDialog::choose_generic_tab(void *arg)
{
	SdlOpenGLDialog *d = static_cast<SdlOpenGLDialog *>(arg);
	d->m_tabs->choose_tab(0);
	d->m_dialog.draw();
}

void SdlOpenGLDialog::choose_advanced_tab(void *arg)
{
	SdlOpenGLDialog *d = static_cast<SdlOpenGLDialog *>(arg);
	d->m_tabs->choose_tab(1);
	d->m_dialog.draw();
}

auto_ptr<OpenGLDialog>
OpenGLDialog::Create()
{
	return auto_ptr<OpenGLDialog>(new SdlOpenGLDialog);
}
