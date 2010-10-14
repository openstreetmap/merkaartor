TEMPLATE = subdirs

!symbian {
    SUBDIRS += \
        # MArbitraryRasterMapBackground \
        # MNavitBackground \
        # MCadastreFranceBackground \
        MYahooTiledBackground \
        MYahooBackground \
        MWalkingPapersBackground

    contains (GDAL, 1) {
        SUBDIRS += MGdalBackground
    }

}

