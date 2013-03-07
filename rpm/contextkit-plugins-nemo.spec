Name:       contextkit-plugins-nemo
Summary:    ContextKit providers for Nemo Mobile
Version:    0.7.39
Release:    1
Group:      Applications/System
License: LGPLv2
URL:        https://github.com/nemomobile/contextkit-nemo
Source0:    %{name}-%{version}.tar.bz2
Requires: contextkit >= 0.5.39
BuildRequires:  pkgconfig(QtCore)
BuildRequires:  pkgconfig(contextprovider-1.0)
BuildRequires:  pkgconfig(contextsubscriber-1.0) >= 0.5.25
BuildRequires:  cmake

%description
ContextKit providers common for all Nemo Mobile platforms.

%define meego_ver 0.1.0
%define meego_ver1 0.1.0.1
%define maemo_ver 0.7.30
%define maemo_ver1 0.7.31

%define p_bluez -n contextkit-plugin-bluez
%define g_bluetooth contextkit-plugin-bluetooth
%define p_upower -n contextkit-plugin-upower
%define g_power contextkit-plugin-power
%define p_session -n contextkit-plugin-session
%define g_session contextkit-plugin-session
%define p_profile -n contextkit-plugin-profile
%define g_profile contextkit-plugin-profile
%define p_connman -n contextkit-plugin-connman
%define g_internet contextkit-plugin-internet
%define p_cellular -n contextkit-plugin-cellular
%define g_cellular contextkit-plugin-cellular
%define p_ofono -n contextkit-plugin-ofono
%define g_phone contextkit-plugin-phone
%define p_music -n contextkit-plugin-meego-music
%define g_media contextkit-plugin-media
%define p_mce -n contextkit-plugin-mce
%define g_mce contextkit-plugin-mce
%define p_gypsy -n contextkit-plugin-location-gypsy
%define p_skyhook -n contextkit-plugin-location-skyhook
%define g_location contextkit-plugin-location
%define p_keyboard -n contextkit-plugin-keyboard-generic
%define g_keyboard contextkit-plugin-keyboard

%define g_all %{g_bluetooth}, %{g_power}, %{g_session}, %{g_profile}, %{g_internet}, %{g_cellular}, %{g_cellular}, %{g_phone}, %{g_media}, %{g_mce}, %{g_location}

%define plugins_dir %{_libdir}/contextkit/subscriber-plugins
%define context_dir %{_datadir}/contextkit/providers
%define tests_dir %{_bindir}

%package %{p_bluez}
Summary: Bluez ContextKit plugin
Group: Applications/System
BuildRequires: pkgconfig(dbus-1)
Provides: %{g_bluetooth}
Obsoletes: contextkit-maemo <= %{maemo_ver}
Provides: contextkit-maemo = %{maemo_ver1}
Obsoletes: contextkit-meego <= %{meego_ver}
Provides: contextkit-meego = %{meego_ver1}
Obsoletes: contextkit-meego-bluetooth <= %{meego_ver}
Provides: contextkit-meego-bluetooth = %{meego_ver1}
%description %{p_bluez}
%{summary}

%package %{p_upower}
Summary:    UPower ContextKit plugin
License: Apache
Group:      Applications/System
Requires:   upower >= 0.9.18
BuildRequires:  pkgconfig(gconf-2.0)
BuildRequires:  pkgconfig(mlite)
Obsoletes: contextkit-meego-battery-upower <= %{meego_ver}
Provides: contextkit-meego-battery-upower = %{meego_ver1}
Provides: %{g_power}
%description %{p_upower}
%{summary}

%package %{p_session}
Summary:    Session ContextKit plugin
License: LGPLv2
Group:      Applications/System
BuildRequires: pkgconfig(x11)
%description %{p_session}
%{summary}

%package %{p_profile}
Summary:    Profile ContextKit plugin
License: LGPLv2
Group:      Applications/System
BuildRequires: pkgconfig(dbus-1)
%description %{p_profile}
%{summary}

%package %{p_connman}
Summary:    Connman ContextKit plugin
License: Apache
Group:      Applications/System
BuildRequires: pkgconfig(dbus-1)
BuildRequires: pkgconfig(connman-qt4)
Provides: %{g_internet}
Obsoletes: contextkit-meego-internet <= %{meego_ver}
Provides: contextkit-meego-internet = %{meego_ver1}
%description %{p_connman}
%{summary}

%package %{p_cellular}
Summary:    Cellular ContextKit plugin
License: Apache
Group:      Applications/System
Obsoletes: contextkit-meego-cellular <= %{meego_ver}
Provides: contextkit-meego-cellular = %{meego_ver1}
BuildRequires: pkgconfig(dbus-1)
%description %{p_cellular}
%{summary}

%package %{p_ofono}
Summary:    Ofono phone ContextKit plugin
License: Apache
Group:      Applications/System
BuildRequires: pkgconfig(dbus-1)
Provides: %{g_phone}
Obsoletes: contextkit-meego-phone <= %{meego_ver}
Provides: contextkit-meego-phone = %{meego_ver1}
%description %{p_ofono}
%{summary}

%package %{p_music}
Summary:    Meego music ContextKit plugin
License: Apache
Group:      Applications/System
BuildRequires: pkgconfig(dbus-1)
Provides: %{g_media}
Obsoletes: contextkit-meego-media <= %{meego_ver}
Provides: contextkit-meego-media = %{meego_ver1}
%description %{p_music}
%{summary}

%package %{p_mce}
Summary:    MCE ContextKit plugin
License: LGPLv2
Group:      Applications/System
Obsoletes: contextkit-maemo-mce <= %{maemo_ver}
Provides: contextkit-maemo-mce = %{maemo_ver1}
BuildRequires: pkgconfig(dbus-1)
BuildRequires: pkgconfig(mce)
%description %{p_mce}
%{summary}

%package %{p_gypsy}
Summary: Location ContextKit plugin using org.freedesktop.Gypsy
License: Apache
Group: Applications/System
BuildRequires: pkgconfig(dbus-1)
BuildRequires: pkgconfig(mlite)
Provides: %{g_location}
Obsoletes: contextkit-meego-location-geoclue <= %{meego_ver}
Provides: contextkit-meego-location-geoclue = %{meego_ver1}
%description %{p_gypsy}
%{summary}

%package %{p_skyhook}
Summary: Location ContextKit plugin using com.skyhookwireless.wps interface
License: Apache
Group: Applications/System
BuildRequires: pkgconfig(dbus-1)
BuildRequires: pkgconfig(mlite)
Provides: %{g_location}
Obsoletes: contextkit-meego-location-skyhook <= %{meego_ver}
Provides: contextkit-meego-location-skyhook = %{meego_ver1}
%description %{p_skyhook}
%{summary}

%package %{p_keyboard}
Summary:    Generic keyboard ContextKit plugin
Group:      Applications/System
BuildRequires: pkgconfig(udev)
Provides:   %{g_keyboard}
%description %{p_keyboard}
%{summary}

%package tests
Summary: Tests for ContextKit plugins
License: GPLv2
Group:      Applications/System
# depends on generic packages can be provided by different specific plugins
Requires: %{g_all}
%description tests
%{summary}

%prep
%setup -q -n %{name}-%{version}

%build
%cmake -DPLATFORM=NEMO_SHARED .
make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}
%make_install

%files %{p_bluez}
%defattr(-,root,root,-)
%{plugins_dir}/bluez.so
%{context_dir}/bluez.context

%post %{p_bluez}
update-contextkit-providers

%files %{p_upower}
%defattr(-,root,root,-)
%{plugins_dir}/upower.so
%{context_dir}/upower.context

%post %{p_upower}
update-contextkit-providers

%files %{p_session}
%defattr(-,root,root,-)
%{plugins_dir}/session.so
%{context_dir}/session.context

%post %{p_session}
update-contextkit-providers

%files %{p_profile}
%defattr(-,root,root,-)
%{plugins_dir}/profile.so
%{context_dir}/profile.context

%post %{p_profile}
update-contextkit-providers

%files %{p_connman}
%defattr(-,root,root,-)
%{plugins_dir}/connman.so
%{context_dir}/connman.context

%post %{p_connman}
update-contextkit-providers

%files %{p_cellular}
%defattr(-,root,root,-)
%{plugins_dir}/cellular.so
%{context_dir}/cellular.context

%post %{p_cellular}
update-contextkit-providers

%files %{p_ofono}
%defattr(-,root,root,-)
%{plugins_dir}/ofono.so
%{context_dir}/ofono.context

%post %{p_ofono}
update-contextkit-providers

%files %{p_music}
%defattr(-,root,root,-)
%{plugins_dir}/meego-music.so
%{context_dir}/meego-music.context

%post %{p_music}
update-contextkit-providers

%files %{p_mce}
%defattr(-,root,root,-)
%{plugins_dir}/mce.so
%{context_dir}/mce.context

%post %{p_mce}
update-contextkit-providers

%files %{p_gypsy}
%defattr(-,root,root,-)
%{plugins_dir}/location-gypsy.so
%{context_dir}/location-gypsy.context

%post %{p_gypsy}
update-contextkit-providers

%files %{p_skyhook}
%defattr(-,root,root,-)
%{plugins_dir}/location-skyhook.so
%{context_dir}/location-skyhook.context

%post %{p_skyhook}
update-contextkit-providers

%files %{p_keyboard}
%defattr(-,root,root,-)
%{plugins_dir}/keyboard-generic.so
%{context_dir}/keyboard-generic.context

%post %{p_keyboard}
update-contextkit-providers

%files tests
%defattr(-,root,root,-)
%{tests_dir}/contextkit-test-bluez
%{tests_dir}/contextkit-test-upower
%{tests_dir}/contextkit-test-session
%{tests_dir}/contextkit-test-profile
%{tests_dir}/contextkit-test-connman
%{tests_dir}/contextkit-test-cellular
%{tests_dir}/contextkit-test-ofono
%{tests_dir}/contextkit-test-meego-music
%{tests_dir}/contextkit-test-mce
%{tests_dir}/contextkit-test-location-gypsy
%{tests_dir}/contextkit-test-location-skyhook
%{tests_dir}/contextkit-test-keyboard-generic
