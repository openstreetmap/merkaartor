TEMPLATE = subdirs

!symbian {
    SUBDIRS += \
        # MArbitraryRasterMapBackground \
        MYahooTiledBackground \
        MYahooBackground

    contains (GDAL, 1) {
        SUBDIRS += MGdalBackground
    }

}

