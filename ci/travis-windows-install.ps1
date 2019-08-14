
powershell Set-MpPreference -DisableRealTimeMonitoring 1
powershell Set-MpPreference -DisableBehaviorMonitoring 1

choco install -q msys2
taskkill -IM "gpg-agent.exe" -F
c:\tools\msys64\usr\bin\bash.exe -l -c "pacman --noconfirm --needed -S base-devel msys/git mingw64/mingw-w64-x86_64-gcc mingw64/mingw-w64-x86_64-qt5 mingw64/mingw-w64-x86_64-gdal mingw64/mingw-w64-x86_64-proj mingw64/mingw-w64-x86_64-openjpeg2 mingw64/mingw-w64-x86_64-json-c mingw64/mingw-w64-x86_64-cmake mingw64/mingw-w64-x86_64-exiv2 mingw64/mingw-w64-x86_64-nsis"

