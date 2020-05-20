#include "BinaryReader.h"

BinaryReader::BinaryReader(const std::string& inputPath)
{
    stream_.open(inputPath, std::ifstream::in | std::ifstream::binary);
}

BinaryReader::~BinaryReader()
{
    stream_.close();
}

uint8_t BinaryReader::ReadUint8()
{
    uint8_t output;
    stream_.read(reinterpret_cast<char*>(&output), 1);
    return output;
}

uint16_t BinaryReader::ReadUint16()
{
    uint16_t output;
    stream_.read(reinterpret_cast<char*>(&output), 2);
    return output;
}

uint32_t BinaryReader::ReadUint32()
{
    //Todo: See if using static or class var speeds these up
    uint32_t output;
    stream_.read(reinterpret_cast<char*>(&output), 4);
    return output;
}

uint64_t BinaryReader::ReadUint64()
{
    uint64_t output;
    stream_.read(reinterpret_cast<char*>(&output), 8);
    return output;
}

int8_t BinaryReader::ReadInt8()
{
    int8_t output;
    stream_.read(reinterpret_cast<char*>(&output), 1);
    return output;
}

int16_t BinaryReader::ReadInt16()
{
    int16_t output;
    stream_.read(reinterpret_cast<char*>(&output), 2);
    return output;
}

int32_t BinaryReader::ReadInt32()
{
    int32_t output;
    stream_.read(reinterpret_cast<char*>(&output), 4);
    return output;
}

int64_t BinaryReader::ReadInt64()
{
    int64_t output;
    stream_.read(reinterpret_cast<char*>(&output), 8);
    return output;
}

char BinaryReader::ReadChar()
{
    char output;
    stream_.read(&output, 1);
    return output;
}

std::string BinaryReader::ReadNullTerminatedString()
{
    std::string output;
    char charBuffer = 0;
    while(NextChar() != '\n')
    {
        stream_.read(&charBuffer, 1);
        output.push_back(charBuffer);
    }
    return output;
}

std::string BinaryReader::ReadFixedLengthString(int length)
{
    std::string output;
    for (int i = 0; i < length; i++)
    {
        char charBuffer;
        stream_.read(&charBuffer, 1);
        output.push_back(charBuffer);
    }
    return output;
}

char BinaryReader::NextChar()
{
    char output = ReadChar();
    SeekCur(-1);
    return output;
}

float BinaryReader::ReadFloat()
{
    float output;
    stream_.read(reinterpret_cast<char*>(&output), 4);
    return output;
}

double BinaryReader::ReadDouble()
{
    double output;
    stream_.read(reinterpret_cast<char*>(&output), 8);
    return output;
}

void BinaryReader::ReadToMemory(void* destination, size_t size)
{
    stream_.read(static_cast<char*>(destination), size);
}

void BinaryReader::SeekBeg(int absoluteOffset)
{
    stream_.seekg(absoluteOffset, std::ifstream::beg);
}

void BinaryReader::SeekCur(int relativeOffset)
{
    stream_.seekg(relativeOffset, std::ifstream::cur);
}

void BinaryReader::Skip(int bytesToSkip)
{
    stream_.seekg(bytesToSkip, std::ifstream::cur);
}

size_t BinaryReader::Align(int alignmentValue)
{
    //Todo: Test that this math is working as expected. Had bug here in C# version
    const size_t remainder = stream_.tellg() % alignmentValue;
    size_t paddingSize = remainder > 0 ? alignmentValue - remainder : 0;
    Skip(paddingSize);
    return paddingSize;
}