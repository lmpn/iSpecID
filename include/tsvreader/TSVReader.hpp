#include <csv.hpp>
#include <iostream>
#include <vector>
#include <string>

class TSVReader : public csv::CSVReader{
	public:
        TSVReader(csv::CSVFormat _format=tsv_format())
            : csv::CSVReader(_format){ };
        TSVReader(std::string filename, csv::CSVFormat _format=tsv_format())
            : csv::CSVReader(filename,_format){};

    static csv::CSVFormat tsv_format(){
        csv::CSVFormat format;
        format.delimiter('\t').header_row(0);
        return format;
    }
};