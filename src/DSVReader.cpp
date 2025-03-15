#include "../include/DSVReader.h"
// #include "DSVReader.h"
// impl details for dsv reading
struct CDSVReader::SImplementation {
    std::shared_ptr<CDataSource> DataSource;
    char Delimiter;

    SImplementation(std::shared_ptr<CDataSource> src, char delimiter)
        : DataSource(std::move(src)), Delimiter(delimiter) {}

    // reads one row from the source; handles quotes, delimiters, and line breaks
    bool ReadRow(std::vector<std::string>& currentRow) {
        currentRow.clear();
        std::string currentCell;
        char currentChar;
        int in_quotes = 0;
        int has_data = 0;

        while (!DataSource->End()) {
            if (!DataSource->Get(currentChar))
                return false;
            has_data = 1;

            if (currentChar == '"') {
                // manage quote state or process an escaped quote
                if (!DataSource->End()) {
                    char nextChar;
                    if (DataSource->Peek(nextChar) && nextChar == '"') {
                        DataSource->Get(nextChar);
                        currentCell.push_back('"');
                    } else {
                        if (in_quotes == 1){
                            in_quotes = 0;
                        } else {
                            in_quotes = 1;
                        }
                    }
                } else {
                    if (in_quotes == 1){
                        in_quotes = 0;
                    } else {
                        in_quotes = 1;
                    }
                }
            } else if (currentChar == Delimiter && in_quotes == 0) {
                currentRow.push_back(currentCell);
                currentCell.clear();
            } else if ((currentChar == '\n' || currentChar == '\r') && in_quotes == 0) {
                if (!currentCell.empty() || !currentRow.empty())
                    currentRow.push_back(currentCell);
                if (currentChar == '\r' && !DataSource->End()) {
                    char nextChar;
                    if (DataSource->Peek(nextChar) && nextChar == '\n')
                        DataSource->Get(nextChar);
                }
                return true;
            } else {
                currentCell.push_back(currentChar);
            }
        }
        if (!currentCell.empty() || has_data == 1)
            currentRow.push_back(currentCell);
        return has_data == 1;
    }
};

CDSVReader::CDSVReader(std::shared_ptr<CDataSource> src, char delimiter)
    : DImplementation(std::make_unique<SImplementation>(src, delimiter)) {}

CDSVReader::~CDSVReader() = default;

bool CDSVReader::End() const {
    return DImplementation->DataSource->End();
}

bool CDSVReader::ReadRow(std::vector<std::string> &row) {
    return DImplementation->ReadRow(row);
}