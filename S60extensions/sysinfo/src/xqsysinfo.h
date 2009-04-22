#ifndef XQSYSINFO_H
#define XQSYSINFO_H

// INCLUDES
#include <QObject>

// FORWARD DECLARATIONS
class XQSysInfoPrivate;

// CLASS DECLARATION
class XQSysInfo : public QObject
{
    Q_OBJECT
    
public:

    enum Error {
        NoError = 0,
        OutOfMemoryError,
        IncorrectDriveError,
        DriveNotFoundError,
        UnknownError = -1
    };
    
    enum Drive {
        DriveA,   DriveB,   DriveC,   DriveD,   DriveE,
        DriveF,   DriveG,   DriveH,   DriveI,   DriveJ,
        DriveK,   DriveL,   DriveM,   DriveN,   DriveO, 
        DriveP,   DriveQ,   DriveR,   DriveS,   DriveT,
        DriveU,   DriveV,   DriveW,   DriveX,   DriveY,
        DriveZ
    };
    
    enum Language {
        LangEnglish = 1,
        LangFrench = 2,
        LangGerman = 3,
        LangSpanish = 4,
        LangItalian = 5,
        LangSwedish = 6,
        LangDanish = 7,
        LangNorwegian = 8,
        LangFinnish = 9,
        LangAmerican = 10,
        LangSwissFrench = 11,
        LangSwissGerman = 12,
        LangPortuguese = 13,
        LangTurkish = 14,
        LangIcelandic = 15,
        LangRussian = 16,
        LangHungarian = 17,
        LangDutch = 18,
        LangBelgianFlemish = 19,
        LangAustralian = 20,
        LangBelgianFrench = 21,
        LangAustrian = 22,
        LangNewZealand = 23,
        LangInternationalFrench = 24,
        LangCzech = 25,
        LangSlovak = 26,
        LangPolish = 27,
        LangSlovenian = 28,
        LangTaiwanChinese = 29,
        LangHongKongChinese = 30,
        LangPrcChinese = 31,
        LangJapanese = 32,
        LangThai = 33,
        LangAfrikaans = 34,
        LangAlbanian = 35,
        LangAmharic = 36,
        LangArabic = 37,
        LangArmenian = 38,
        LangTagalog = 39,
        LangBelarussian = 40,
        LangBengali = 41,
        LangBulgarian = 42,
        LangBurmese = 43,
        LangCatalan = 44,
        LangCroatian = 45,
        LangCanadianEnglish = 46,
        LangInternationalEnglish = 47,
        LangSouthAfricanEnglish = 48,
        LangEstonian = 49,
        LangFarsi = 50,
        LangCanadianFrench = 51,
        LangScotsGaelic = 52,
        LangGeorgian = 53,
        LangGreek = 54,
        LangCyprusGreek = 55,
        LangGujarati = 56,
        LangHebrew = 57,
        LangHindi = 58,
        LangIndonesian = 59,
        LangIrish = 60,
        LangSwissItalian = 61,
        LangKannada = 62,
        LangKazakh = 63,
        LangKhmer = 64,
        LangKorean = 65,
        LangLao = 66,
        LangLatvian = 67,
        LangLithuanian = 68,
        LangMacedonian = 69,
        LangMalay = 70,
        LangMalayalam = 71,
        LangMarathi = 72,
        LangMoldavian = 73,
        LangMongolian = 74,
        LangNorwegianNynorsk = 75,
        LangBrazilianPortuguese = 76,
        LangPunjabi = 77,
        LangRomanian = 78,
        LangSerbian = 79,
        LangSinhalese = 80,
        LangSomali = 81,
        LangInternationalSpanish = 82,
        LangLatinAmericanSpanish = 83,
        LangSwahili = 84,
        LangFinlandSwedish = 85,
        LangReserved1 = 86,
        LangTamil = 87,
        LangTelugu = 88,
        LangTibetan = 89,
        LangTigrinya = 90,
        LangCyprusTurkish = 91,
        LangTurkmen = 92,
        LangUkrainian = 93,
        LangUrdu = 94,
        LangReserved2 = 95,
        LangVietnamese = 96,
        LangWelsh = 97,
        LangZulu = 98,
        LangOther = 99,
        LangManufacturerEnglish = 100,
        LangSouthSotho = 101,
        LangBasque = 102,
        LangGalician = 103,
        LangJavanese = 104,
        LangMaithili = 105,
        LangAzerbaijani_Latin = 106,
        LangAzerbaijani_Cyrillic = 107,
        LangOriya = 108,
        LangBhojpuri = 109,
        LangSundanese = 110,
        LangKurdish_Latin = 111,
        LangKurdish_Arabic = 112,
        LangPashto = 113,
        LangHausa = 114,
        LangOromo = 115,
        LangUzbek_Latin = 116,
        LangUzbek_Cyrillic = 117,
        LangSindhi_Arabic = 118,
        LangSindhi_Devanagari = 119,
        LangYoruba = 120,
        LangCebuano = 121,
        LangIgbo = 122,
        LangMalagasy = 123,
        LangNepali = 124,
        LangAssamese = 125,
        LangShona = 126,
        LangZhuang = 127,
        LangMadurese = 128,
        LangEnglish_Apac=129,
        LangEnglish_Taiwan=157,
        LangEnglish_HongKong=158,
        LangEnglish_Prc=159,
        LangEnglish_Japan=160,
        LangEnglish_Thailand=161,
        LangFulfulde = 162,
        LangTamazight = 163,
        LangBolivianQuechua = 164,
        LangPeruQuechua = 165,
        LangEcuadorQuechua = 166,
        LangTajik_Cyrillic = 167,
        LangTajik_PersoArabic = 168,
        LangNyanja = 169,
        LangHaitianCreole = 170,
        LangLombard = 171,
        LangKoongo = 172,
        LangAkan = 173,
        LangHmong = 174,
        LangYi = 175,
        LangTshiluba = 176,
        LangIlocano = 177,
        LangUyghur = 178,
        LangNeapolitan = 179,
        LangRwanda = 180,
        LangXhosa = 181,
        LangBalochi = 182,
        LangMinangkabau = 184,
        LangMakhuwa = 185,
        LangSantali = 186,
        LangGikuyu = 187,
        LangMoore = 188,
        LangGuarani = 189,
        LangRundi = 190,
        LangRomani_Latin = 191,
        LangRomani_Cyrillic = 192,
        LangTswana = 193,
        LangKanuri = 194,
        LangKashmiri_Devanagari = 195,
        LangKashmiri_PersoArabic = 196,
        LangUmbundu = 197,
        LangKonkani = 198,
        LangBalinese = 199,
        LangNorthernSotho = 200,
        LangWolof = 201,
        LangBemba = 202,
        LangTsonga = 203,
        LangYiddish = 204,
        LangKirghiz = 205,
        LangGanda = 206,
        LangSoga = 207,
        LangMbundu = 208,
        LangBambara = 209,
        LangCentralAymara = 210,
        LangZarma = 211,
        LangLingala = 212,
        LangBashkir = 213,
        LangChuvash = 214,
        LangSwati = 215,
        LangTatar = 216,
        LangSouthernNdebele = 217,
        LangSardinian = 218,
        LangScots = 219,
        LangMeitei = 220,
        LangWalloon = 221,
        LangKabardian = 222,
        LangMazanderani = 223,
        LangGilaki = 224,
        LangShan = 225,
        LangLuyia = 226,
        LanguageLuo = 227,
        LangSukuma = 228,
        LangAceh = 229,
        LangMalay_Apac=326,        
    };
    
    XQSysInfo(QObject *parent = 0);
    ~XQSysInfo();
    
    XQSysInfo::Language currentLanguage() const;
    QString imei() const;
    QString model() const;
    QString manufacturer() const;
    QString softwareVersion() const;

    uint batteryLevel() const;
    QString imsi() const;
    int signalStrength() const;
    qlonglong diskSpace(XQSysInfo::Drive drive) const;
    bool isDiskSpaceCritical(XQSysInfo::Drive drive) const;
    bool isNetwork() const;
    int memory() const;
    QString browserVersion() const;
    
    static bool isSupported(int featureId);

    XQSysInfo::Error error() const;

Q_SIGNALS:
    void networkSignalChanged(ulong signalStrength);
    void batteryLevelChanged(uint batteryLevel);
    
private:
    friend class XQSysInfoPrivate;
    XQSysInfoPrivate *d;
};

#endif /*XQSYSINFO_H*/

// End of file

