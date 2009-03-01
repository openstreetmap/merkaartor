#ToWin = `cygpath --windows $1`
ToWin = $1
DOSWIXDIR := c:\\Program Files\\Windows Installer XML v3\\
LIGHT := "c:/Program Files/Windows Installer XML v3/bin/light.exe"
CANDLE := "c:/Program Files/Windows Installer XML v3/bin/candle.exe"
LRELEASE := "lrelease.exe"
VERSION=0.13
REVISION=-rc4

all: release debug 

release: merkaartor-$(VERSION)$(REVISION).msi

debug: merkaartor-$(VERSION)$(REVISION)-debug.msi  

merkaartor-$(VERSION)$(REVISION).msi: merkaartor.wixobj Makefile banner.bmp dialog.bmp merkaartor.qm
	$(LIGHT) $(call ToWin,$<) \
		-dWixUILicenseRtf=..\\License.rtf \
		-dWixUIBannerBmp=banner.bmp \
		-dWixUIDialogBmp=dialog.bmp \
		-ext "$(DOSWIXDIR)\\bin\\WixUiExtension.dll" \
		-cultures:en-us \
		-out $(call ToWin,$@)

merkaartor-$(VERSION)$(REVISION)-debug.msi: merkaartor_dbg.wixobj Makefile banner.bmp dialog.bmp merkaartor.qm
	$(LIGHT) $(call ToWin,$<) \
		-dWixUILicenseRtf=..\\License.rtf \
		-dWixUIBannerBmp=banner.bmp \
		-dWixUIDialogBmp=dialog.bmp \
		-ext "$(DOSWIXDIR)\\bin\\WixUiExtension.dll" \
		-cultures:en-us \
		-out $(call ToWin,$@)


merkaartor.wixobj: merkaartor.wxs defines.wxi qt.wxi qttrans.wxi Makefile
	$(CANDLE) $(call ToWin,$<) \
		-dVERSION=$(VERSION) \
		-dREVISION=$(REVISION) \
		-out $(call ToWin,$@)

merkaartor_dbg.wixobj: merkaartor_dbg.wxs defines_dbg.wxi qt_dbg.wxi qttrans.wxi Makefile
	$(CANDLE) $(call ToWin,$<) \
		-dVERSION=$(VERSION) \
		-dREVISION=$(REVISION) \
		-out $(call ToWin,$@)

merkaartor.qm: 
	$(LRELEASE) ..\\merkaartor.pro

clean:
	rm -rf *.msi *.wixobj *.wixpdb
