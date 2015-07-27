# see http://merkaartor.be/wiki/merkaartor/Compiling

REVISION = $$system(git describe --tags 2> /dev/null)
VERSION = $$system(git describe --tags 2> /dev/null | sed "'s/-g.*//;s/-/./g'")
isEmpty( REVISION ) {
	REVISION = $$system(head -n 1 ../CHANGELOG | sed "'s/^v//'")
	VERSION = $$REVISION
}

ARCH=""
BITS=""
win32 {
	ARCH="-$$QMAKE_HOST.arch"
	win32-g++:contains(QMAKE_HOST.arch, x86_64):{
		BITS=64
		REVISION=$${REVISION}-64bit
	} else {
		BITS=32
		REVISION=$${REVISION}-32bit
	}
}

linux-g++:contains(QT_ARCH, x86_64):{
    REVISION=$${REVISION}-64bit
}

linux-g++:contains(QT_ARCH, i386):{
    REVISION=$${REVISION}-32bit
}



win32 {
	system(echo "!define VER $${REVISION}" > ../windows/version.nch )
	system(echo "!define BITS $${BITS}" >> ../windows/version.nch )
}

QMAKE_CXXFLAGS_WARN_ON += -Wno-reorder
