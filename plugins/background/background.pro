TEMPLATE = subdirs

!symbian {
    SUBDIRS += \
        # MArbitraryRasterMapBackground \
        # MNavitBackground \
        MYahooTiledBackground \
        MYahooBackground \
        MWalkingPapersBackground

    greaterThan(QT_VER_MAJ, 3) : greaterThan(QT_VER_MIN, 5) {
        SUBDIRS += MCadastreFranceBackground
    }

    contains (GDAL, 1) {
        SUBDIRS += MGdalBackground
    }
    contains (SPATIALITE, 1) {
        SUBDIRS += MSpatialiteBackground
    }

}

