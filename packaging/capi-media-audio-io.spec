Name:           capi-media-audio-io
Summary:        An Audio Input & Audio Output library in Tizen Native API
Version:        0.3.30
Release:        0
Group:          Multimedia/API
License:        Apache-2.0
Source0:        %{name}-%{version}.tar.gz
Source1001:     capi-media-audio-io.manifest
BuildRequires:  cmake
BuildRequires:  pkgconfig(dlog)
BuildRequires:  pkgconfig(mm-common)
BuildRequires:  pkgconfig(mm-session)
BuildRequires:  pkgconfig(mm-sound)
BuildRequires:  pkgconfig(vconf)
BuildRequires:  pkgconfig(capi-media-sound-manager)
BuildRequires:  pkgconfig(capi-base-common)
BuildRequires:  pkgconfig(libpulse)

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
cp %{SOURCE1001} .

%build
MAJORVER=`echo %{version} | awk 'BEGIN {FS="."}{print $1}'`
%cmake . -DFULLVER=%{version} -DMAJORVER=${MAJORVER}

make %{?jobs:-j%jobs}

%install
%make_install

%post
/sbin/ldconfig

%postun -p /sbin/ldconfig

%files
%manifest %{name}.manifest
%license LICENSE
%{_libdir}/libcapi-media-audio-io.so.*

%files devel
%manifest %{name}.manifest
%{_includedir}/media/audio_io.h
%{_libdir}/pkgconfig/*.pc
%{_libdir}/libcapi-media-audio-io.so
#%{_prefix}/bin/audio_io_test

