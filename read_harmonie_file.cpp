#include "read_harmonie_file.h"

void header_harmonie_read_starting_time(std::ifstream &file){

    std::string result = "";
    double f;

    // find the position of pattern in binary file
    // inspired by this: https://stackoverflow.com/questions/58670046/how-can-i-check-if-string-exists-in-binary-file
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

    // this read the 64bit float that giver number of seconds between start of the file and 1.1. 0000 00:00
    file.read(reinterpret_cast<char *>(&f), sizeof(f));

    std::cout.precision(17);
    std::cout << "Date: " << std::fixed << f << std::endl;


    //    // then it reads Harmonie 5.4  EN-0ÿÿ - I guess I dont need that...
    //    while ((ch = file.get()) != 0) {
    //        result += ch;
    //    }

    //    std::cout << result << std::endl;

}




void header_harmonie_read_patient_info(std::ifstream &file){
    //for XP

    std::string result = "";
    char ch;
    int no;
    std::vector<std::string> results;

    // find the position of pattern in binary file
    // inspired by this: https://stackoverflow.com/questions/58670046/how-can-i-check-if-string-exists-in-binary-file
    std::string pattern = "CPatientInfo";


    file.seekg(0, std::ios::end);
    auto size = file.tellg();
    file.seekg(0);
    std::vector<char> content(size, '\0');
    file.read(content.data(), size);
    auto res = std::search(content.begin(), content.end(), pattern.begin(), pattern.end());

    std::cout << res - content.begin() << std::endl;

    file.seekg(res-content.begin());


    std::cout << pattern << " found at position: "  << file.tellg() << std::endl;

    // there is one more zero after the pattern that needs to be skipped
    while (file.get() != 0) {
    }


    //    while ((ch = file.get()) != 0) {
    //        result += ch;
    //        //std::cout << ch << std::endl;
    //        //std::cout  << char(ch) << std::endl;
    //    }

    char three[3];

    // look for three consecutive nulls
    // https://stackoverflow.com/questions/20595336/peek-multiple-places-ahead

    while ((ch = file.get()) != 0) {
        result = "";
        no = int(ch);
        for (int i = 0; i < no; i++){
            ch = file.get();
            result += ch;
        }
        results.push_back(result);
    }

    std::cout << results.at(0) <<"|"<< results.at(1) <<"|"<< results.at(2) <<"|"<< results.at(3) << std::endl;
}

QRecord read_harmonie_file(QFileInfo fileInfo)
{

    const long long file_size = fileInfo.size();

    std::string path = fileInfo.filePath().toLocal8Bit().toStdString();

    std::cout << path << std::endl;

    QRecord qrecord;

    std::ifstream file(path, std::ios::binary);

    if (!file){
        std::cout << "ERROR: Cannot open the file..." << std::endl;
        return qrecord;
    }


    if (file.fail()){
        std::cout << "ERROR: Cannot open the file..." << std::endl;
        return qrecord;
    }

    header_harmonie_read_starting_time(file);
    header_harmonie_read_patient_info(file);
    //header_harmonie_read_annotations(file);

    return qrecord;

}
