//
// Created by Jonathan on 28.11.2021.
//

#ifndef AUDIOFY_LIB_AL_FILE_H
#define AUDIOFY_LIB_AL_FILE_H

#include "al_util.h"
#include "win32_framework.h"
#include "al_player.h"
#include <map>
enum AudioFileType {
    PCM,
    WMF,
    OGG,
};

class IAudioFile {
public:
    virtual double getLengthSeconds() = 0;
    virtual u32 getSampleRate() = 0;
    virtual const wchar_t* getPath() = 0;
    virtual const wchar_t* getExtension() = 0;

    virtual u64 getSampleCount() = 0;
    virtual AudioFileType getFileType() = 0;
};

class VorbisAudio {
public:
    class VorbisAudioFile : public IAudioFile {
    public:
        explicit VorbisAudioFile(_In_z_ const wchar_t* path,
                                 OggVorbis_File file,
                                 _In_ vorbis_info* info,
                                 u64 sampleCount,
                                 double seconds) :
                _path(path),
                _file(file),
                _info(info),
                _sampleCount(sampleCount),
                _seconds(seconds) {}

        double getLengthSeconds() override {return _seconds;}
        const wchar_t* getPath() override {return _path;}

        //TODO: IMPLEMENT
        const wchar_t* getExtension() override {return _path;}

        u64 getSampleCount() override {return _sampleCount;}
        u32 getSampleRate() override {return _info->rate;}
        AudioFileType getFileType() override {return AudioFileType::OGG;}

        OggVorbis_File _file;
    private:
        const wchar_t* _path;
        vorbis_info* _info;
        u64 _sampleCount = 0;
        double _seconds = 0;
    };

public:
    VorbisAudioFile* openFile(_In_z_ const wchar_t* path);

    void decodeFile(_Inout_ AudioPlayBuffer<>& buffer,
                           _In_ VorbisAudioFile* file);

    explicit VorbisAudio(_In_z_ const wchar_t* libPath = L"vorbisfile.dll") : _dll(DllHelper(libPath)),

    ov_open_callbacks(_dll["ov_open_callbacks"]),
    ov_info(_dll["ov_info"]),
    ov_pcm_total(_dll["ov_pcm_total"]),
    ov_read(_dll["ov_read"]),
    ov_pcm_seek(_dll["ov_pcm_seek"]),
    ov_raw_seek(_dll["ov_raw_seek"]),
    ov_clear(_dll["ov_clear"]),
    ov_time_total(_dll["ov_time_total"]){}


private:
    AudioFormatInfo<i16> formatInfo;
    DllHelper _dll;
    decltype(ov_open_callbacks)* ov_open_callbacks;
    decltype(ov_info)* ov_info;
    decltype(ov_pcm_total)* ov_pcm_total;
    decltype(ov_read)* ov_read;
    decltype(ov_pcm_seek)* ov_pcm_seek;
    decltype(ov_raw_seek)* ov_raw_seek;
    decltype(ov_clear)* ov_clear;
    decltype(ov_time_total)* ov_time_total;
};

class WmfAudio {
public:
    class WmfAudioFile: public IAudioFile{
    public:
        WmfAudioFile(_In_z_ const wchar_t* path,
                     _In_ IMFSourceReader* sourceReader,
                     _In_ IMFMediaType* mediaType,
                     _In_ WAVEFORMATEX* wf)
                    :
                    _path(path),
                    _reader(sourceReader),
                    _mediaType(mediaType),
                    _waveFormat(wf) {}

        double getLengthSeconds() override;

        const wchar_t *getPath() override {return _path;}

        const wchar_t *getExtension() override {return _path;}

        u64 getSampleCount() override;
        u32 getSampleRate() override {return _waveFormat->nSamplesPerSec;}
        AudioFileType getFileType() override;
        IMFSourceReader* getReader() {return _reader.Get();}
        WAVEFORMATEX* getWF() {return _waveFormat;}

    private:
        const wchar_t* _path;
        Microsoft::WRL::ComPtr<IMFSourceReader> _reader;
        Microsoft::WRL::ComPtr<IMFMediaType> _mediaType;
        WAVEFORMATEX* _waveFormat;
    };

    WmfAudio();
    WmfAudioFile* openFile(_In_z_ const wchar_t* path);
    void decodeFile(_Inout_ AudioPlayBuffer<>& buffer, WmfAudioFile* file);

private:

};
class AudioDecoder {
public:
    AudioDecoder() = default;
    IAudioFile* loadAudioFile(_In_z_ const wchar_t* path);
    void decodeAudioFile(_In_ IAudioFile* file, AudioPlayBuffer<>& buffer);
private:
    std::map<std::wstring, AudioFileType> fileTypes =
            {{L"wma", AudioFileType::WMF}, {L"wmv", AudioFileType::WMF},{L"wma", AudioFileType::WMF},
            {L"asf", AudioFileType::WMF}, {L"aac", AudioFileType::WMF},{L"wav", AudioFileType::WMF},
            {L"mp4", AudioFileType::WMF}, {L"mp3", AudioFileType::WMF}, {L"ogg", AudioFileType::OGG}};

    std::unique_ptr<VorbisAudio> vorbis = nullptr;
    std::unique_ptr<WmfAudio> wmf = nullptr;
};
class DebugWaveFileWriter {
public:
    static void writeBufferToWaveFile(_In_z_ const wchar_t* destPath,
                                      AudioPlayBuffer<>& buffer);
private:
};
class AudioCodec {

};

#endif //AUDIOFY_LIB_AL_FILE_H
