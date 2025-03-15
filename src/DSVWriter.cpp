#include "../include/DSVWriter.h"
#include "../include/DataSink.h"
// #include "DSVWriter.h"
// #include "DataSink.h"

// core implementation for dsv writer functionality
struct CDSVWriter::SImplementation {
    std::shared_ptr<CDataSink> Sink;
    char Delimiter;
    bool QuoteAll;

    SImplementation(std::shared_ptr<CDataSink> sink, char delimiter, bool quoteall)
        : Sink(sink), Delimiter(delimiter), QuoteAll(quoteall) {}

    // formats and writes a single row to the sink
    bool WriteRow(const std::vector<std::string>& row) {
        for (size_t i = 0; i < row.size(); ++i) {
            // cell requires quotes if quoteall is true, or if it contains the delimiter or a quote
            bool quotes = QuoteAll ||
                          row[i].find(Delimiter) != std::string::npos ||
                          row[i].find('"') != std::string::npos;
            if (quotes) {
                Sink->Put('"');
                for (char ch : row[i]) {
                    if (ch == '"')
                        Sink->Put('"'); // escape internal quotes
                    Sink->Put(ch);
                }
                Sink->Put('"');
            } else {
                for (char ch : row[i])
                    Sink->Put(ch);
            }
            if (i < row.size() - 1)
                Sink->Put(Delimiter);
        }
        return Sink->Put('\n');
    }
};

CDSVWriter::CDSVWriter(std::shared_ptr<CDataSink> sink, char delimiter, bool quoteall)
    : DImplementation(std::make_unique<SImplementation>(sink, delimiter, quoteall)) {}

CDSVWriter::~CDSVWriter() = default;

bool CDSVWriter::WriteRow(const std::vector<std::string>& row) {
    return DImplementation->WriteRow(row);
}