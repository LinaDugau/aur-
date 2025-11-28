Name:       ru.auroraos.aurcad
Summary:    AurСад - Детектор свежести яблок с AI
Version:    1.0.0
Release:    1
License:    MIT
URL:        https://github.com/LinaDugau/aurcad
Source0:    %{name}-%{version}.tar.bz2

Requires:   sailfishsilica-qt5 >= 0.10.9
Requires:   qt5-qtmultimedia

BuildRequires:  pkgconfig(auroraapp)
BuildRequires:  pkgconfig(Qt5Core)
BuildRequires:  pkgconfig(Qt5Qml)
BuildRequires:  pkgconfig(Qt5Quick)
BuildRequires:  pkgconfig(Qt5Multimedia)
BuildRequires:  desktop-file-utils

%description
AurСад — приложение для определения свежести яблок с помощью AI.

%prep
%autosetup

%build
%qmake5
%make_build

%install
%make_install

# ---------------------------
# ФАЙЛЫ ПАКЕТА
# ---------------------------
%files
%defattr(-,root,root,-)

# бинарник
%{_bindir}/%{name}

# desktop-файл
%{_datadir}/applications/%{name}.desktop

# QML и переводы
%{_datadir}/%{name}/qml
%{_datadir}/%{name}/qml/*
%{_datadir}/%{name}/translations
%{_datadir}/%{name}/translations/*

# Иконки
%{_datadir}/icons/hicolor/86x86/apps/%{name}.png
%{_datadir}/icons/hicolor/108x108/apps/%{name}.png
%{_datadir}/icons/hicolor/128x128/apps/%{name}.png
%{_datadir}/icons/hicolor/172x172/apps/%{name}.png

# Датасет для обучения модели
%{_datadir}/%{name}/dataset
%{_datadir}/%{name}/dataset/omsk
%{_datadir}/%{name}/dataset/omsk/*

%changelog
* Thu Feb 27 2025 AuroraOS AI Hackathon Team
- Fixed packaging, added icons, QML and translations
