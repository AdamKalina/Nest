#ifndef READ_HARMONIE_FILE_H
#define READ_HARMONIE_FILE_H

#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm> // std::search
#include <string>
#include <iterator>
#include <cstdio>

#include <QFileInfo>
#include <QDateTime>
#include <QDebug>

#include "qpatient.h"

QRecord read_harmonie_file(QFileInfo fileInfo);
std::vector<std::string> header_harmonie_read_patient_info(std::ifstream &file);
double header_harmonie_read_starting_time(std::ifstream &file);


#endif // READ_HARMONIE_FILE_H
