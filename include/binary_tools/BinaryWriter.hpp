#pragma once

#include <filesystem>
#include <fstream>
#include <string>

#include <binary_tools/MemoryBuffer.hpp>

namespace binary_tools
{
    // Class that can write binary data either from a file or from a fixed size buffer
    // depending on the constructor used.
    class BinaryWriter
    {
    public:
        // Writes binary data from file at path. If truncate == true any existing file contents will be cleared
        BinaryWriter(std::string_view inputPath, bool truncate = true)
        {
            // Can't simply exclude the truncate flag when !truncate. More details here: https://stackoverflow.com/a/57070159
            int flags = 0;
            if (truncate)
                flags = std::ofstream::out | std::ofstream::binary | std::ofstream::trunc; // Clears existing contents of the file
            else
                flags = std::ofstream::in | std::ofstream::out | std::ofstream::binary;

            // If not truncating and the file doesn't exist, then opening will fail. So we create the file first if it doesn't exist
            if (!truncate && !std::filesystem::exists(inputPath))
            {
                std::fstream f;
                f.open(inputPath, std::fstream::out);
                f.close();
            }

            stream_ = new std::ofstream(std::string(inputPath), flags);
        }

        // Writes binary data from fixed size memory buffer
        BinaryWriter(char *buffer, uint32_t sizeInBytes)
        {
            buffer_ = new MemoryBuffer(buffer, sizeInBytes);
            stream_ = new std::ostream(buffer_);
        }

        ~BinaryWriter()
        {
            delete stream_;
            if (buffer_)
                delete[] buffer_;
        }

        void Flush()
        {
            stream_->flush();
        }

#pragma region Unsigned integers
        void WriteUint8(uint8_t value)
        {
            stream_->write(reinterpret_cast<const char *>(&value), 1);
        }

        void WriteUint16(uint16_t value)
        {
            stream_->write(reinterpret_cast<const char *>(&value), 2);
        }

        void WriteUint32(uint32_t value)
        {
            stream_->write(reinterpret_cast<const char *>(&value), 4);
        }

        void WriteUint64(uint64_t value)
        {
            stream_->write(reinterpret_cast<const char *>(&value), 8);
        }
#pragma endregion

#pragma region Signed integers
        void WriteInt8(int8_t value)
        {
            stream_->write(reinterpret_cast<const char *>(&value), 1);
        }

        void WriteInt16(int16_t value)
        {
            stream_->write(reinterpret_cast<const char *>(&value), 2);
        }

        void WriteInt32(int32_t value)
        {
            stream_->write(reinterpret_cast<const char *>(&value), 4);
        }

        void WriteInt64(int64_t value)
        {
            stream_->write(reinterpret_cast<const char *>(&value), 8);
        }
#pragma endregion

        void WriteBoolean(bool value)
        {
            stream_->write(reinterpret_cast<const char *>(&value), 1);
        }

#pragma region Bytes
        void WriteByte(uint8_t value)
        {
            stream_->write(reinterpret_cast<const char *>(&value), 1);
        }

        void WriteBytes(const uint8_t *data, size_t size)
        {
            stream_->write(reinterpret_cast<const char *>(data), size);
        }
#pragma endregion

#pragma region Characters
        void WriteChar(char value)
        {
            stream_->write(reinterpret_cast<const char *>(&value), 1);
        }

        // Write string to output with null terminator
        void WriteNullTerminatedString(const std::string &value)
        {
            stream_->write(value.data(), value.size());
            WriteChar('\0');
        }

        // Write string to output without null terminator
        void WriteFixedLengthString(const std::string &value)
        {
            stream_->write(value.data(), value.size());
        }
#pragma endregion

#pragma region Floating point
        void WriteFloat(float value)
        {
            stream_->write(reinterpret_cast<const char *>(&value), 4);
        }

        void WriteDouble(double value)
        {
            stream_->write(reinterpret_cast<const char *>(&value), 8);
        }
#pragma endregion

#pragma region Memory
        void WriteFromMemory(const void *data, size_t size)
        {
            stream_->write(reinterpret_cast<const char *>(data), size);
        }

        template <typename T>
        void Write(const T &data)
        {
            // Don't allow T to be a pointer to avoid accidentally writing the value of a pointer instead of what it points to.
            static_assert(!std::is_pointer<T>(), "BinaryWriter::Write<T> requires T to be a non pointer type.");
            WriteFromMemory(&data, sizeof(T));
        }

        template <typename T>
        void WriteSpan(T *data, size_t size)
        {
            WriteFromMemory(data, size);
        }
#pragma endregion

#pragma region Seek
        void SeekBeg(size_t absoluteOffset)
        {
            stream_->seekp(absoluteOffset, std::ifstream::beg);
        }

        void SeekCur(size_t relativeOffset)
        {
            stream_->seekp(relativeOffset, std::ifstream::cur);
        }
#pragma endregion

        void Skip(size_t bytesToSkip)
        {
            size_t position = Position();
            size_t length = Length();

            // If we're skipped past the end of the stream then skip what's available and write null bytes for the rest
            if (position + bytesToSkip > length)
            {
                size_t bytesAvailable = length - position;
                size_t bytesNeeded = bytesToSkip - bytesAvailable;

                stream_->seekp(bytesAvailable, std::ifstream::cur);
                WriteNullBytes(bytesNeeded);
            }
            else
                stream_->seekp(bytesToSkip, std::ifstream::cur);
        }

        void WriteNullBytes(size_t bytesToWrite)
        {
            // Todo: See if quicker to allocate array of zeros and use WriteFromMemory
            for (size_t i = 0; i < bytesToWrite; i++)
                WriteUint8(0);
        }

#pragma region Alignment
        // Static method for calculating alignment pad from pos and alignment. Does not change position since static
        static size_t CalcAlign(size_t position, size_t alignmentValue = 2048)
        {
            const size_t remainder = position % alignmentValue;
            size_t paddingSize = remainder > 0 ? alignmentValue - remainder : 0;
            return paddingSize;
        }

        // Aligns stream to alignment value. Returns padding byte count
        size_t Align(size_t alignmentValue = 2048)
        {
            const size_t paddingSize = CalcAlign(stream_->tellp(), alignmentValue);
            Skip(paddingSize);
            return paddingSize;
        }
#pragma endregion

#pragma region Position and Length
        size_t Position() const
        {
            return stream_->tellp();
        }

        size_t Length()
        {
            // Save current position
            size_t realPosition = Position();

            // Seek to end of file and get position (the length)
            stream_->seekp(0, std::ios::end);
            size_t endPosition = Position();

            // Seek back to real pos and return length
            if (realPosition != endPosition)
                SeekBeg(realPosition);

            return endPosition;
        }
#pragma endregion

    private:
        std::ostream *stream_ = nullptr;
        MemoryBuffer *buffer_ = nullptr;
    };
}
