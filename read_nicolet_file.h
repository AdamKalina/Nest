#ifndef READ_NICOLET_FILE_H
#define READ_NICOLET_FILE_H

#include <iostream>
#include <fstream>
#include <iterator>
#include <vector>
#include <algorithm> // std::search
#include <string>
#include <cstring>
#include <map>
#include <cmath>
#include <sstream>
#include <iomanip>
#include <cctype>
#include <QDateTime>
#include <QDebug>
#include <QFile>
#include <QString>
#include <QDir>
#include "qpatient.h"

class read_nicolet_file
{
public:
    int LABELSIZE = 32;
    int TSLABELSIZE = 64;
    int UNITSIZE = 16;
    int ITEMNAMESIZE  = 64;

    unsigned long long Idx;
    unsigned long long sectionIdx;

    struct Tag{
        std::string tag;
        long int index;
        std::string IDStr;
    };

    struct Qi{
        unsigned long nrEntries;
        unsigned long misc1;
        unsigned long indexIdx;
        unsigned long misc3;
        unsigned long LQi;
        std::vector<unsigned long long> firstIdx;
    };

    struct Index{
        unsigned long long sectionIdx;
        unsigned long long offset;
        unsigned long long blockL;
        unsigned long long sectionL;
    };

    struct DynamicPacket{
        unsigned long long offset;
        std::string guid;
        std::string guidAsStr;
        double date;
        double datefrac;
        unsigned long long internalOffsetStart;
        unsigned long long packetSize;
        int data;
        std::string IDStr;
    };

    struct SegmentTimes{
        std::vector<QDateTime> segmentStartTimes;
        std::vector<double> durations;
    };

    struct NiconeHeader{
        std::vector<QString> patientInfo;
        std::vector<QString> studyInfo;
        SegmentTimes segmentTimes;
        double duration;
    };

    std::vector<Tag> sections; // Tags
    std::vector<Index> indices; // Index

    read_nicolet_file();
    QRecord get_qrecord_nicolet(QFileInfo fileInfo);
    NiconeHeader read_nicolet_header(std::ifstream &file);
    bool is_digits(std::string& str);
    std::string hexVectorTostring(std::vector<unsigned char>& vec);
    std::string formatGuid(const std::string& guidStr);
    QString readUint16toQString(int size,std::ifstream &file);
    Index getSectionIdx(std::string sectionName);
    std::vector<QString> readFileProperties(std::string sectionName,std::ifstream &file);
    SegmentTimes readSegmentStartTimes(std::ifstream &file);
};

#endif // READ_NICOLET_FILE_H
