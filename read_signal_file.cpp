#include "read_signal_file.h"

// function definitions

time_t decode_date_time(long date, long time)
{
    int y = floor(date / (31 * 12));
    int m = floor(date / 31);
    m = m % 12;

    if (m == 0)
    {
        m = 12;
        y--;
    }

    int d = date % 31;

    if (d == 0)
    {
        d = 31;
        m--;
    }
    //cout << y << " " << m << " " << d <<endl;

    int h = floor(time / (60 * 60 * 1000));
    int min = floor(time / (60 * 1000));
    min = min % 60;
    int s = floor(time / 1000);
    s = s % 60;
    int ms = time % 1000;
    //cout << h << " " << min << " " << s << " " << ms <<endl;

    struct tm tm
    {
        0
    };

    tm.tm_year = y - 1900;
    tm.tm_mon = m - 1;
    tm.tm_mday = d;
    tm.tm_hour = h;
    tm.tm_min = min;
    tm.tm_sec = s;
    time_t t = mktime(&tm);
    return t;
};

void decode_time(long time)
{
    int h = floor(time / (60 * 60 * 1000));
    int min = floor(time / (60 * 1000));
    min = min % 60;
    int s = floor(time / 1000);
    s = s % 60;
    int ms = time % 1000;

    cout << h << ":" << min << ":" << s << "." << ms << endl;
}

template <typename T>
vector<T> readChannel(T tch, fstream &file, int nch)
{
    vector<T> x;
    for (int i = 0; i < nch; i++)
    {
        file.read(reinterpret_cast<char *>(&tch), sizeof(tch));
        x.push_back(tch);
        //cout << x[i] << " ";
    }
    //cout <<endl;
    return x;
}

template <typename ByteArray>
vector<string> readChannelChar(ByteArray &a, fstream &file, int nch)
{
    vector<string> x;
    for (int i = 0; i < nch; i++)
    {
        file.read(reinterpret_cast<char *>(&a), sizeof(a));
        x.push_back(a);
    }
    return x;
}

void whereAmI(fstream &file)
{
    int testing = 1;
    if(testing){
       cout << "Position in file: " << file.tellg() << endl;
    }
}

SignalHeader read_signal_header(fstream &file)
{
    SignalHeader stud;
    file.read(reinterpret_cast<char *>(&stud.program_id), sizeof(stud.program_id));
    file.read(reinterpret_cast<char *>(&stud.signal_id), sizeof(stud.signal_id));
    file.read(reinterpret_cast<char *>(&stud.version_id), sizeof(stud.version_id));
    file.read(reinterpret_cast<char *>(&stud.read_only), sizeof(stud.read_only));
    return stud;
}

DataTable read_data_table(fstream &file)
{
    DataTable data_table;
    long y;
    vector<long> dt = readChannel(y, file, 17);
    data_table.measurement_info = Block{dt[0], dt[1], NULL};
    data_table.recorder_montage_info = Block{dt[2], dt[3]};
    data_table.events_info = Block{dt[4], dt[5]};
    data_table.notes_info = Block{dt[6], dt[7]};
    data_table.impedance_info = Block{dt[8], dt[9]};
    data_table.display_montages_info = Block{dt[10], dt[11]};
    data_table.stimulator_info = Block{dt[12], dt[13]};
    data_table.signal_info = Block{dt[14], dt[15], dt[16]};
    return data_table;
}

Measurement read_measurement(fstream &file, long offset, long size)
{
    Measurement measurement;
    file.seekg(offset);
    file.read(measurement.id, sizeof(measurement.id));
    file.read(measurement.name, sizeof(measurement.name));
    file.read(measurement.street, sizeof(measurement.street));
    file.read(measurement.zip_code, sizeof(measurement.zip_code));
    file.read(measurement.city, sizeof(measurement.city));
    file.read(measurement.state, sizeof(measurement.street));
    file.read(measurement.country, sizeof(measurement.street));
    file.read(reinterpret_cast<char *>(&measurement.birthday), sizeof(measurement.birthday));
    file.read(reinterpret_cast<char *>(&measurement.sex), sizeof(measurement.sex));
    file.read(reinterpret_cast<char *>(&measurement.start_date), sizeof(measurement.start_date));
    file.read(reinterpret_cast<char *>(&measurement.start_hour), sizeof(measurement.start_hour));
    file.read(measurement.room, sizeof(measurement.room));
    file.read(measurement.doctor, sizeof(measurement.doctor));
    file.read(measurement.technician, sizeof(measurement.technician));
    file.read(measurement.class_code, sizeof(measurement.class_code));
    file.read(measurement.clin_info, sizeof(measurement.clin_info));
    file.read(measurement.backup_flag, sizeof(measurement.backup_flag));
    file.read(reinterpret_cast<char *>(&measurement.archive_flag), sizeof(measurement.archive_flag));
    file.read(reinterpret_cast<char *>(&measurement.vcr_timing_correction), sizeof(measurement.vcr_timing_correction));
    file.read(measurement.referring_doctor_name, sizeof(measurement.referring_doctor_name));
    file.read(measurement.referring_doctor_code, sizeof(measurement.referring_doctor_code));
    file.read(reinterpret_cast<char *>(&measurement.weight), sizeof(measurement.weight));
    file.read(reinterpret_cast<char *>(&measurement.height), sizeof(measurement.height));
    file.read(reinterpret_cast<char *>(&measurement.weight_unit), sizeof(measurement.weight_unit));
    file.read(reinterpret_cast<char *>(&measurement.height_unit), sizeof(measurement.height_unit));
    file.read(measurement.protocol, sizeof(measurement.protocol));
    file.read(reinterpret_cast<char *>(&measurement.maximum_voltage), sizeof(measurement.maximum_voltage));
    file.read(reinterpret_cast<char *>(&measurement.maximum_amplitude), sizeof(measurement.maximum_amplitude));
    return measurement;
}

Record read_signal_file(string file_path){

    // GET THE FILE SIZE
    ifstream myfile(file_path, ios::binary);
    const auto begin = myfile.tellg();
    myfile.seekg(0, ios::end);
    const auto end = myfile.tellg();
    const auto file_size = (end - begin);
    //cout << "File size: " << file_size << " bytes" << endl;

    // READ THE FILE
    streampos fileSize;
    fstream file(file_path, ios::in | ios::out | ios::binary);

    if (file.fail())
    {
        cout << "ERROR: Cannot open the file..." << endl;
        exit(0);
    }

    // Signal struct
    SignalFile signal;

    // Signal header
    signal.header = read_signal_header(file);

    // Data table
    signal.data_table = read_data_table(file);

    //whereAmI(file); // should be 80
    // Measurement
    signal.measurement = read_measurement(file, signal.data_table.measurement_info.offset, signal.data_table.measurement_info.size);
    file.close();

    Record record;
    // or make special constructor for this?
    record.record_start = decode_date_time(signal.measurement.start_date, signal.measurement.start_hour);
    record.id = signal.measurement.id;
    record.name = signal.measurement.name;
    record.sex = signal.measurement.sex;
    record.protocol = signal.measurement.protocol;
    record.class_code = signal.measurement.class_code;
    record.file_path = file_path;
    string file_name = file_path.substr(file_path.find_last_of("/\\") + 1);
    std::string::size_type const p(file_name.find_last_of('.'));
    record.file_name = file_name.substr(0, p);

    //cout << record.file_name << endl;

    // removes "/" in ID (rodné èíslo)
    record.id.erase(std::remove(record.id.begin(), record.id.end(), '/'), record.id.end());

   return record;
}
