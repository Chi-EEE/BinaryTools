#pragma once

#include <fstream>
#include <string>
#include <vector>

#include <binary_tools/MemoryBuffer.h>

namespace binary_tools
{
    struct MemoryBuffer;

    // Class that can read binary data either from a file or from a fixed size buffer
    // depending on the constructor used.
    class BinaryReader
    {
    public:
        // Reads binary data from file at path
        BinaryReader(std::string_view inputPath)
        {
            stream_ = new std::ifstream(std::string(inputPath), std::ifstream::in | std::ifstream::binary);
        }

        // Reads binary data from fixed size memory buffer
        BinaryReader(char *buffer, uint32_t sizeInBytes)
        {
            buffer_ = new basic_memstreambuf(buffer, sizeInBytes);
            stream_ = new std::istream(buffer_, false);
        }

        // Reads binary data from fixed size memory buffer
        BinaryReader(uint8_t *buffer, std::size_t length)
        {
            buffer_ = new basic_memstreambuf((char *)buffer, length);
            stream_ = new std::istream(buffer_, false);
        }

        ~BinaryReader()
        {
            delete stream_;
            if (buffer_)
                delete buffer_;
        }

#pragma region Unsigned integers
        [[nodiscard]] uint8_t ReadUint8()
        {
            uint8_t output;
            stream_->read(reinterpret_cast<char *>(&output), 1);
            return output;
        }

        [[nodiscard]] uint16_t ReadUint16()
        {
            uint16_t output;
            stream_->read(reinterpret_cast<char *>(&output), 2);
            return output;
        }

        [[nodiscard]] uint32_t ReadUint32()
        {
            // Todo: See if using static or class var speeds these up
            uint32_t output;
            stream_->read(reinterpret_cast<char *>(&output), 4);
            return output;
        }

        [[nodiscard]] uint64_t ReadUint64()
        {
            uint64_t output;
            stream_->read(reinterpret_cast<char *>(&output), 8);
            return output;
        }
#pragma endregion

#pragma region Signed integers
        [[nodiscard]] int8_t ReadInt8()
        {
            int8_t output;
            stream_->read(reinterpret_cast<char *>(&output), 1);
            return output;
        }

        [[nodiscard]] int16_t ReadInt16()
        {
            int16_t output;
            stream_->read(reinterpret_cast<char *>(&output), 2);
            return output;
        }

        [[nodiscard]] int32_t ReadInt32()
        {
            int32_t output;
            stream_->read(reinterpret_cast<char *>(&output), 4);
            return output;
        }

        [[nodiscard]] int64_t ReadInt64()
        {
            int64_t output;
            stream_->read(reinterpret_cast<char *>(&output), 8);
            return output;
        }
#pragma endregion

        [[nodiscard]] bool ReadBoolean()
        {
            return this->ReadUint8() != 0;
        }

#pragma region Bytes
        [[nodiscard]] uint8_t ReadByte()
        {
            return this->ReadUint8();
        }

        [[nodiscard]] std::vector<uint8_t> ReadBytes(size_t count)
        {
            std::vector<uint8_t> output(count);
            stream_->read(reinterpret_cast<char *>(&output), count);
            return output;
        }
#pragma endregion

#pragma region Characters
        [[nodiscard]] char ReadChar()
        {
            char output;
            stream_->read(&output, 1);
            return output;
        }

        [[nodiscard]] wchar_t ReadCharWide()
        {
            wchar_t output;
            stream_->read((char *)&output, 2);
            return output;
        }

        [[nodiscard]] std::string ReadNullTerminatedString()
        {
            std::string output;
            char charBuffer = 0;
            while (PeekChar() != '\0')
            {
                stream_->read(&charBuffer, 1);
                output.push_back(charBuffer);
            }
            Skip(1); // Move past null terminator
            return output;
        }

        [[nodiscard]] std::string ReadFixedLengthString(size_t length)
        {
            std::string output;
            output.reserve(length);
            for (int i = 0; i < length; i++)
            {
                char charBuffer;
                stream_->read(&charBuffer, 1);
                output.push_back(charBuffer);
            }
            return output;
        }

        [[nodiscard]] std::wstring ReadNullTerminatedStringWide()
        {
            std::wstring output;
            wchar_t charBuffer = 0;
            while (PeekCharWide() != '\0')
            {
                stream_->read((char *)&charBuffer, 2);
                output.push_back(charBuffer);
            }
            Skip(2); // Move past null terminator
            return output;
        }

        [[nodiscard]] std::wstring ReadFixedLengthStringWide(size_t length)
        {
            std::wstring output;
            output.reserve(length);
            for (int i = 0; i < length; i++)
            {
                wchar_t charBuffer;
                stream_->read((char *)&charBuffer, 2);
                output.push_back(charBuffer);
            }
            return output;
        }

        [[nodiscard]] std::vector<std::string> ReadSizedStringList(size_t listSize)
        {
            std::vector<std::string> stringList = {};
            if (listSize == 0)
                return stringList;

            size_t startPos = Position();
            while (Position() - startPos < listSize)
            {
                stringList.push_back(ReadNullTerminatedString());
                while (Position() - startPos < listSize)
                {
                    // TODO: See if Align(4) would accomplish the same. This is really for RfgTools++ since many RFG formats have sized string lists
                    // Sometimes names have extra null bytes after them for some reason. Simple way to handle this
                    if (PeekChar() == '\0')
                        Skip(1);
                    else
                        break;
                }
            }

            return stringList;
        }
#pragma endregion

#pragma region Peek
        [[nodiscard]] char PeekChar()
        {
            char output = ReadChar();
            SeekReverse(1);
            return output;
        }

        [[nodiscard]] wchar_t PeekCharWide()
        {
            wchar_t output = ReadCharWide();
            SeekReverse(2);
            return output;
        }

        [[nodiscard]] uint32_t PeekUint32()
        {
            uint32_t output = ReadUint32();
            SeekReverse(4);
            return output;
        }
#pragma endregion

#pragma region Floating point
        [[nodiscard]] float ReadFloat()
        {
            float output;
            stream_->read(reinterpret_cast<char *>(&output), 4);
            return output;
        }

        [[nodiscard]] double ReadDouble()
        {
            double output;
            stream_->read(reinterpret_cast<char *>(&output), 8);
            return output;
        }
#pragma endregion

#pragma region Memory
        void ReadToMemory(void *destination, size_t size)
        {
            stream_->read(static_cast<char *>(destination), size);
        }
#pragma endregion

#pragma region Seek
        void SeekBeg(size_t absoluteOffset)
        {
            stream_->seekg(absoluteOffset, std::ifstream::beg);
        }

        void SeekCur(size_t relativeOffset)
        {
            stream_->seekg(relativeOffset, std::ifstream::cur);
        }

        void SeekEnd(size_t relativeOffset)
        {
            stream_->seekg(relativeOffset, std::ifstream::end);
        }

        // Move backwards from the current stream position
        void SeekReverse(size_t relativeOffset)
        {
            const size_t delta = std::min(Position(), relativeOffset); // Don't allow seeking before the beginning of the stream
            const size_t targetOffset = Position() - delta;
            SeekBeg(targetOffset);
        }

        void Skip(size_t bytesToSkip)
        {
            stream_->seekg(bytesToSkip, std::ifstream::cur);
        }
#pragma endregion

#pragma region Alignment
        size_t Align(size_t alignmentValue = 2048)
        {
            // Todo: Test that this math is working as expected. Had bug here in C# version
            const size_t remainder = stream_->tellg() % alignmentValue;
            size_t paddingSize = remainder > 0 ? alignmentValue - remainder : 0;
            Skip(paddingSize);
            return paddingSize;
        }
#pragma endregion

#pragma region Position and length
        size_t Position() const
        {
            return stream_->tellg();
        }

        size_t Length()
        {
            // Save current position
            size_t realPosition = Position();

            // Seek to end of file and get position (the length)
            stream_->seekg(0, std::ios::end);
            size_t endPosition = Position();

            // Seek back to real pos and return length
            if (realPosition != endPosition)
                SeekBeg(realPosition);

            return endPosition;
        }
#pragma endregion

    private:
        std::istream *stream_ = nullptr;
        basic_memstreambuf *buffer_ = nullptr;
    };
}
