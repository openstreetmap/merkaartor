TEMPLATE = subdirs

!symbian {
	SUBDIRS += \
		MYahooBackground \
		MYahooTiledBackground

	contains (GDAL, 1) {
		SUBDIRS += MGdalBackground
	}

}

