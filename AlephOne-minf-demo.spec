Name: AlephOne-minf-demo
Summary: AlephOne data files for Marathon Infinity Demo
Group: X11/Games/Video
Version: 1.0
Release: 1
License: Demo
Group: X11/Games/Video
Source: InfinityDemo_data.tar.gz
URL: http://www.uni-mainz.de/~bauec002/A1Main.html
BuildRoot: %{_tmppath}/%{name}-root
BuildArchitectures: noarch
Provides: AlephOne-core-data
Prefix: %{_prefix}

%description
This package contains shape, sound, and map information from Bungie
Software's Marathon Infinity Demo. It can be used as AlephOne's ``core
data''-- the foundation upon which other maps can be built -- in addition to
providing the Infinity Demo itself, a fun (if short) series of levels.

%prep

%build

%install
rm -rf ${RPM_BUILD_ROOT}/usr/share/AlephOne &&
mkdir -p ${RPM_BUILD_ROOT}/usr/share/AlephOne &&
cd ${RPM_BUILD_ROOT}/usr/share/AlephOne &&
tar zxf %{_sourcedir}/InfinityDemo_data.tar.gz

%files
%{_datadir}/AlephOne/Map
%{_datadir}/AlephOne/Images
%{_datadir}/AlephOne/Shapes
%{_datadir}/AlephOne/Sounds
%doc %{_datadir}/AlephOne/README


%changelog
* Thu Oct  5 2000 Christian Bauer <Christian.Bauer@uni-mainz.de>
- Renamed and tweaked a bit

* Fri Sep 30 2000 Tom Moertel <tom-rpms-alephone@moertel.com>
- First cut.
