Name:       capi-media-audio-io
Summary:    An Audio Input & Audio Output library in Tizen Native API
Version: 0.1.1
Release:    3
Group:      TO_BE/FILLED_IN
License:    TO BE FILLED IN
Source0:    %{name}-%{version}.tar.gz
BuildRequires:  cmake
BuildRequires:  pkgconfig(dlog)
BuildRequires:  pkgconfig(mm-sound)
BuildRequires:  pkgconfig(capi-media-sound-manager)
BuildRequires:  pkgconfig(capi-base-common)
Requires(post): /sbin/ldconfig  
Requires(postun): /sbin/ldconfig

%description
An Audio Input & Audio Output library in Tizen Native API

%package devel
Summary:  An Audio Input & Audio Output library in Tizen Native API (Development)
Group:    TO_BE/FILLED_IN
Requires: %{name} = %{version}-%{release}

%description devel
An Audio Input & Audio Output library in Tizen Native API (DEV)

%prep
%setup -q


%build
MAJORVER=`echo %{version} | awk 'BEGIN {FS="."}{print $1}'`
cmake . -DCMAKE_INSTALL_PREFIX=/usr -DFULLVER=%{version} -DMAJORVER=${MAJORVER}


make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}
%make_install

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig


%files
%{_libdir}/libcapi-media-audio-io.so.*
%manifest capi-media-audio-io.manifest

%files devel
%{_includedir}/media/audio_io.h
%{_libdir}/pkgconfig/*.pc
%{_libdir}/libcapi-media-audio-io.so


