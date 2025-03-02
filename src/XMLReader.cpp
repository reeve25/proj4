// #include "../include/XMLReader.h"
#include "XMLReader.h"
#include <expat.h>
#include <queue>
#include <memory>
#include <vector>

// xmlreader implementation using expat for parsing xml data
struct CXMLReader::SImplementation {
    std::shared_ptr<CDataSource> DataSource;  // source for xml input
    XML_Parser Parser;                        // expat parser instance
    std::queue<SXMLEntity> EntityQueue;       // queue for parsed xml nodes
    int IsEndOfData;                          // 1 when input is finished, 0 otherwise
    std::string CharDataBuffer;               // accumulates text between tags

    // handler for start tags; flushes text then queues a start element
    static void StartElementHandler(void* userData, const char* name, const char** attr) {
        auto* impl = static_cast<SImplementation*>(userData);
        impl->FlushCharData();

        SXMLEntity entity;
        entity.DType = SXMLEntity::EType::StartElement;
        entity.DNameData = name;

        if (attr) {
            for (int i = 0; attr[i]; i += 2) {
                if (attr[i + 1])
                    entity.DAttributes.emplace_back(attr[i], attr[i + 1]);
            }
        }
        impl->EntityQueue.push(entity);
    }

    // handler for end tags; flushes buffered text then queues an end element
    static void EndElementHandler(void* userData, const char* name) {
        auto* impl = static_cast<SImplementation*>(userData);
        impl->FlushCharData();

        SXMLEntity entity;
        entity.DType = SXMLEntity::EType::EndElement;
        entity.DNameData = name;
        impl->EntityQueue.push(entity);
    }

    // handler for text data; appends incoming text to the buffer
    static void CharDataHandler(void* userData, const char* data, int length) {
        auto* impl = static_cast<SImplementation*>(userData);
        if (data && length > 0)
            impl->CharDataBuffer.append(data, length);
    }

    // constructor: sets up parser and its callbacks
    SImplementation(std::shared_ptr<CDataSource> src)
        : DataSource(std::move(src)), IsEndOfData(0) {
        Parser = XML_ParserCreate(nullptr);
        XML_SetUserData(Parser, this);
        XML_SetElementHandler(Parser, StartElementHandler, EndElementHandler);
        XML_SetCharacterDataHandler(Parser, CharDataHandler);
    }

    // destructor: frees the parser
    ~SImplementation() {
        XML_ParserFree(Parser);
    }

    // if there is buffered text, package it as a char data entity and clear the buffer
    void FlushCharData() {
        if (!CharDataBuffer.empty()) {
            SXMLEntity entity;
            entity.DType = SXMLEntity::EType::CharData;
            entity.DNameData = CharDataBuffer;
            EntityQueue.push(entity);
            CharDataBuffer.clear();
        }
    }

    // reads and parses data until an entity is available; skips char data if requested
    bool ReadEntity(SXMLEntity& entity, bool skipCharData) {
        while (EntityQueue.empty() && !IsEndOfData) {
            std::vector<char> buffer(4096);
            size_t bytesRead = 0;

            while (bytesRead < buffer.size() && !DataSource->End()) {
                char ch;
                if (DataSource->Get(ch))
                    buffer[bytesRead++] = ch;
                else
                    break;
            }

            if (bytesRead == 0) {
                IsEndOfData = 1;
                XML_Parse(Parser, nullptr, 0, 1);
                break;
            }

            if (XML_Parse(Parser, buffer.data(), bytesRead, 0) == XML_STATUS_ERROR)
                return false;
        }

        if (!EntityQueue.empty()) {
            entity = EntityQueue.front();
            EntityQueue.pop();

            if (skipCharData && entity.DType == SXMLEntity::EType::CharData)
                return ReadEntity(entity, skipCharData);

            return true;
        }
        return false;
    }
};

CXMLReader::CXMLReader(std::shared_ptr<CDataSource> src)
    : DImplementation(std::make_unique<SImplementation>(std::move(src))) {}

CXMLReader::~CXMLReader() = default;

bool CXMLReader::End() const {
    return DImplementation->IsEndOfData && DImplementation->EntityQueue.empty();
}

bool CXMLReader::ReadEntity(SXMLEntity& entity, bool skipCharData) {
    return DImplementation->ReadEntity(entity, skipCharData);
}