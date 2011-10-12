//
// C++ Interface: ImportExportPBF
//
// Description:
//
//
// Author: cbro <cbro@semperpax.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef ImportExportPBF_H
#define ImportExportPBF_H

#include "IImportExport.h"

#include "fileformat.pb.h"
#include "osmformat.pb.h"

class QDomDocument;
/**
    @author cbro <cbro@semperpax.com>
*/
class ImportExportPBF : public IImportExport
{
protected:
    enum Mode {
        ModeNode, ModeWay, ModeRelation, ModeDense
    };

public:
    ImportExportPBF(Document* doc);

    ~ImportExportPBF();

    // Specify the input
    virtual bool loadFile(QString filename);
    // import the  input
    virtual bool import(Layer* aLayer);

    //export
    virtual bool export_(const QList<Feature *>& featList);

protected:
    OSMPBF::BlobHeader m_blockHeader;
    OSMPBF::Blob m_blob;

    OSMPBF::HeaderBlock m_headerBlock;
    OSMPBF::PrimitiveBlock m_primitiveBlock;

    int m_currentGroup;
    int m_currentEntity;
    bool m_loadBlock;

    Mode m_mode;

    QHash< QString, int > m_nodeTags;
    QHash< QString, int > m_wayTags;
    QHash< QString, int > m_relationTags;

    std::vector< int > m_nodeTagIDs;
    std::vector< int > m_wayTagIDs;
    std::vector< int > m_relationTagIDs;

    long long m_lastDenseID;
    long long m_lastDenseLatitude;
    long long m_lastDenseLongitude;
    long long m_lastDenseTimestamp;
    long long m_lastDenseChangeset;
    long long m_lastDenseUId;
    long long m_lastDenseUserSid;
    int m_lastDenseTag;

    QFile m_file;
    QByteArray m_buffer;
    QByteArray m_bzip2Buffer;

protected:
    void loadGroup();
    void loadBlock();
    bool readNextBlock();
    bool readBlockHeader();
    bool readBlob();
    bool unpackZlib();
    bool unpackBzip2();
    bool unpackLzma();

    void parseNode( Layer* aLayer );
    void parseWay( Layer* aLayer );
    void parseRelation( Layer* aLayer );
    void parseDense( Layer* aLayer );
};

#endif
