ToWin = `cygpath --windows $1`
DOSWIXDIR := c:\\Program Files\\Windows Installer XML v3\\
LIGHT := "/c/Program Files/Windows Installer XML v3/bin/light.exe"
CANDLE := "/c/Program Files/Windows Installer XML v3/bin/candle.exe"


merkaartor.msi: merkaartor.wixobj Makefile banner.bmp dialog.bmp
	$(LIGHT) $(call ToWin,$<) \
		-dWixUILicenseRtf=..\\License.rtf \
		-dWixUIBannerBmp=banner.bmp \
		-dWixUIDialogBmp=dialog.bmp \
		-ext "$(DOSWIXDIR)\\bin\\WixUiExtension.dll" \
		-cultures:en-us \
		-out $(call ToWin,$@)


%.wixobj: %.wxs Makefile
	$(CANDLE) $(call ToWin,$<) -out $(call ToWin,$@)

clean:
	rm -rf merkaartor.msi merkaartor.wixobj
