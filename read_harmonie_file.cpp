#include "read_harmonie_file.h"

void header_harmonie_read_starting_time2(std::ifstream &file){

    std::string result = "";
    double f;

    // find the position of pattern in binary file
    // inspired by this: https://stackoverflow.com/questions/58670046/how-can-i-check-if-string-exists-in-binary-file
    // does not work on XP
    // TO DO - read data from content, so it can be passed just once
    std::string pattern = "CFileHeader";

    file.seekg(0, std::ios::end);
    auto size = file.tellg();
    file.seekg(0);
    std::vector<char> content(size, '\0');
    file.read(content.data(), size);
    auto res = std::search(content.begin(), content.end(), pattern.begin(), pattern.end());

    //std::cout << res - content.begin() << std::endl;

    file.seekg(res-content.begin());

    std::cout << pattern << " found at position: "  << file.tellg() << std::endl;

    // there is one more zero after the pattern that needs to be skipped
    while (file.get() != 0) {
    }

    // channel info - skip it
    while (file.get() != 0) {
    }

    // this read the 64bit float that gives number of seconds between start of the file and 1.1. 0000 00:00
    file.read(reinterpret_cast<char *>(&f), sizeof(f));

    std::cout.precision(17);
    std::cout << "Date: " << std::fixed << f << std::endl;


    //    // then it reads Harmonie 5.4  EN-0ÿÿ - I guess I dont need that...
    //    while ((ch = file.get()) != 0) {
    //        result += ch;
    //    }

    //    std::cout << result << std::endl;

}

double header_harmonie_read_starting_time(std::ifstream &file){

    double f;
    char currentChar;
    std::string pattern = "CFileHeader";
    std::string buffer;

    // find the position of pattern in binary file - using GPT code

    while (file.get(currentChar)) {
        buffer += currentChar;
        if (buffer.size() > pattern.size())
            buffer.erase(0, 1);

        if (buffer == pattern) {
            //std::cout << pattern << " found at position: "  << file.tellg() << std::endl;
            break;
        }
    }

    // there is one more zero after the pattern that needs to be skipped
    while (file.get() != 0) {
    }

    // channel info - skip it
    while (file.get() != 0) {
    }

    // this read the 64bit float that giver number of seconds between start of the file and 1.1. 0000 00:00
    file.read(reinterpret_cast<char *>(&f), sizeof(f));

    //std::cout.precision(17);
    //std::cout << "Date: " << std::fixed << f << std::endl;

    return f;
    //    // then it reads Harmonie 5.4  EN-0ÿÿ - I guess I dont need that...
    //    while ((ch = file.get()) != 0) {
    //        result += ch;
    //    }
}


std::vector<std::string> header_harmonie_read_patient_info(std::ifstream &file){
    //using GPT code

    std::string result = "";
    char ch;
    int no;
    std::vector<std::string> results;
    char currentChar;
    std::string pattern = "CPatientInfo";
    std::string buffer;

    // find the position of pattern in binary file

    while (file.get(currentChar)) {
        buffer += currentChar;
        if (buffer.size() > pattern.size())
            buffer.erase(0, 1);

        if (buffer == pattern) {
            //std::cout << pattern << " found at position: "  << file.tellg() << std::endl;
            break;
        }
    }

    // there is one more zero after the pattern that needs to be skipped
    while (file.get() != 0) {
    }

    // now there is patient info divided into four fields: [w][id1][x][id2][y][surname][z][name][null null null] wher w,x,y,z are numbers of characters in the next field.
    // Since it can be zero, the whole header is terminated by three consecutive nulls.
    // char currentChar;
    int nullCount = 0;

    while (file.get(currentChar)) {
        if (currentChar == '\0') {
            nullCount++;
            if (nullCount == 3)
                break;
        } else {
            nullCount = 0;
        }

        result = "";
        no = int(currentChar);
        //qDebug() << no;


        for (int i = 0; i < no; i++){
            ch = file.get();
            result += ch;
        }

        //qDebug() << QString::fromStdString(result);
        results.push_back(result);
    }

    return results;
}

QDateTime decode_harmonie_time(double start_time){
    QDateTime beginning_of_epoch(QDate(1, 1, 1), QTime(0, 0, 0)); // No Year 0 in QDateTime, so I need to set it to January 1st year 1 and then deduct one year and one day...
    QDateTime record_start = beginning_of_epoch.addSecs(start_time-365*24*60*60-24*60*60);

    if(record_start.isDaylightTime()){
        record_start = record_start.addSecs(-3600); // not sure why this is not automatic
    }
    return record_start;
}

int sexFromID(std::string id){
    qDebug() << QString::fromStdString(id);

    if(id == "0"){
        return 0;
    }

    std::string month = id.substr(2, 2); // 0 = female, 1 = male
    int m = atoi(month.c_str());
    if(m <= 12){
        return 1;
    }
    else{
        return 0;
    }
}

std::vector<int> read_harmonie_channels_info(std::ifstream &file){ // TO DO - make it read also channel labels
    std::string pattern = "CRecordingMontage";
    std::string montage_name = "";
    char ch;
    short no;
    short r_INF1_NUM;
    short sf;
    int name_length;
    std::vector<int> sfs;
    char currentChar;
    std::string buffer;

    // find the position of pattern in binary file

    while (file.get(currentChar)) {
        buffer += currentChar;
        if (buffer.size() > pattern.size())
            buffer.erase(0, 1);

        if (buffer == pattern) {
            std::cout << pattern << " found at position: "  << file.tellg() << std::endl;
            break;
        }
    }

    file.get(currentChar); //read two empty spaces
    file.get(currentChar);
    name_length = file.get(); // read length of montage name

    //std::cout << "name_length: " << name_length << std::endl;

    for (int i = 0; i < name_length; i++){ // read the name of recording montage
        ch = file.get();
        montage_name += ch;
    }

    //std::cout << "montage_name: " << montage_name << std::endl;
    file.read((char *)&no,sizeof(short)); // read number of channels
    //std::cout << "no of channels: " << no << std::endl;
    file.read((char *)&r_INF1_NUM,sizeof(short));
    //std::cout << "r_INF1_NUM: " << r_INF1_NUM << std::endl;

    for (int i = 0; i < r_INF1_NUM; i++){
        file.read((char *)&sf,sizeof(short));
        //std::cout << "sampling frequency: " << sf << std::endl;
        sfs.push_back(sf);
    }

    std::cout << "sfs size: " << sfs.size() << std::endl;

    // from PJs MATLAB script - r_INF*_NUM is the number channels/items to read, r_INF* is the vector of values
    // INF8 = channel labels, not sure about the others, maybe channel types (different numbers in ECG)
    // see Browser --> Channels --> Edit --> Properties
    //       r_INF1_NUM=fread(FIDSOURCE,1,'int16');
    //       r_INF1=fread(FIDSOURCE,r_INF1_NUM,'int16');
    //       r_INF2_NUM=fread(FIDSOURCE,1,'int16');
    //       r_INF2=fread(FIDSOURCE,r_INF2_NUM,'int16');
    //       r_INF3_NUM=fread(FIDSOURCE,1,'int16');
    //       r_INF3=fread(FIDSOURCE,r_INF3_NUM,'int16');
    //       r_INF4_NUM=fread(FIDSOURCE,1,'int16');
    //       r_INF4=fread(FIDSOURCE,r_INF4_NUM,'int16');
    //       r_INF5_NUM=fread(FIDSOURCE,1,'int16');
    //       r_INF5=fread(FIDSOURCE,r_INF5_NUM,'int16');
    //       r_INF6_NUM=fread(FIDSOURCE,1,'int16');
    //       r_INF6=fread(FIDSOURCE,r_INF5_NUM,'int32');
    //       r_INF7_NUM=fread(FIDSOURCE,3,'int16');
    //       r_INF8_NUM=fread(FIDSOURCE,1,'int16');
    //       for i=1:r_INF8_NUM
    //           LABEL_NUM=fread(FIDSOURCE,1,'char');
    //           r_LABEL{i}=fread(FIDSOURCE,LABEL_NUM,'*char');
    //       end
    return sfs;
}

QRecord read_harmonie_file(QFileInfo fileInfo)
{
    std::string path = fileInfo.filePath().toLocal8Bit().toStdString();

    //std::cout << path << std::endl;

    QRecord qrecord;
    std::vector<std::string> results;
    double start_time_double;
    QDateTime record_start;

    std::ifstream file(path, std::ios::binary);

    if (!file){
        std::cout << "ERROR: Cannot open the file..." << std::endl;
        return qrecord;
    }


    if (file.fail()){
        std::cout << "ERROR: Cannot open the file..." << std::endl;
        return qrecord;
    }

    start_time_double = header_harmonie_read_starting_time(file);
    record_start = decode_harmonie_time(start_time_double);

    // this is for debugging the one hour difference in calculated times
    //    QRegularExpression re("(\\d\\d\\d\\d-\\d\\d-\\d\\d) (\\d\\d)H(\\d\\d)M",QRegularExpression::CaseInsensitiveOption);
    //    QRegularExpressionMatch match = re.match(fileInfo.filePath());
    //    if (match.hasMatch()) {
    //        QString stringFromName = match.captured(1) + " " + match.captured(2) +":" + match.captured(3); // match.captured(0) is the whole match
    //        QDateTime dateTimefromString = QDateTime::fromString(stringFromName, "yyyy-MM-dd hh:mm");
    //        if(dateTimefromString.secsTo(record_start) > 3600){
    //            qDebug() << fileInfo.filePath();
    //            qDebug() << dateTimefromString.secsTo(record_start);
    //            qDebug() << record_start.isDaylightTime();
    //        }
    //    }

    results = header_harmonie_read_patient_info(file);
    std::cout << results.at(0) <<"|"<< results.at(1) <<"|"<< results.at(2) <<"|"<< results.at(3) << "|" << record_start.toString().toLocal8Bit().toStdString() <<"\n"<< std::endl;
    //header_harmonie_read_annotations(file);
    std::vector<int> sfs = read_harmonie_channels_info(file);

    //    QFile log_file("harmonie_log.txt");
    //    if(log_file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text))
    //    {
    //        QTextStream stream(&log_file);
    //        //stream << "file " << path << " has finished recording " << QDateTime::currentDateTime().toLocalTime().toString() << "\n";
    //        stream << fileInfo.filePath() + "\n";
    //        stream << QString::fromLocal8Bit(results.at(0).c_str()) + "|" + QString::fromLocal8Bit(results.at(1).c_str()) + "|" + QString::fromLocal8Bit(results.at(2).c_str()) + "|" + QString::fromLocal8Bit(results.at(3).c_str()) + "|" + record_start.toString("yyyy-MM-dd hh:mm") + "\n\n";
    //        file.close();
    //    }

    QString patientName = QString::fromLocal8Bit(results.at(2).c_str()) + " " + QString::fromLocal8Bit(results.at(3).c_str());
    QString SigFilePath = fileInfo.canonicalPath() + "/" + fileInfo.baseName() + ".SIG";
    QFileInfo SigFileInfo(SigFilePath);

    if(!SigFileInfo.exists()){ // check if the *.SIG exists
        return qrecord;
    }

    const long long SigFileSize = SigFileInfo.size();

    // export file info in qrecord
    qrecord.setID(results.at(0));
    qrecord.setPath(SigFilePath);
    qrecord.name = patientName;
    qrecord.file_size = SigFileSize;
    qrecord.record_start = record_start;
    qrecord.recording_flag = 0;
    qrecord.video_flag = QFileInfo::exists(fileInfo.canonicalPath() + "/" + fileInfo.baseName() + ".MPG"); // bool to int
    qrecord.sex = sexFromID(results.at(0));
    qrecord.record_duration_s = (SigFileSize/((64*float(sfs.size()) +4)*2)) * (64/float(sfs[0])); // every block = 64*no_channels + 4 [samples], 1 sample = 16 bits = 2 bytes, 1 block = 64/sf [s]
    qrecord.check_flag = 1;
    qrecord.recording_system = "Harmonie";
    qrecord.class_code = "Harmonie";

    return qrecord;
}
