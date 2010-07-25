TEMPLATE = subdirs

!symbian {
    SUBDIRS += \
        # MArbitraryRasterMapBackground \
        # MNavitBackground \
        MYahooTiledBackground \
        MYahooBackground \
        MWalkingPapersBackground

    contains (GDAL, 1) {
        SUBDIRS += MGdalBackground
    }

}

