#ToWin = `cygpath --windows $1`
ToWin = $1
DOSWIXDIR := c:\\Program Files\\Windows Installer XML v3\\
LIGHT := "c:/Program Files/Windows Installer XML v3/bin/light.exe"
CANDLE := "c:/Program Files/Windows Installer XML v3/bin/candle.exe"
LRELEASE := "C:/Qt/4.6.2-win/bin/lrelease.exe"
#VERSION=0.16
#REVISION=-svn

all: release debug svn 

release: merkaartor-$(VERSION)$(REVISION).msi

svn: merkaartor-$(VERSION)-svn$(SVNREV).msi

debug: merkaartor-$(VERSION)-svn$(SVNREV)-debug.msi  
qt-debug: merkaartor-qt-debug-$(VERSION).msi  

merkaartor-$(VERSION)-svn$(SVNREV).msi: merkaartor_svn.wixobj Makefile banner.bmp dialog.bmp merkaartor.qm
	$(LIGHT) $(call ToWin,$<) \
		-dWixUILicenseRtf=..\\License.rtf \
		-dWixUIBannerBmp=banner.bmp \
		-dWixUIDialogBmp=dialog.bmp \
		-ext "$(DOSWIXDIR)\\bin\\WixUiExtension.dll" \
		-cultures:en-us \
		-out "$(call ToWin,$@)"

merkaartor-$(VERSION)$(REVISION).msi: merkaartor.wixobj Makefile banner.bmp dialog.bmp merkaartor.qm
	$(LIGHT) $(call ToWin,$<) \
		-dWixUILicenseRtf=..\\License.rtf \
		-dWixUIBannerBmp=banner.bmp \
		-dWixUIDialogBmp=dialog.bmp \
		-ext "$(DOSWIXDIR)\\bin\\WixUiExtension.dll" \
		-cultures:en-us \
		-out "$(call ToWin,$@)"

merkaartor-$(VERSION)-svn$(SVNREV)-debug.msi: merkaartor_dbg.wixobj Makefile banner.bmp dialog.bmp merkaartor.qm
	$(LIGHT) $(call ToWin,$<) \
		-dWixUILicenseRtf=..\\License.rtf \
		-dWixUIBannerBmp=banner.bmp \
		-dWixUIDialogBmp=dialog.bmp \
		-ext "$(DOSWIXDIR)\\bin\\WixUiExtension.dll" \
		-cultures:en-us \
		-out "$(call ToWin,$@)"

merkaartor-qt-debug-$(VERSION).msi: qt_dbg.wixobj Makefile banner.bmp dialog.bmp
	$(LIGHT) $(call ToWin,$<) \
		-dWixUILicenseRtf=..\\License.rtf \
		-dWixUIBannerBmp=banner.bmp \
		-dWixUIDialogBmp=dialog.bmp \
		-ext "$(DOSWIXDIR)\\bin\\WixUiExtension.dll" \
		-cultures:en-us \
		-out "$(call ToWin,$@)"


merkaartor.wixobj: merkaartor.wxs defines.wxi qt.wxi qttrans.wxi Makefile
	$(CANDLE) $(call ToWin,$<) \
		-dVERSION=$(VERSION) \
		-dREVISION=$(REVISION) \
		-out $(call ToWin,$@)

merkaartor_svn.wixobj: merkaartor_svn.wxs defines_svn.wxi qt.wxi qttrans.wxi Makefile
	$(CANDLE) $(call ToWin,$<) \
		-dVERSION=$(VERSION) \
		-dREVISION=$(REVISION) \
		-out $(call ToWin,$@)

merkaartor_dbg.wixobj: merkaartor_dbg.wxs defines_dbg.wxi qt_dbg.wxi qttrans.wxi Makefile
	$(CANDLE) $(call ToWin,$<) \
		-dVERSION=$(VERSION) \
		-dREVISION=$(REVISION) \
		-out $(call ToWin,$@)

qt_dbg.wixobj: qt_dbg.wxs defines_qt_dbg.wxi qt_dbg.wxi qttrans.wxi Makefile
	$(CANDLE) $(call ToWin,$<) \
		-dVERSION=$(VERSION) \
		-dREVISION=$(REVISION) \
		-out $(call ToWin,$@)

merkaartor.qm: 
	$(LRELEASE) ../src/src.pro

clean:
	rm -rf *.msi *.wixobj *.wixpdb
