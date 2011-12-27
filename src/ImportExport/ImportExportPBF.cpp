//
// C++ Implementation: ImportExportPBF
//
// Description:
//
//
// Author: cbro <cbro@semperpax.com>, (C) 2010
// Stefan de Konink <stefan at konink dot de>
// Roeland Douma <contact at rullzer dot com>
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include <QApplication>
#include <QMessageBox>
#include <QDateTime>

#include "ImportExportPBF.h"
#include "Global.h"

#include "zlib.h"
#include "bzlib.h"

#define NANO ( 1000.0 * 1000.0 * 1000.0 )
#define MAX_BLOCK_HEADER_SIZE ( 64 * 1024 )
#define MAX_BLOB_SIZE ( 32 * 1024 * 1024 )

ImportExportPBF::ImportExportPBF(Document* doc)
    : IImportExport(doc)
{
    GOOGLE_PROTOBUF_VERIFY_VERSION;
}

ImportExportPBF::~ImportExportPBF()
{
}

// export
bool ImportExportPBF::export_(const QList<Feature *>& /*featList*/)
{
    return false;
}

/***************************************************/
/*
Copyright 2010  Christian Vetter veaac.fdirct@gmail.com

This ripped code is part of MoNav.
*/

int convertNetworkByteOrder( char data[4] )
{
    return ( ( ( unsigned ) data[0] ) << 24 ) | ( ( ( unsigned ) data[1] ) << 16 ) | ( ( ( unsigned ) data[2] ) << 8 ) | ( unsigned ) data[3];
}

static void *SzAlloc( void *p, size_t size)
{
    p = p;
    return malloc( size );
}

static void SzFree( void *p, void *address)
{
    p = p;
    free( address );
}

void ImportExportPBF::loadGroup()
{
    const OSMPBF::PrimitiveGroup& group = m_primitiveBlock.primitivegroup( m_currentGroup );
    if ( group.nodes_size() != 0 ) {
        m_mode = ModeNode;
    } else if ( group.ways_size() != 0 ) {
        m_mode = ModeWay;
    } else if ( group.relations_size() != 0 ) {
        m_mode = ModeRelation;
    } else if ( group.has_dense() )  {
        m_mode = ModeDense;
        m_lastDenseID = 0;
        m_lastDenseTag = 0;
        m_lastDenseLatitude = 0;
        m_lastDenseLongitude = 0;
        m_lastDenseTimestamp = 0;
        m_lastDenseChangeset = 0;
        m_lastDenseUId = 0;
        m_lastDenseUserSid = 0;
        assert( group.dense().id_size() != 0 );
    } else
        assert( false );
}

void ImportExportPBF::loadBlock()
{
    m_loadBlock = false;
    m_currentGroup = 0;
    m_currentEntity = 0;
    int stringCount = m_primitiveBlock.stringtable().s_size();
    // precompute all strings that match a necessary tag
    m_nodeTagIDs.resize( m_primitiveBlock.stringtable().s_size() );
    for ( int i = 1; i < stringCount; i++ )
        m_nodeTagIDs[i] = m_nodeTags.value( m_primitiveBlock.stringtable().s( i ).data(), -1 );
    m_wayTagIDs.resize( m_primitiveBlock.stringtable().s_size() );
    for ( int i = 1; i < stringCount; i++ )
        m_wayTagIDs[i] = m_wayTags.value( m_primitiveBlock.stringtable().s( i ).data(), -1 );
    m_relationTagIDs.resize( m_primitiveBlock.stringtable().s_size() );
    for ( int i = 1; i < stringCount; i++ )
        m_relationTagIDs[i] = m_relationTags.value( m_primitiveBlock.stringtable().s( i ).data(), -1 );
}

bool ImportExportPBF::readNextBlock()
{
    if ( !readBlockHeader() )
        return false;

    if ( m_blockHeader.type() != "OSMData" ) {
        qCritical() << "invalid block type, found" << m_blockHeader.type().data() << "instead of OSMData";
        return false;
    }

    if ( !readBlob() )
        return false;

    if ( !m_primitiveBlock.ParseFromArray( m_buffer.data(), m_buffer.size() ) ) {
        qCritical() << "failed to parse PrimitiveBlock";
        return false;
    }
    return true;
}

bool ImportExportPBF::readBlockHeader()
{
    char sizeData[4];
    if ( m_file.read( sizeData, 4 * sizeof( char ) ) != 4 * sizeof( char ) )
        return false; // end of stream?

    int size = convertNetworkByteOrder( sizeData );
    if ( size > MAX_BLOCK_HEADER_SIZE || size < 0 ) {
        qCritical() << "BlockHeader size invalid:" << size;
        return false;
    }
    m_buffer.resize( size );
    int readBytes = m_file.read( m_buffer.data(), size );
    if ( readBytes != size ) {
        qCritical() << "failed to read BlockHeader";
        return false;
    }
    if ( !m_blockHeader.ParseFromArray( m_buffer.constData(), size ) ) {
        qCritical() << "failed to parse BlockHeader";
        return false;
    }
    return true;
}

bool ImportExportPBF::readBlob()
{
    int size = m_blockHeader.datasize();
    if ( size < 0 || size > MAX_BLOB_SIZE ) {
        qCritical() << "invalid Blob size:" << size;
        return false;
    }
    m_buffer.resize( size );
    int readBytes = m_file.read( m_buffer.data(), size );
    if ( readBytes != size ) {
        qCritical() << "failed to read Blob";
        return false;
    }
    if ( !m_blob.ParseFromArray( m_buffer.constData(), size ) ) {
        qCritical() << "failed to parse blob";
        return false;
    }

    if ( m_blob.has_raw() ) {
        const std::string& data = m_blob.raw();
        m_buffer.resize( data.size() );
        for ( unsigned i = 0; i < data.size(); i++ )
            m_buffer[i] = data[i];
    } else if ( m_blob.has_zlib_data() ) {
        if ( !unpackZlib() )
            return false;
//    } else if ( m_blob.has_bzip2_data() ) {
//        if ( !unpackBzip2() )
//            return false;
    } else if ( m_blob.has_lzma_data() ) {
        if ( !unpackLzma() )
            return false;
    } else {
        qCritical() << "Blob contains no data";
        return false;
    }

    return true;
}

bool ImportExportPBF::unpackZlib()
{
    m_buffer.resize( m_blob.raw_size() );
    z_stream compressedStream;
    compressedStream.next_in = ( unsigned char* ) m_blob.zlib_data().data();
    compressedStream.avail_in = m_blob.zlib_data().size();
    compressedStream.next_out = ( unsigned char* ) m_buffer.data();
    compressedStream.avail_out = m_blob.raw_size();
    compressedStream.zalloc = Z_NULL;
    compressedStream.zfree = Z_NULL;
    compressedStream.opaque = Z_NULL;
    int ret = inflateInit( &compressedStream );
    if ( ret != Z_OK ) {
        qCritical() << "failed to init zlib stream";
        return false;
    }
    ret = inflate( &compressedStream, Z_FINISH );
    if ( ret != Z_STREAM_END ) {
        qCritical() << "failed to inflate zlib stream";
        return false;
    }
    ret = inflateEnd( &compressedStream );
    if ( ret != Z_OK ) {
        qCritical() << "failed to deinit zlib stream";
        return false;
    }
    return true;
}

bool ImportExportPBF::unpackBzip2()
{
//    unsigned size = m_blob.raw_size();
//    m_buffer.resize( size );
//    m_bzip2Buffer.resize( m_blob.bzip2_data().size() );
//    for ( unsigned i = 0; i < m_blob.bzip2_data().size(); i++ )
//        m_bzip2Buffer[i] = m_blob.bzip2_data()[i];
//    int ret = BZ2_bzBuffToBuffDecompress( m_buffer.data(), &size, m_bzip2Buffer.data(), m_bzip2Buffer.size(), 0, 0 );
//    if ( ret != BZ_OK ) {
//        qCritical() << "failed to unpack bzip2 stream";
//        return false;
//    }
    return true;
}

bool ImportExportPBF::unpackLzma()
{
//    ISzAlloc alloc = { SzAlloc, SzFree };
//    ELzmaStatus status;
//    SizeT destinationLength = m_blob.raw_size();
//    SizeT sourceLength = m_blob.lzma_data().size() - LZMA_PROPS_SIZE + 8;
//    int ret = LzmaDecode(
//            ( unsigned char* ) m_buffer.data(),
//            &destinationLength,
//            ( const unsigned char* ) m_blob.lzma_data().data() + LZMA_PROPS_SIZE + 8,
//            &sourceLength,
//            ( const unsigned char* ) m_blob.lzma_data().data(),
//            LZMA_PROPS_SIZE + 8,
//            LZMA_FINISH_END,
//            &status,
//            &alloc );

//    if ( ret != SZ_OK )
//        return false;

    return true;
}

void ImportExportPBF::parseNode( Layer* aLayer )
{
    const OSMPBF::Node& inputNode = m_primitiveBlock.primitivegroup( m_currentGroup ).nodes( m_currentEntity );

    Node* N = STATIC_CAST_NODE(theDoc->getFeature(IFeature::FId(IFeature::Point, inputNode.id())));
    if (!N) {
        N = g_backend.allocNode(aLayer, Coord(
                ( ( qreal ) inputNode.lon() * m_primitiveBlock.granularity() + m_primitiveBlock.lon_offset() ) / NANO,
                ( ( qreal ) inputNode.lat() * m_primitiveBlock.granularity() + m_primitiveBlock.lat_offset() ) / NANO
                ));
        N->setId(IFeature::FId(IFeature::Point, inputNode.id()));
        aLayer->add(N);
    } else {
        N->setPosition(Coord(
                ( ( qreal ) inputNode.lon() * m_primitiveBlock.granularity() + m_primitiveBlock.lon_offset() ) / NANO,
                ( ( qreal ) inputNode.lat() * m_primitiveBlock.granularity() + m_primitiveBlock.lat_offset() ) / NANO
                ));
        N->setLastUpdated(Feature::OSMServer);
    }

#ifndef FRISIUS_BUILD
    if (inputNode.has_info()) {
        OSMPBF::Info info = inputNode.info();
        if (info.has_version())
            N->setVersionNumber(info.version());
        if (info.has_timestamp())
            N->setTime(QDateTime::fromTime_t(info.timestamp()));
        if (info.has_user_sid())
            N->setUser(m_primitiveBlock.stringtable().s(info.user_sid()).data());
    }
#endif

    for ( int tag = 0; tag < inputNode.keys_size(); tag++ ) {
        QString key = QString::fromUtf8( m_primitiveBlock.stringtable().s( inputNode.keys( tag ) ).data() );
        QString value = QString::fromUtf8( m_primitiveBlock.stringtable().s( inputNode.vals( tag ) ).data() );
        N->setTag(key, value);
    }

    m_currentEntity++;
    if ( m_currentEntity >= m_primitiveBlock.primitivegroup( m_currentGroup ).nodes_size() ) {
        m_currentEntity = 0;
        m_currentGroup++;
        if ( m_currentGroup >= m_primitiveBlock.primitivegroup_size() )
            m_loadBlock = true;
        else
            loadGroup();
    }
}

void ImportExportPBF::parseWay( Layer* aLayer )
{
    const OSMPBF::Way& inputWay = m_primitiveBlock.primitivegroup( m_currentGroup ).ways( m_currentEntity );

    Way* W = STATIC_CAST_WAY(theDoc->getFeature(IFeature::FId(IFeature::LineString, inputWay.id())));
    if (!W) {
        W = g_backend.allocWay(aLayer);
        W->setId(IFeature::FId(IFeature::LineString, inputWay.id()));
        aLayer->add(W);
    } else {
        W->setLastUpdated(Feature::OSMServer);
    }

#ifndef FRISIUS_BUILD
    if (inputWay.has_info()) {
        OSMPBF::Info info = inputWay.info();
        if (info.has_version())
            W->setVersionNumber(info.version());
        if (info.has_timestamp())
            W->setTime(QDateTime::fromTime_t(info.timestamp()));
        if (info.has_user_sid())
            W->setUser(m_primitiveBlock.stringtable().s(info.user_sid()).data());
    }
#endif

    for ( int tag = 0; tag < inputWay.keys_size(); tag++ ) {
        QString key = QString::fromUtf8( m_primitiveBlock.stringtable().s( inputWay.keys( tag ) ).data() );
        QString value = QString::fromUtf8( m_primitiveBlock.stringtable().s( inputWay.vals( tag ) ).data() );
        W->setTag(key, value);
    }

    long long lastRef = 0;
    for ( int i = 0; i < inputWay.refs_size(); i++ ) {
        lastRef += inputWay.refs( i );

        Node* N = STATIC_CAST_NODE(theDoc->getFeature(IFeature::FId(IFeature::Point, lastRef)));
        if (!N) {
            N = g_backend.allocNode(aLayer, Coord(0, 0));
            N->setId(IFeature::FId(IFeature::Point, lastRef));
            N->setLastUpdated(Feature::NotYetDownloaded);
            aLayer->add(N);
        }
        W->add(N);
    }

    m_currentEntity++;
    if ( m_currentEntity >= m_primitiveBlock.primitivegroup( m_currentGroup ).ways_size() ) {
        m_currentEntity = 0;
        m_currentGroup++;
        if ( m_currentGroup >= m_primitiveBlock.primitivegroup_size() )
            m_loadBlock = true;
        else
            loadGroup();
    }
}

void ImportExportPBF::parseRelation( Layer* aLayer )
{
    const OSMPBF::Relation& inputRelation = m_primitiveBlock.primitivegroup( m_currentGroup ).relations( m_currentEntity );

    Relation* R = STATIC_CAST_RELATION(theDoc->getFeature(IFeature::FId(IFeature::OsmRelation, inputRelation.id())));
    if (!R) {
        R = g_backend.allocRelation(aLayer);
        R->setId(IFeature::FId(IFeature::OsmRelation, inputRelation.id()));
        aLayer->add(R);
    } else {
        R->setLastUpdated(Feature::OSMServer);
    }

#ifndef FRISIUS_BUILD
    if (inputRelation.has_info()) {
        OSMPBF::Info info = inputRelation.info();
        if (info.has_version())
            R->setVersionNumber(info.version());
        if (info.has_timestamp())
            R->setTime(QDateTime::fromTime_t(info.timestamp()));
        if (info.has_user_sid())
            R->setUser(m_primitiveBlock.stringtable().s(info.user_sid()).data());
    }
#endif

    for ( int tag = 0; tag < inputRelation.keys_size(); tag++ ) {
        QString key = QString::fromUtf8( m_primitiveBlock.stringtable().s( inputRelation.keys( tag ) ).data() );
        QString value = QString::fromUtf8( m_primitiveBlock.stringtable().s( inputRelation.vals( tag ) ).data() );
        R->setTag(key, value);
    }

    long long lastRef = 0;
    for ( int i = 0; i < inputRelation.types_size(); i++ ) {
        lastRef += inputRelation.memids( i );
        QString role = QString::fromUtf8(m_primitiveBlock.stringtable().s( inputRelation.roles_sid( i ) ).data());

        switch (inputRelation.types( i )) {
        case OSMPBF::Relation::NODE: {
            Node* N = STATIC_CAST_NODE(theDoc->getFeature(IFeature::FId(IFeature::Point, lastRef)));
            if (!N) {
                N = g_backend.allocNode(aLayer, Coord(0, 0));
                N->setId(IFeature::FId(IFeature::Point, lastRef));
                N->setLastUpdated(Feature::NotYetDownloaded);
                aLayer->add(N);
            }
            R->add(role, N);
            break;
        }
        case OSMPBF::Relation::WAY: {
            Way* W = STATIC_CAST_WAY(theDoc->getFeature(IFeature::FId(IFeature::LineString, lastRef)));
            if (!W) {
                W = g_backend.allocWay(aLayer);
                W->setId(IFeature::FId(IFeature::LineString, lastRef));
                W->setLastUpdated(Feature::NotYetDownloaded);
                aLayer->add(W);
            }
            R->add(role, W);
            break;
        }
        case OSMPBF::Relation::RELATION: {
            Relation* Rl = STATIC_CAST_RELATION(theDoc->getFeature(IFeature::FId(IFeature::OsmRelation, lastRef)));
            if (!Rl) {
                Rl = g_backend.allocRelation(aLayer);
                Rl->setId(IFeature::FId(IFeature::OsmRelation, lastRef));
                Rl->setLastUpdated(Feature::NotYetDownloaded);
                aLayer->add(Rl);
            }
            R->add(role, Rl);
            break;
        }
        }
    }

    m_currentEntity++;
    if ( m_currentEntity >= m_primitiveBlock.primitivegroup( m_currentGroup ).relations_size() ) {
        m_currentEntity = 0;
        m_currentGroup++;
        if ( m_currentGroup >= m_primitiveBlock.primitivegroup_size() )
            m_loadBlock = true;
        else
            loadGroup();
    }
}

void ImportExportPBF::parseDense( Layer* aLayer )
{
    const OSMPBF::DenseNodes& dense = m_primitiveBlock.primitivegroup( m_currentGroup ).dense();

    m_lastDenseID += dense.id( m_currentEntity );
    m_lastDenseLatitude += dense.lat( m_currentEntity );
    m_lastDenseLongitude += dense.lon( m_currentEntity );

    Node* N = STATIC_CAST_NODE(theDoc->getFeature(IFeature::FId(IFeature::Point, m_lastDenseID)));
    if (!N) {
        N = g_backend.allocNode(aLayer, Coord(
                ( ( qreal ) m_lastDenseLongitude * m_primitiveBlock.granularity() + m_primitiveBlock.lon_offset() ) / NANO,
                ( ( qreal ) m_lastDenseLatitude * m_primitiveBlock.granularity() + m_primitiveBlock.lat_offset() ) / NANO
                ));
        N->setId(IFeature::FId(IFeature::Point, m_lastDenseID));
        aLayer->add(N);
    } else {
        N->setPosition(Coord(
                ( ( qreal ) m_lastDenseLongitude * m_primitiveBlock.granularity() + m_primitiveBlock.lon_offset() ) / NANO,
                ( ( qreal ) m_lastDenseLatitude * m_primitiveBlock.granularity() + m_primitiveBlock.lat_offset() ) / NANO
                ));
        N->setLastUpdated(Feature::OSMServer);
    }

    if (dense.has_denseinfo()) {
        m_lastDenseTimestamp += dense.denseinfo().timestamp(m_currentEntity);
        m_lastDenseChangeset += dense.denseinfo().changeset(m_currentEntity);
        m_lastDenseUId += dense.denseinfo().uid(m_currentEntity);
        m_lastDenseUserSid += dense.denseinfo().user_sid(m_currentEntity);

#ifndef FRISIUS_BUILD
        N->setVersionNumber(dense.denseinfo().version(m_currentEntity));
        N->setTime(m_lastDenseTimestamp);
        N->setUser(m_primitiveBlock.stringtable().s(m_lastDenseUserSid).data());
#endif
    }

    while ( true ){
        if ( m_lastDenseTag >= dense.keys_vals_size() )
            break;

        int tagValue = dense.keys_vals( m_lastDenseTag );
        if ( tagValue == 0 ) {
            m_lastDenseTag++;
            break;
        }

        QString key = QString::fromUtf8( m_primitiveBlock.stringtable().s( dense.keys_vals( m_lastDenseTag ) ).data() );
        QString value = QString::fromUtf8( m_primitiveBlock.stringtable().s( dense.keys_vals( m_lastDenseTag + 1 ) ).data() );
        N->setTag(key, value);

        m_lastDenseTag += 2;
    }

    m_currentEntity++;
    if ( m_currentEntity >= dense.id_size() ) {
        m_currentEntity = 0;
        m_currentGroup++;
        if ( m_currentGroup >= m_primitiveBlock.primitivegroup_size() )
            m_loadBlock = true;
        else
            loadGroup();
    }
}

/* End of MoNav rip */
/***************************************************/

// Specify the input as a QFile
bool ImportExportPBF::loadFile(QString filename)
{
    FileName = filename;
    ownDevice = true;

    m_file.setFileName( FileName );

    if ( !m_file.open(QIODevice::ReadOnly))
        return false;

    if ( !readBlockHeader() )
        return false;

    if ( m_blockHeader.type() != "OSMHeader" ) {
        qCritical() << "OSMHeader missing, found" << m_blockHeader.type().data() << "instead";
        return false;
    }

    if ( !readBlob() )
        return false;

    if ( !m_headerBlock.ParseFromArray( m_buffer.data(), m_buffer.size() ) ) {
        qCritical() << "failed to parse HeaderBlock";
        return false;
    }
    for ( int i = 0; i < m_headerBlock.required_features_size(); i++ ) {
        const std::string& feature = m_headerBlock.required_features( i );
        bool supported = false;
        if ( feature == "OsmSchema-V0.6" )
            supported = true;
        else if ( feature == "DenseNodes" )
            supported = true;

        if ( !supported ) {
            qCritical() << "required feature not supported:" << feature.data();
            return false;
        }
    }
    m_loadBlock = true;
    return true;
}

// import the  input
bool ImportExportPBF::import(Layer* aLayer)
{
    QProgressDialog progress(QApplication::tr("Importing..."), QApplication::tr("Cancel"), 0, 0);
    progress.setWindowModality(Qt::WindowModal);
    progress.setRange(0, m_file.size());
    progress.show();

    while (true && !progress.wasCanceled()) {
        if ( m_loadBlock ) {
            if ( !readNextBlock() )
                break;
            loadBlock();
            loadGroup();
        }

        switch ( m_mode ) {
        case ModeNode:
            parseNode( aLayer );
            break;
        case ModeWay:
            parseWay( aLayer );
            break;
        case ModeRelation:
            parseRelation( aLayer );
            break;
        case ModeDense:
            parseDense( aLayer );
            break;
        }
        progress.setValue(m_file.pos());
        qApp->processEvents();
//#ifndef NDEBUG
//        if (aLayer->size() > 1000000)
//            break;
//#endif
    }
    progress.reset();

    return true;
}
