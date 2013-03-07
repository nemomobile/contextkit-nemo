Name:       contextkit-plugins-n950-n9
Summary:    N950/N9 specific ContextKit providers for Nemo Mobile
Version:    0.7.39
Release:    1
Group:      Applications/System
License:    LGPLv2
URL:        https://github.com/nemomobile/contextkit-nemo
Source0:    %{name}-%{version}.tar.bz2
BuildRequires:  pkgconfig(QtCore)
BuildRequires:  pkgconfig(contextprovider-1.0)
BuildRequires:  pkgconfig(contextsubscriber-1.0) >= 0.5.25
BuildRequires:  cmake

%description
ContextKit providers specific to N9/N950

%define meego_ver 0.1.0
%define meego_ver1 0.1.0.1

%define p_power -n contextkit-plugin-power-bme
%define g_power contextkit-plugin-power
%define p_kbslider -n contextkit-plugin-kbslider
%define g_keyboard contextkit-plugin-keyboard
%define g_all %{g_power} %{g_keyboard}

%define plugins_dir %{_libdir}/contextkit/subscriber-plugins
%define context_dir %{_datadir}/contextkit/providers
%define tests_dir %{_bindir}


%package %{p_power}
Summary:    Battery ContextKit plugin using bme on N9/N950
Group:      Applications/System
Requires:   bme-rm-680-bin >= 0.9.95
Obsoletes: contextkit-meego-battery-upower <= %{meego_ver}
Provides: contextkit-meego-battery-upower = %{meego_ver1}
Obsoletes: contextkit-plugin-upower <= %{version}
Provides: contextkit-plugin-upower = %{version}
Provides:   %{g_power}
%description %{p_power}
%{summary}

%package %{p_kbslider}
Summary:    Slider keyboard ContextKit plugin
Group:      Applications/System
BuildRequires: pkgconfig(udev)
Provides:   %{g_keyboard}
%description %{p_kbslider}
%{summary}

%package tests
Summary: Tests for ContextKit plugins
Group:      Applications/System
Requires: %{g_all}
%description tests
%{summary}

%prep
%setup -q -n %{name}-%{version}

%build
%cmake -DPLATFORM=N9_50 .
make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}
%make_install

%files %{p_power}
%defattr(-,root,root,-)
%{plugins_dir}/power-bme.so
%{context_dir}/power-bme.context

%post %{p_power}
update-contextkit-providers

%files %{p_kbslider}
%defattr(-,root,root,-)
%{plugins_dir}/kbslider.so
%{context_dir}/kbslider.context

%post %{p_kbslider}
update-contextkit-providers

%files tests
%defattr(-,root,root,-)
%{tests_dir}/contextkit-test-power-bme
%{tests_dir}/contextkit-test-kbslider
