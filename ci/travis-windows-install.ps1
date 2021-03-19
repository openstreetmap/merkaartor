tgdr

choco install -q msys2
taskkill -IM "gpg-agent.exe" -F

gdr

# We also install mingw-w64-x86_64-mesa, a per https://travis-ci.community/t/opengl32-dll-is-missing/10472/6 , it should provide the missing opengl32.dll for our tests to run.
c:\tools\msys64\usr\bin\bash.exe -l -c "pacman --noconfirm --needed -S msys/make msys/git mingw64/mingw-w64-x86_64-gcc mingw64/mingw-w64-x86_64-qt5 mingw64/mingw-w64-x86_64-gdal mingw64/mingw-w64-x86_64-proj mingw64/mingw-w64-x86_64-openjpeg2 mingw64/mingw-w64-x86_64-json-c mingw64/mingw-w64-x86_64-cmake mingw64/mingw-w64-x86_64-exiv2 mingw64/mingw-w64-x86_64-nsis mingw64/mingw-w64-x86_64-mesa"

# At some point, we had issues with disk space on travis windows. Here are a few tricks that helped:
## Uninstall some choco packages we are not using
#
# choco uninstall -y mingw llvm wsl
#
## Install Qt first, as it requires quite a bit of space
#c:\tools\msys64\usr\bin\bash.exe -l -c "pacman --noconfirm --needed -S mingw64/mingw-w64-x86_64-qt5"
#
#gdr
#
## Clean the cache
#c:\tools\msys64\usr\bin\bash.exe -l -c "pacman --noconfirm --needed -Scc"
#
#gdr
#
## Install Dev tools
#c:\tools\msys64\usr\bin\bash.exe -l -c "pacman --noconfirm --needed -S base-devel msys/git mingw64/mingw-w64-x86_64-gcc mingw64/mingw-w64-x86_64-gdal mingw64/mingw-w64-x86_64-proj mingw64/mingw-w64-x86_64-openjpeg2 mingw64/mingw-w64-x86_64-json-c mingw64/mingw-w64-x86_64-cmake mingw64/mingw-w64-x86_64-exiv2 mingw64/mingw-w64-x86_64-nsis"
#
## Clean the cache
#c:\tools\msys64\usr\bin\bash.exe -l -c "pacman --noconfirm --needed -Scc"
