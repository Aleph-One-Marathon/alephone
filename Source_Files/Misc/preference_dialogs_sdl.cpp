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

		m_dialog.add_to_tab(new w_static_text("Distant Texture Smoothing"), ADVANCED_TAB);
		w_toggle *wall_mipmap_w = new w_toggle("Mipmap Walls", false);
		m_dialog.add_to_tab(wall_mipmap_w, ADVANCED_TAB);

		w_toggle *sprite_mipmap_w = new w_toggle("Mipmap Sprites", false);
		m_dialog.add_to_tab(sprite_mipmap_w, ADVANCED_TAB);

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
		
		m_mipMapWallsWidget = new ToggleWidget (wall_mipmap_w);
		m_mipMapSpritesWidget = new ToggleWidget (sprite_mipmap_w);

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
