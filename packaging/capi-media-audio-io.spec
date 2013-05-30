Name:       capi-media-audio-io
Summary:    An Audio Input & Audio Output library in Tizen Native API
Version: 0.2.0
Release:    0
Group:      Multimedia/API
License:    Apache-2.0
Source0:    %{name}-%{version}.tar.gz
BuildRequires:  cmake
BuildRequires:  pkgconfig(dlog)
BuildRequires:  pkgconfig(mm-sound)
BuildRequires:  pkgconfig(capi-media-sound-manager)
BuildRequires:  pkgconfig(capi-base-common)

%description
An Audio Input & Audio Output library in Tizen Native API

%package devel
Summary:  An Audio Input & Audio Output library in Tizen Native API (Development)
Group:    Multimedia/Development
Requires: %{name} = %{version}-%{release}

%description devel
An Audio Input & Audio Output library in Tizen Native API (DEV)

%prep
%setup -q


%build
MAJORVER=`echo %{version} | awk 'BEGIN {FS="."}{print $1}'`
%cmake . -DFULLVER=%{version} -DMAJORVER=${MAJORVER}

make %{?jobs:-j%jobs}

%install
%make_install

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig


%files
%license LICENSE.APLv2
%{_libdir}/libcapi-media-audio-io.so.*
%manifest capi-media-audio-io.manifest

%files devel
%{_includedir}/media/audio_io.h
%{_libdir}/pkgconfig/*.pc
%{_libdir}/libcapi-media-audio-io.so


