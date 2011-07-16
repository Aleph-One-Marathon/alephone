
/*

	Copyright (C) 2006 and beyond by Bungie Studios, Inc.
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

#ifndef BINDERS_H
#define BINDERS_H

#include <list>
#include <algorithm>

template<typename T>
class Bindable
{
public:
	virtual T bind_export () = 0;
	virtual void bind_import (T) = 0;
	
	virtual ~Bindable () {}
};

class ABinder
{
public:
	virtual void migrate_first_to_second () = 0;
	virtual void migrate_second_to_first () = 0;
	
	virtual ~ABinder () {}
};

template<typename T>
class Binder : public ABinder
{
public:
	Binder (Bindable<T>* t1, Bindable<T>* t2)
		: thing1 (t1)
		, thing2 (t2)
		{}

	void migrate_first_to_second () { thing2->bind_import (thing1->bind_export ()); }
	void migrate_second_to_first () { thing1->bind_import (thing2->bind_export ()); }

private:
	Bindable<T>* thing1;
	Bindable<T>* thing2;	
};

class BinderSet
{
public:
	BinderSet () {}
	~BinderSet () { for_each (m_list.begin (), m_list.end (), delete_it); }

	template<typename T>
	void insert (Bindable<T>* first, Bindable<T>* second)
	{ if (first && second) m_list.push_back (new Binder<T> (first, second)); }
	
	void migrate_all_first_to_second () { for_each (m_list.begin (), m_list.end (), call_first_second); }
	void migrate_all_second_to_first () { for_each (m_list.begin (), m_list.end (), call_second_first); }

private:
	std::list<ABinder*> m_list;
	
	static void call_first_second (ABinder* b) { b->migrate_first_to_second (); }
	static void call_second_first (ABinder* b) { b->migrate_second_to_first (); }
	static void delete_it (ABinder* b) { delete b; }
};

#endif
