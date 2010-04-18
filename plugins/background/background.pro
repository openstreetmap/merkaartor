TEMPLATE = subdirs

!symbian {
    SUBDIRS += \
        # MArbitraryRasterMapBackground \
        MYahooTiledBackground \
        MYahooBackground \
        MWalkingPapersBackground

    contains (GDAL, 1) {
        SUBDIRS += MGdalBackground
    }

}

