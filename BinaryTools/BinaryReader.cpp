#include "BinaryReader.h"
#include "MemoryBuffer.h"

BinaryReader::BinaryReader(const std::string& inputPath)
{
    stream_ = new std::ifstream(inputPath, std::ifstream::in | std::ifstream::binary);
}

BinaryReader::BinaryReader(char* buffer, uint32_t sizeInBytes)
{
    buffer_ = new basic_memstreambuf<char>(buffer, sizeInBytes);
    stream_ = new std::istream(buffer_, false);
}

BinaryReader::BinaryReader(std::span<uint8_t> buffer)
{
    buffer_ = new basic_memstreambuf<char>((char*)buffer.data(), buffer.size_bytes());
    stream_ = new std::istream(buffer_, false);
}

BinaryReader::~BinaryReader()
{
    delete stream_;
    if (buffer_)
        delete buffer_;
}

uint8_t BinaryReader::ReadUint8()
{
    uint8_t output;
    stream_->read(reinterpret_cast<char*>(&output), 1);
    return output;
}

uint16_t BinaryReader::ReadUint16()
{
    uint16_t output;
    stream_->read(reinterpret_cast<char*>(&output), 2);
    return output;
}

uint32_t BinaryReader::ReadUint32()
{
    //Todo: See if using static or class var speeds these up
    uint32_t output;
    stream_->read(reinterpret_cast<char*>(&output), 4);
    return output;
}

uint64_t BinaryReader::ReadUint64()
{
    uint64_t output;
    stream_->read(reinterpret_cast<char*>(&output), 8);
    return output;
}

int8_t BinaryReader::ReadInt8()
{
    int8_t output;
    stream_->read(reinterpret_cast<char*>(&output), 1);
    return output;
}

int16_t BinaryReader::ReadInt16()
{
    int16_t output;
    stream_->read(reinterpret_cast<char*>(&output), 2);
    return output;
}

int32_t BinaryReader::ReadInt32()
{
    int32_t output;
    stream_->read(reinterpret_cast<char*>(&output), 4);
    return output;
}

int64_t BinaryReader::ReadInt64()
{
    int64_t output;
    stream_->read(reinterpret_cast<char*>(&output), 8);
    return output;
}

char BinaryReader::ReadChar()
{
    char output;
    stream_->read(&output, 1);
    return output;
}

std::string BinaryReader::ReadNullTerminatedString()
{
    std::string output;
    char charBuffer = 0;
    while(PeekChar() != '\0')
    {
        stream_->read(&charBuffer, 1);
        output.push_back(charBuffer);
    }
    Skip(1);
    return output;
}

std::string BinaryReader::ReadFixedLengthString(size_t length)
{
    std::string output;
    for (int i = 0; i < length; i++)
    {
        char charBuffer;
        stream_->read(&charBuffer, 1);
        output.push_back(charBuffer);
    }
    return output;
}

char BinaryReader::PeekChar()
{
    char output = ReadChar();
    SeekCur(-1);
    return output;
}

float BinaryReader::ReadFloat()
{
    float output;
    stream_->read(reinterpret_cast<char*>(&output), 4);
    return output;
}

double BinaryReader::ReadDouble()
{
    double output;
    stream_->read(reinterpret_cast<char*>(&output), 8);
    return output;
}

void BinaryReader::ReadToMemory(void* destination, size_t size)
{
    stream_->read(static_cast<char*>(destination), size);
}

void BinaryReader::SeekBeg(size_t absoluteOffset)
{
    stream_->seekg(absoluteOffset, std::ifstream::beg);
}

void BinaryReader::SeekCur(size_t relativeOffset)
{
    stream_->seekg(relativeOffset, std::ifstream::cur);
}

void BinaryReader::Skip(size_t bytesToSkip)
{
    stream_->seekg(bytesToSkip, std::ifstream::cur);
}

size_t BinaryReader::Align(size_t alignmentValue)
{
    //Todo: Test that this math is working as expected. Had bug here in C# version
    const size_t remainder = stream_->tellg() % alignmentValue;
    size_t paddingSize = remainder > 0 ? alignmentValue - remainder : 0;
    Skip(paddingSize);
    return paddingSize;
}

size_t BinaryReader::Position() const
{
    return stream_->tellg();
}

size_t BinaryReader::Length()
{
    //Save current position
    size_t realPosition = Position();

    //Seek to end of file and get position (the length)
    stream_->seekg(0, std::ios::end);
    size_t endPosition = Position();

    //Seek back to real pos and return length
    SeekBeg(realPosition);
    return endPosition;
}
