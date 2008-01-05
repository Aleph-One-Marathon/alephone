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
	
	Int16Pref wallResoPref (graphics_preferences->OGL_Configure.TxtrConfigList [0].Resolution);
	binders.insert<int> (m_textureResolutionWidget [0], &wallResoPref);
	Int16Pref landscapeResoPref (graphics_preferences->OGL_Configure.TxtrConfigList [1].Resolution);
	binders.insert<int> (m_textureResolutionWidget [1], &landscapeResoPref);
	Int16Pref spriteResoPref (graphics_preferences->OGL_Configure.TxtrConfigList [2].Resolution);
	binders.insert<int> (m_textureResolutionWidget [2], &spriteResoPref);
	Int16Pref weaponResoPref (graphics_preferences->OGL_Configure.TxtrConfigList [3].Resolution);
	binders.insert<int> (m_textureResolutionWidget [3], &weaponResoPref);
	
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
		m_dialog.add(new w_static_text("OPENGL OPTIONS", TITLE_FONT, TITLE_COLOR));
		m_dialog.add(new w_spacer());
		
		vector<string> tab_strings;
		tab_strings.push_back ("GENERAL");
		tab_strings.push_back ("ADVANCED");
		make_tab_buttons_for_dialog (m_dialog, tab_strings, TAB_WIDGET);
		m_dialog.set_active_tab (BASIC_TAB);
		
		m_dialog.add(new w_spacer());
		
		w_toggle *zbuffer_w = new w_toggle("Z Buffer", false);
		m_dialog.add_to_tab(zbuffer_w, BASIC_TAB);
		w_toggle *fog_w = new w_toggle("Fog", false);
		m_dialog.add_to_tab(fog_w, BASIC_TAB);
		w_toggle *static_w = new w_toggle("Static Effect", false);
		m_dialog.add_to_tab(static_w, BASIC_TAB);
		w_toggle *fader_w = new w_toggle("Color Effects", false);
		m_dialog.add_to_tab(fader_w, BASIC_TAB);
		w_toggle *liq_w = new w_toggle("Transparent Liquids", false);
		m_dialog.add_to_tab(liq_w, BASIC_TAB);
		w_toggle *models_w = new w_toggle("3D Models", false);
		m_dialog.add_to_tab(models_w, BASIC_TAB);
		m_dialog.add_to_tab(new w_spacer(), BASIC_TAB);

		w_select_popup *fsaa_w = new w_select_popup ("Full Scene Antialiasing");
		m_dialog.add_to_tab(fsaa_w, BASIC_TAB);
		vector<string> fsaa_strings;
		fsaa_strings.push_back ("off");
		fsaa_strings.push_back ("x2");
		fsaa_strings.push_back ("x4");
		fsaa_w->set_labels (fsaa_strings);
		
		w_slider* aniso_w = new w_slider("Anisotropic Filtering", 6, 1);
		m_dialog.add_to_tab(aniso_w, BASIC_TAB);
		m_dialog.add_to_tab(new w_spacer(), BASIC_TAB);

		m_dialog.add_to_tab(new w_static_text("Replacement Texture Quality"), BASIC_TAB);
	
		w_select_popup *texture_quality_wa[OGL_NUMBER_OF_TEXTURE_TYPES];
		for (int i = 0; i < OGL_NUMBER_OF_TEXTURE_TYPES; i++) texture_quality_wa[i] = NULL;
		
		texture_quality_wa[OGL_Txtr_Wall] =  new w_select_popup ("Walls");
		texture_quality_wa[OGL_Txtr_Landscape] = new w_select_popup ("Landscapes");
		texture_quality_wa[OGL_Txtr_Inhabitant] = new w_select_popup ("Sprites");
		texture_quality_wa[OGL_Txtr_WeaponsInHand] = new w_select_popup ("Weapons in Hand");
	
		vector<string> tex_quality_strings;
		tex_quality_strings.push_back ("Unlimited");
		tex_quality_strings.push_back ("normal");
		tex_quality_strings.push_back ("high");
		tex_quality_strings.push_back ("higher");
		tex_quality_strings.push_back ("highest");
	
		for (int i = 0; i < OGL_NUMBER_OF_TEXTURE_TYPES; i++) {
			if (texture_quality_wa[i]) {
				m_dialog.add_to_tab(texture_quality_wa[i], BASIC_TAB);
				texture_quality_wa[i]->set_labels (tex_quality_strings);
			}
		}
	
		w_toggle *geforce_fix_w = new w_toggle("GeForce 1-4 Texture Fix", false);
		m_dialog.add_to_tab(geforce_fix_w, ADVANCED_TAB);
		
		m_dialog.add_to_tab(new w_spacer(), ADVANCED_TAB);

		m_dialog.add_to_tab(new w_static_text("Distant Texture Filtering"), ADVANCED_TAB);
		w_select *wall_filter_w = new w_select("Walls", 0, filter_labels);
		m_dialog.add_to_tab(wall_filter_w, ADVANCED_TAB);

		w_select *sprite_filter_w = new w_select("Sprites", 0, filter_labels);
		m_dialog.add_to_tab(sprite_filter_w, ADVANCED_TAB);

		m_dialog.add_to_tab(new w_spacer(), ADVANCED_TAB);

		w_select_popup *texture_resolution_wa[OGL_NUMBER_OF_TEXTURE_TYPES];
		for (int i = 0; i < OGL_NUMBER_OF_TEXTURE_TYPES; i++) texture_resolution_wa[i] = NULL;
		texture_resolution_wa[OGL_Txtr_Wall] = new w_select_popup("Walls");
		texture_resolution_wa[OGL_Txtr_Landscape] = new w_select_popup("Landscapes");
		texture_resolution_wa[OGL_Txtr_Inhabitant] = new w_select_popup("Sprites");
		texture_resolution_wa[OGL_Txtr_WeaponsInHand] = new w_select_popup("Weapons in Hand / HUD");

		m_dialog.add_to_tab(new w_static_text("Texture Resolution (reduce for machines with low VRAM)"), ADVANCED_TAB);

		vector<string> tex_reso_strings;
		tex_reso_strings.push_back ("full");
		tex_reso_strings.push_back ("1/2");
		tex_reso_strings.push_back ("1/4");

		for (int i = 0; i < OGL_NUMBER_OF_TEXTURE_TYPES; i++) {
			if (texture_resolution_wa[i]) {
				m_dialog.add_to_tab(texture_resolution_wa[i], ADVANCED_TAB);
				texture_resolution_wa[i]->set_labels (tex_reso_strings);
			}
		}
	
		m_dialog.add(new w_spacer());

		w_left_button* ok_w = new w_left_button("ACCEPT");
		m_dialog.add(ok_w);
		w_right_button* cancel_w = new w_right_button("CANCEL");
		m_dialog.add(cancel_w);

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
		}
	}

	virtual bool Run ()
	{	
		return (m_dialog.run () == 0);
	}

	virtual void Stop (bool result)
	{
		m_dialog.quit (result ? 0 : -1);
	}

private:
	enum {
		TAB_WIDGET = 400,
		BASIC_TAB,
		ADVANCED_TAB
	};
	
	dialog m_dialog;
};

auto_ptr<OpenGLDialog>
OpenGLDialog::Create()
{
	return auto_ptr<OpenGLDialog>(new SdlOpenGLDialog);
}
