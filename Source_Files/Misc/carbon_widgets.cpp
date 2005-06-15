
/*

	Copyright (C) 2005 and beyond by the "Aleph One" developers.
 
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

/*
 *  carbon_widgets.cpp - Widgets for carbon dialogs
 *
 */

#include "carbon_widgets.h"

void StaticTextWidget::set_text (std::string s)
{
	SetControlData(m_ctrl, kControlLabelPart, kControlStaticTextTextTag, s.length (), s.c_str ());
	Draw1Control (m_ctrl);
}

void EditTextWidget::set_text (std::string s)
{
	SetControlData(m_ctrl, kControlEditTextPart, kControlEditTextTextTag, s.length (), s.c_str ());
	Draw1Control (m_ctrl);
}

const string EditTextWidget::get_text ()
{
	Size size = 0;
	GetControlDataSize(m_ctrl, kControlEditTextPart, kControlEditTextTextTag, &size);

	std::vector<char> buffer(size);
	GetControlData(m_ctrl, kControlEditTextPart, kControlEditTextTextTag, buffer.size(), &buffer[0], NULL);
	return std::string(&buffer[0], buffer.size());
}
