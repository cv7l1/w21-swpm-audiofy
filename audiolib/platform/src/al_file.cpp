//
// Created by Jonathan on 28.11.2021.
//

#include <Shlwapi.h>
#include <io.h>
#include "al_file.h"
#include "al_debug.h"
#include "win32_framework.h"
#include<string>
#include<format>
#include<array>
void winFileHandleToCFileHandle(_Inout_ _Notnull_ HANDLE *winFileHandle,
                                         _Outptr_ FILE **cFileHandle) {
    if(!winFileHandle) {throw std::exception("Invalid param");}
    int cDesc = _open_osfhandle(reinterpret_cast<intptr_t>(*winFileHandle), 0);
    if(cDesc == -1) {throw Win32Exception(GetLastError());}
    *cFileHandle = _fdopen(cDesc, "r+b");
    if(*cFileHandle == nullptr) {throw Win32Exception(GetLastError());}
}
enum AudioDecoderType {
    Wav,
    Vorbis,
    Wmf,
    ffmpeg
};

VorbisAudio::VorbisAudioFile* VorbisAudio::openFile(_In_z_ const wchar_t *path) {
    OggVorbis_File vorbisFile;
    al_ErrorInfo("Opening vorbis file...");
    al_ErrorInfo("Opening file");
    HANDLE winFileHandle = CreateFileW(path, GENERIC_READ | GENERIC_WRITE,
                                       FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING,
                                       FILE_ATTRIBUTE_NORMAL, nullptr);
    if(winFileHandle == INVALID_HANDLE_VALUE) {throw IOError(path);}
    FILE* cFile;
    winFileHandleToCFileHandle(&winFileHandle, &cFile);
    al_ErrorInfo("Done");
    al_ErrorInfo("Init vorbis file");
    int oggResult = ov_open_callbacks(cFile, &vorbisFile, nullptr, 0, OV_CALLBACKS_NOCLOSE);
    if(oggResult < 0) {
        fclose(cFile);
        throw IOError(path);
    }
    al_ErrorInfo("Done");
    double seconds = ov_time_total(&vorbisFile, -1);
    u64 sampleCount = ov_pcm_total(&vorbisFile, -1);
    al_ErrorInfo(std::format("Vorbis File: {} sec, {} sample Count", seconds, sampleCount).c_str());

    vorbis_info* info = ov_info(&vorbisFile, -1);
    al_ErrorInfo(std::format("Vorbis File: {} sample Rate, {} channels", info->rate, info->channels).c_str());
    return new VorbisAudioFile(path, vorbisFile, info, sampleCount, seconds);

}

void VorbisAudio::decodeFile(AudioPlayBuffer<> &buffer, _In_ VorbisAudioFile* file) {
    al_ErrorInfo("Vorbis: Start decoding");
    size_t blockSize = 4096;
    bool eof = false;
    buffer.getRawData().resize(file->getSampleCount() * 2, 0);

    int currentSection;
    u64 bytesRead = 0;
    while(!eof) {
        u64 ret = ov_read(&file->_file, reinterpret_cast<char *>(buffer.getRawData().data() + (bytesRead / sizeof(i16))), blockSize, 0, 2, 1, &currentSection);
        if(ret == 0) {eof = true;
            al_ErrorInfo("EOF");}
        else if(ret == OV_HOLE) {al_ErrorWarn("OV_HOLE ERROR"); break;}
        else if(ret == OV_EBADLINK) { al_ErrorWarn("OV_EBADLINK ERROR"); break;}
        else {bytesRead += ret;}
    }
    al_ErrorInfo(std::format("Bytes read: {}", bytesRead).c_str());
    buffer.getRawData().resize(bytesRead / sizeof(i16), 0);
    AudioFormatInfo<i16> newInfo;

    newInfo.sampleRate = file->getSampleRate();
    newInfo.bitsPerSample = 16;
    newInfo.numberOfChannels = 2;
    al_ErrorInfo("Finished Decoding");
    buffer.setAudioFormat(newInfo);

}


WmfAudio::WmfAudio() {
    throwIfFailed(MFStartup(MF_VERSION));
}

WmfAudio::WmfAudioFile* WmfAudio::openFile(_In_z_ const wchar_t* path) {
    IMFSourceReader* reader = nullptr;
    IMFMediaType* partialType = nullptr;
    IMFMediaType* mediaType = nullptr;
    WAVEFORMATEX* waveFormat = nullptr;
    al_ErrorInfo("Load wmf source reader");
    HRESULT result = MFCreateSourceReaderFromURL(path, nullptr, &reader);
    if(FAILED(result)) {
        throw IOError(path);
    }
    al_ErrorInfo("Done");
    al_ErrorInfo("Set reader specs");
    throwIfFailed(reader->SetStreamSelection(MF_SOURCE_READER_ALL_STREAMS, false));
    throwIfFailed(reader->SetStreamSelection(MF_SOURCE_READER_FIRST_AUDIO_STREAM, true));

    throwIfFailed(MFCreateMediaType(&partialType));

    //NOTE: I'll add error handling proper later. Usually nothing bad should happen here...
    MFCreateMediaType(&partialType);
    partialType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
    partialType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM);
    reader->SetCurrentMediaType(MF_SOURCE_READER_FIRST_AUDIO_STREAM,
                                nullptr, partialType);
    reader->GetCurrentMediaType(MF_SOURCE_READER_FIRST_AUDIO_STREAM, &mediaType);
    reader->SetStreamSelection(MF_SOURCE_READER_FIRST_AUDIO_STREAM, TRUE);
    al_ErrorInfo("Done");
    size_t waveSize = 0;
    throwIfFailed(MFCreateWaveFormatExFromMFMediaType(mediaType,
                                                 reinterpret_cast<WAVEFORMATEX **>(&waveFormat),
                                                      reinterpret_cast<UINT32 *>(&waveSize),
                                                 MFWaveFormatExConvertFlag_Normal));
    al_ErrorInfo("Done");
    return new WmfAudioFile(path, reader, mediaType, waveFormat);
}

void WmfAudio::decodeFile(AudioPlayBuffer<>& buffer, WmfAudioFile* file) {
    al_ErrorInfo("Wmf decode");
    IMFMediaBuffer* dataBuffer = nullptr;
    IMFSample* sample = nullptr;

    bool endOfStream = false;

    while(!endOfStream) {
        DWORD flags;
        throwIfFailed(file->getReader()->ReadSample(
                MF_SOURCE_READER_FIRST_AUDIO_STREAM,
                0,
                nullptr,
                &flags,
                nullptr,
                &sample
        ));
        endOfStream = flags & MF_SOURCE_READERF_ENDOFSTREAM;
        if(endOfStream) {break;}
        throwIfFailed(sample->ConvertToContiguousBuffer(&dataBuffer));

        DWORD bufferReadCount = 0;
        i16* audioData = nullptr;

        throwIfFailed(dataBuffer->Lock(reinterpret_cast<BYTE **>(&audioData), nullptr, &bufferReadCount));

        //TODO: Handle other cases

        buffer.getRawData().insert(buffer.getRawData().end(), &audioData[0], &audioData[bufferReadCount / sizeof(i16)]);
        throwIfFailed(dataBuffer->Unlock());

        sample->Release();
        dataBuffer->Release();
    }
    auto decodedAudioFormat = AudioFormatInfo<>::PCMDefault();

    decodedAudioFormat.sampleRate = file->getWF()->nSamplesPerSec;
    decodedAudioFormat.numberOfChannels = file->getWF()->nChannels;
    decodedAudioFormat.bitsPerSample = file->getWF()->wBitsPerSample;

    buffer.setAudioFormat(decodedAudioFormat);
    al_ErrorInfo("Done");
}

double WmfAudio::WmfAudioFile::getLengthSeconds() {
    PROPVARIANT prop;
    throwIfFailed(_reader->GetPresentationAttribute(MF_SOURCE_READER_MEDIASOURCE, MF_PD_DURATION, &prop));

    const MFTIME ONE_SECOND = 10000000;
    const LONG   ONE_MSEC = 1000;

    double duration = (double) prop.uhVal.QuadPart / ONE_SECOND;
    return duration;
}

u64 WmfAudio::WmfAudioFile::getSampleCount() {
    return getLengthSeconds() * _waveFormat->nSamplesPerSec;
}

AudioFileType WmfAudio::WmfAudioFile::getFileType() {
    return WMF;
}

IAudioFile* AudioDecoder::loadAudioFile(const wchar_t *path) {
    al_ErrorInfo("Loading audio file");
    if(PathIsDirectoryW(path) || !PathFileExistsW(path)) {
        al_ErrorWarn("Invalid file");
        throw IOError(path);
    }

    wchar_t* pathPtr = PathFindExtensionW(path);
    if(pathPtr[0] == L'\0') {throw IOError(path);}
    auto ext = std::wstring(pathPtr + 1);
    AudioFileType fileType = fileTypes.at(ext);

    if(fileType == AudioFileType::OGG) {
        al_ErrorInfo("Got filetype: Vorbis");
        if(vorbis == nullptr) {vorbis = std::make_unique<VorbisAudio>();}
        return vorbis->openFile(path);
    } else if(fileType == AudioFileType::WMF) {
        al_ErrorInfo("Got filetype: WMF");
        if(wmf == nullptr) {wmf = std::make_unique<WmfAudio>();}
        return wmf->openFile(path);
    }
}

void AudioDecoder::decodeAudioFile(IAudioFile *file, AudioPlayBuffer<>& buffer) {
    al_ErrorInfo("Decode");
    if(file->getFileType() == AudioFileType::WMF) {
        al_ErrorInfo("Got filetype: WMF");
        if(wmf == nullptr) {wmf = std::make_unique<WmfAudio>();}
        wmf->decodeFile(buffer, dynamic_cast<WmfAudio::WmfAudioFile *>(file));
    } else if(file->getFileType() == AudioFileType::OGG) {
        al_ErrorInfo("Got filetype: Vorbis");
        try {
            if(vorbis == nullptr) {vorbis = std::make_unique<VorbisAudio>();}
        } catch(std::exception &e) {

        }

        vorbis->decodeFile(buffer, dynamic_cast<VorbisAudio::VorbisAudioFile *>(file));
    }
}

void DebugWaveFileWriter::writeBufferToWaveFile(_In_z_ const wchar_t *destPath, AudioPlayBuffer<> &buffer) {
    u32 byteRate = buffer.getAudioFormat().sampleRate * buffer.getAudioFormat().numberOfChannels * 2;
    u16 blockAlign = buffer.getAudioFormat().numberOfChannels * 2;
    u16 bitsPerSample = 16;
    u32 chunkSize = 16;
    u16 pcmTag = 1;
    u32 riffSize = 36 + (buffer.getRawData().size() * 2);
    u16 numChannels = 2;
    u32 sampleRate = buffer.getAudioFormat().sampleRate;
    u32 bufferSize = buffer.getRawData().size() * 2;
    HANDLE fileHandle = CreateFileW(destPath, GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    if(fileHandle == INVALID_HANDLE_VALUE) {throw IOError(destPath);}
    WriteFile(fileHandle, "RIFF", 4, NULL, NULL);
    WriteFile(fileHandle, reinterpret_cast<LPCVOID>(&riffSize), sizeof(u32), NULL, NULL);
    WriteFile(fileHandle, "WAVE", 4, NULL, NULL);
    WriteFile(fileHandle, "fmt ", 4, NULL, NULL);
    WriteFile(fileHandle, &chunkSize, 4, NULL, NULL);
    WriteFile(fileHandle, &pcmTag, 2, NULL, NULL);
    WriteFile(fileHandle, &numChannels, 2, NULL, NULL);
    WriteFile(fileHandle, &sampleRate, 4, NULL, NULL);
    WriteFile(fileHandle, &byteRate, 4, NULL, NULL);
    WriteFile(fileHandle, &blockAlign, 2, NULL, NULL);
    WriteFile(fileHandle, &bitsPerSample, 2, NULL, NULL);
    WriteFile(fileHandle, "data", 4, NULL, NULL);
    WriteFile(fileHandle, &bufferSize, 4, NULL, NULL);
    WriteFile(fileHandle, buffer.getRawData().data(), buffer.getRawData().size() * 2, NULL, NULL);

    CloseHandle(fileHandle);
}
