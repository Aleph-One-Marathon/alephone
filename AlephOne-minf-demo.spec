Name: AlephOne-minf-demo
Summary: AlephOne data files for Marathon Infinity Demo
Group: Amusements/Games
Version: 1.0
Release: 2
License: Demo
Source: AlephOne-minf-demo.tar.gz
URL: http://www.uni-mainz.de/~bauec002/A1Main.html
Vendor: Bungie Software Corporation
BuildRoot: %{_tmppath}/%{name}-root
BuildArchitectures: noarch
Provides: AlephOne-core-data
# not relocatable because of absolute paths in "start" script

%description
This package contains shape, sound, and map information from Bungie
Software's Marathon Infinity Demo. It can be used as AlephOne's "core
data" -- the foundation upon which other maps can be built -- in addition to
providing the Infinity Demo itself, a fun (if short) series of levels.

Use the included "start" script to run Aleph One with the Marathon Infinity
Demo files.

%prep

%build

%install
rm -rf ${RPM_BUILD_ROOT}/usr/share/AlephOne_minf_demo &&
mkdir -p ${RPM_BUILD_ROOT}/usr/share/AlephOne_minf_demo &&
cd ${RPM_BUILD_ROOT}/usr/share/AlephOne_minf_demo &&
tar zxf %{_sourcedir}/AlephOne-minf-demo.tar.gz

%files
%defattr(-,root,root)
%{_prefix}/share/AlephOne_minf_demo/start
%{_prefix}/share/AlephOne_minf_demo/Map
%{_prefix}/share/AlephOne_minf_demo/Images
%{_prefix}/share/AlephOne_minf_demo/Shapes
%{_prefix}/share/AlephOne_minf_demo/Sounds
%{_prefix}/share/AlephOne_minf_demo/MML/*.mml
%{_prefix}/share/AlephOne_minf_demo/MML
%doc %{_prefix}/share/AlephOne_minf_demo/README


%changelog
* Tue Oct 17 2000 Christian Bauer <Christian.Bauer@uni-mainz.de>
- No longer installed in Aleph One default data directory
- Added a script to set the data path and start the game

* Thu Oct  5 2000 Christian Bauer <Christian.Bauer@uni-mainz.de>
- Renamed and tweaked a bit

* Fri Sep 30 2000 Tom Moertel <tom-rpms-alephone@moertel.com>
- First cut.
