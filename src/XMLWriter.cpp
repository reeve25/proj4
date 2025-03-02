// #include "../include/XMLWriter.h"
#include "XMLWriter.h"
#include <vector>
#include <string>

// core implementation for xml writing
struct CXMLWriter::SImplementation {
    std::shared_ptr<CDataSink> sink_;    // output sink
    std::vector<std::string> elem_stack_;  // stack of open elements

    SImplementation(std::shared_ptr<CDataSink> sink)
        : sink_(sink) {}

    // write a string char by char to the sink
    bool write_str(const std::string &s) {
        for (char c : s) {
            if (!sink_->Put(c))
                return false;
        }
        return true;
    }

    // output the string with xml escaping applied
    bool escape_str(const std::string &s) {
        for (char c : s) {
            if (c == '<') {
                if (!write_str("&lt;"))
                    return false;
            } else if (c == '>') {
                if (!write_str("&gt;"))
                    return false;
            } else if (c == '&') {
                if (!write_str("&amp;"))
                    return false;
            } else if (c == '\'') {
                if (!write_str("&apos;"))
                    return false;
            } else if (c == '"') {
                if (!write_str("&quot;"))
                    return false;
            } else {
                if (!sink_->Put(c))
                    return false;
            }
        }
        return true;
    }

    // close any still-open tags, writing closing tags in reverse order
    bool close_all_tags() {
        for (auto it = elem_stack_.rbegin(); it != elem_stack_.rend(); ++it) {
            if (!write_str("</") || !write_str(*it) || !write_str(">"))
                return false;
        }
        elem_stack_.clear();
        return true;
    }

    // output an xml entity based on its type
    bool write_ent(const SXMLEntity &ent) {
        if (ent.DType == SXMLEntity::EType::StartElement) {
            if (!write_str("<") || !write_str(ent.DNameData))
                return false;
            for (const auto &attr : ent.DAttributes) {
                if (!write_str(" ") || !write_str(attr.first) ||
                    !write_str("=") || !write_str("\"") || !escape_str(attr.second) ||
                    !write_str("\""))
                    return false;
            }
            if (!write_str(">"))
                return false;
            elem_stack_.push_back(ent.DNameData);
        }
        else if (ent.DType == SXMLEntity::EType::EndElement) {
            if (!write_str("</") || !write_str(ent.DNameData) || !write_str(">"))
                return false;
            if (!elem_stack_.empty())
                elem_stack_.pop_back();
        }
        else if (ent.DType == SXMLEntity::EType::CharData) {
            if (!escape_str(ent.DNameData))
                return false;
        }
        else if (ent.DType == SXMLEntity::EType::CompleteElement) {
            if (!write_str("<") || !write_str(ent.DNameData))
                return false;
            for (const auto &attr : ent.DAttributes) {
                if (!write_str(" ") || !write_str(attr.first) ||
                    !write_str("=") || !write_str("\"") || !escape_str(attr.second) ||
                    !write_str("\""))
                    return false;
            }
            if (!write_str("/>"))
                return false;
        }
        return true;
    }
};

CXMLWriter::CXMLWriter(std::shared_ptr<CDataSink> sink)
    : DImplementation(std::make_unique<SImplementation>(sink)) {}

CXMLWriter::~CXMLWriter() = default;

bool CXMLWriter::Flush() {
    return DImplementation->close_all_tags();
}

bool CXMLWriter::WriteEntity(const SXMLEntity &entity) {
    return DImplementation->write_ent(entity);
}