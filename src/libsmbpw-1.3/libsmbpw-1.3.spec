Summary:   smbpasswd entry manipulation library
Name:      libsmbpw
Version:   1.3
Release:   1
Copyright: GPL
Packager:  Andy Phillips <atp@pergamentum.com>		
Group:     Development/Libraries
Source:    %{name}-%{version}.tar.gz
Buildroot: /tmp/%{name}-%{version}-root

%description 
smbpasswd entry manipulation library
smbcrypt password encryption program.

%prep
%setup -q

%build
make

%install
mkdir -p $RPM_BUILD_ROOT/usr/lib
mkdir -p $RPM_BUILD_ROOT/usr/include/libsmbpw

cp -f libsmbpw.%{version}.so $RPM_BUILD_ROOT/usr/lib
cp *.h $RPM_BUILD_ROOT/usr/include/libsmbpw/
cp -f smbcrypt $RPM_BUILD_ROOT/usr/bin

cd $RPM_BUILD_ROOT/usr/lib
ln -sf libsmbpw.%{version}.so libsmbpw.so


%clean 
rm -fr $RPM_BUILD_ROOT

%post
/sbin/ldconfig

%postun
/sbin/ldconfig

%files 
%defattr(-,root,root)

/usr/lib
/usr/include/libsmbpw

