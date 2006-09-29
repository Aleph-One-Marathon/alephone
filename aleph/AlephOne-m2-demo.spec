Name: AlephOne-m2-demo
Summary: AlephOne data files for Marathon 2 Demo
Group: Amusements/Games
Version: 1.0
Release: 1
License: Demo
Source: AlephOne-m2-demo.tar.gz
URL: http://www.uni-mainz.de/~bauec002/A1Main.html
Vendor: Bungie Software Corporation
BuildRoot: %{_tmppath}/%{name}-root
BuildArchitectures: noarch
Provides: AlephOne-core-data
# not relocatable because of absolute paths in "start" script

%description
This package contains shape, sound, and map information from Bungie
Software's Marathon 2 Demo. It can be used as AlephOne's "core data"--
the foundation upon which other maps can be built -- in addition to
providing the Marathon 2 Demo itself, a fun (if short) series of levels.

Use the included "start" script to run Aleph One with the Marathon 2 Demo
files.

%prep

%build

%install
rm -rf ${RPM_BUILD_ROOT}/usr/share/AlephOne_m2_demo &&
mkdir -p ${RPM_BUILD_ROOT}/usr/share/AlephOne_m2_demo &&
cd ${RPM_BUILD_ROOT}/usr/share/AlephOne_m2_demo &&
tar zxf %{_sourcedir}/AlephOne-m2-demo.tar.gz

%files
%defattr(-,root,root)
%{_prefix}/share/AlephOne_m2_demo/start
%{_prefix}/share/AlephOne_m2_demo/Map
%{_prefix}/share/AlephOne_m2_demo/Images
%{_prefix}/share/AlephOne_m2_demo/Shapes
%{_prefix}/share/AlephOne_m2_demo/Sounds
%{_prefix}/share/AlephOne_m2_demo/MML/*.mml
%{_prefix}/share/AlephOne_m2_demo/MML


%changelog
* Tue Oct 17 2000 Christian Bauer <Christian.Bauer@uni-mainz.de>
- Created from the AlephOne-minf-demo.spec
