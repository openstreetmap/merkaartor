TEMPLATE = subdirs

!symbian {
    SUBDIRS += \
        # MArbitraryRasterMapBackground \
        MYahooBackground \
        MYahooTiledBackground

	contains (GDAL, 1) {
		SUBDIRS += MGdalBackground
	}

}

