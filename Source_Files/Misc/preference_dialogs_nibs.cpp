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

class NibsOpenGLDialog : public OpenGLDialog
{
public:
	NibsOpenGLDialog::NibsOpenGLDialog ()
	: m_dialog_nib (CFSTR ("Prefs_OpenGL"))
	, m_dialog_window (m_dialog_nib.nibReference (), CFSTR ("Prefs_OpenGL"))
	, m_dialog (m_dialog_window (), true)
	{
		m_cancelWidget = new ButtonWidget (GetCtrlFromWindow (m_dialog_window (), 0, Cancel_Item));
		m_okWidget = new ButtonWidget (GetCtrlFromWindow (m_dialog_window (), 0, OK_Item));
		
		m_zBufferWidget = new ToggleWidget (GetCtrlFromWindow (m_dialog_window (), 0, ZBuffer_Item));
		m_fogWidget = new ToggleWidget (GetCtrlFromWindow (m_dialog_window (), 0, AllowFog_Item));
		m_staticEffectWidget = new ToggleWidget (GetCtrlFromWindow (m_dialog_window (), 0, FlatStaticEffect_Item));
		m_colourEffectsWidget = new ToggleWidget (GetCtrlFromWindow (m_dialog_window (), 0, Fader_Item));
		m_transparentLiquidsWidget = new ToggleWidget (GetCtrlFromWindow (m_dialog_window (), 0, LiquidSeeThru_Item));
		m_3DmodelsWidget = new ToggleWidget (GetCtrlFromWindow (m_dialog_window (), 0, Model_Item));

		m_colourTheVoidWidget = new ToggleWidget (GetCtrlFromWindow (m_dialog_window (), 0, ColorVoid_Item));
		m_voidColourWidget = new ColourPickerWidget (GetCtrlFromWindow (m_dialog_window (), 0, ColorVoidSwatch_Item));

		m_fsaaWidget = 0;

		m_anisotropicWidget = new SelectorWidget (GetCtrlFromWindow (m_dialog_window (), 0, AnisoSlider_Item));

		m_geForceFixWidget = new ToggleWidget (GetCtrlFromWindow (m_dialog_window (), 0, GeForceFix_Item));
		
		m_mipMapWallsWidget = new ToggleWidget (GetCtrlFromWindow (m_dialog_window (), 0, MipMapWalls_Item));
		m_mipMapSpritesWidget = new ToggleWidget (GetCtrlFromWindow (m_dialog_window (), 0, MipMapSprites_Item));

		m_textureQualityWidget [0] = new SelectorWidget (GetCtrlFromWindow (m_dialog_window (), 0, Wall_Quality_Item));
		m_textureQualityWidget [1] = new SelectorWidget (GetCtrlFromWindow (m_dialog_window (), 0, Landscape_Quality_Item));
		m_textureQualityWidget [2] = new SelectorWidget (GetCtrlFromWindow (m_dialog_window (), 0, Sprite_Quality_Item));
		m_textureQualityWidget [3] = new SelectorWidget (GetCtrlFromWindow (m_dialog_window (), 0, Weapon_Quality_Item));
		
		m_textureResolutionWidget [0] = new SelectorWidget (GetCtrlFromWindow (m_dialog_window (), 0, Wall_Reso_Item));
		m_textureResolutionWidget [1] = new SelectorWidget (GetCtrlFromWindow (m_dialog_window (), 0, Landscape_Reso_Item));
		m_textureResolutionWidget [2] = new SelectorWidget (GetCtrlFromWindow (m_dialog_window (), 0, Sprite_Reso_Item));
		m_textureResolutionWidget [3] = new SelectorWidget (GetCtrlFromWindow (m_dialog_window (), 0, Weapon_Reso_Item));
	}

	virtual bool Run ()
	{
		vector<ControlRef> tab_panes;
		tab_panes.push_back (GetCtrlFromWindow (m_dialog_window (), 0, Tab_1));
		tab_panes.push_back (GetCtrlFromWindow (m_dialog_window (), 0, Tab_2));
		AutoTabHandler tab_handler (GetCtrlFromWindow (m_dialog_window (), 0, Tabs_Item), tab_panes, m_dialog_window ());
	
		return m_dialog.Run ();
	}

	virtual void Stop (bool result)
	{
		m_dialog.Stop (result);
	}

private:
	AutoNibReference m_dialog_nib;
	AutoNibWindow m_dialog_window;
	Modal_Dialog m_dialog;

	enum
	{
		OK_Item = 1,
		Cancel_Item = 2,
	
		OpenGL_Dialog = 2000,
		Walls_Item = 5,
		Landscape_Item = 6,
		Inhabitants_Item = 7,
		WeaponsInHand_Item = 8,
		ZBuffer_Item = 9,
		ColorVoid_Item = 10,
		ColorVoidSwatch_Item = 11,
		FlatColorLandscapes_Item = 12,
		
		// The first of 8 items, in order:
		// Day Ground, Day Sky, Night Ground, Night Sky, ...
		LandscapeSwatch_ItemBase = 17,
	
		TwoDimGraphics_Item = 25,
		FlatStaticEffect_Item = 26,
		Fader_Item = 27,
		LiquidSeeThru_Item = 28,
		Map_Item = 29,
		//TextureFix_Item = 30,
		AllowFog_Item = 31,
		Model_Item = 37,
		HUD_Item = 38,
		AnisoSlider_Item = 39,
		Wall_Quality_Item = 40,
		Landscape_Quality_Item = 41,
		Sprite_Quality_Item = 42,
		Weapon_Quality_Item = 43,
		Wall_Reso_Item = 44,
		Landscape_Reso_Item = 45,
		Sprite_Reso_Item = 46,
		Weapon_Reso_Item = 47,
		MipMapWalls_Item = 48,
		MipMapSprites_Item = 49,
		GeForceFix_Item = 50,
		
		Tabs_Item = 400,
		Tab_1 = 401,
		Tab_2 = 402,
	
		ColorPicker_PromptStrings = 200,
		ColorVoid_String = 0,
		FlatColorLandscapes_StringBase = 1,
		FogColor_String = 9,
	
		OpenGL_Textures_Dialog = 2100,
		WhichOne_Item = 4,
		Near_Item = 5,
		Far_Item = 6,
		Resolution_Item = 7,
		ColorDepth_Item = 8,
		BasedOn_Item = 9,
	
		BasedOn_Menu = 12140
	};
};

auto_ptr<OpenGLDialog>
OpenGLDialog::Create()
{
	return auto_ptr<OpenGLDialog>(new NibsOpenGLDialog);
}
