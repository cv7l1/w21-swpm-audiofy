//
// Created by Jonathan on 04.12.2021.
//

#include "playback_example.h"
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <assert.h>
#include "al_device.h"
#include "al_debug.h"
#include "al_player.h"
#include "al_file.h"
#include <cmath>
#include "gui_test.h"
#define M_PI 3.14159265358979323846

i16 sineWave(double time, double freq, double amplitude) {
    i16 result;
    double tpc = 44100 / freq;
    double cycles = time / tpc;
    double rad = M_PI * 2 * cycles;
    i16 _amp = 32767 * amplitude;
    result = _amp  * sin(rad);
    return result;
}

std::vector<i16> createSineWaveBuffer(size_t len) {
    std::vector<i16> vec;

    for(int i = 0; i<len - 1; i+=2) {
        //This should give us a middle C4 (for reference: https://www.youtube.com/watch?v=t8ZE1Mlkg2s)
        i16 currentSample = sineWave(i, 261.6, 0.9);
        vec.push_back(currentSample / 2);
        vec.push_back(currentSample / 2);
    }
    return vec;
}

int WINAPI WinMain(_In_ HINSTANCE hInstance,
                   _In_ HINSTANCE hPrevInstance,
                   _In_ PSTR lpCmdLine,
                   _In_ INT nChmdShow) {

    /*This will initialize some specific windows components and has to be called before using any audiolib functions.
     *In the future, this will be handled by the library.
    */
    CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    CreateThread(NULL, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(runGui), nullptr, 0, nullptr);
    /*
     * This will initialize the console logger. Only for debug purposes
     */
    Console::setup();

    /*This will initialize the audio device manager, through which we can recieve information
     * about all recognized audio devices.
     */
    AudioDeviceManager audioDeviceManager;

    /*
     * This will return the audio device set as 'default' for a specific role.
     */
    auto defaultDevice = audioDeviceManager.getDefaultDevice(AudioDeviceRole::Playback);

    /* Note that getDefaultDevice() may return a nullptr in which case no default device could be found
     * This usually means, that there are no device available for the given role.
     * This is not an error and does not throw an exception. This case should be handled by the user
     */
    if(!defaultDevice) {
        al_ErrorWarn("There is no usable speaker currently plugged in :(");
    }

    /*
     * The default device may have been unplugged or disabled in some other way. This is something
     * that the user of the library should always watch out for and handle
     */
    u32 deviceState = defaultDevice->getCurrentDeviceState();
    if(deviceState == DEVICE_STATE_ACTIVE) {
        al_ErrorInfo("Device ready and working! :)");
    } else {
        al_ErrorWarn("Device is not plugged in");
    }

    /* This will initialize our audio player. We can optionally pass a playback device
     * but we don't have to. If we don't, it will use the default device or throw
     * an exception if no valid device was found
     * We can also tell the audio player to use debug functionality, which enables
     * some asserts and debug output. By default, this is set so false. Please note,
     * that this may impact performance in a negative way.
     * We may also want to specify which audio format the player is going to use by default.
     * If nothing is specified, it's going to use 44.1khz, 16 bit samples and 2 channels.
     * When the player is destroyed, all buffers currently playing will be stopped
     * and no futher buffers can be submitted.
     * Please ensure that only ONE audioPlayer is active during runtime.
     * If you want to construct multiple audio players (which you really shouldn't...)
     * please make sure that all other instances have been destroyed.
     */
    AudioPlayer audioPlayer(true);

    /*
     * We can set a callback, which the audio player is going to call if a critical error
     * occurs. If such an error is raised, the player is going to be restarted.
     * NOTE: I have not tested this yet. A critical error may result in an endless loop
     * or memory leaks.
     * The callback is called, before the engine restarts.
     */
    audioPlayer.setErrorCallback([]() { al_ErrorWarn("Critical error, restarting the audio player");});

    /*
     * This is the default audio format with 44.1khz, 16 bits per sample
     * and 2 channels.
     */
    auto defaultAudioFormat = AudioFormatInfo<i16>::PCMDefault();

    u32 channels = defaultAudioFormat.numberOfChannels;
    assert(channels == 2);

    auto sampleRate = defaultAudioFormat.sampleRate;
    assert(sampleRate == 44100);

    /*
     * An AudioPlayBuffer can hold a buffer, containing raw pcm data. Currently
     * only 16 bit samples are really supported, but you should not encounter any other, weird
     * pcm audio formats. Internally the raw data is stored in a vector, so the initial audio size
     * may be set to any value really (Though you can avoid some reallocs by setting a reasonable, initial
     * size).
     * By default, the start cursor is set to play the first sample in the buffer, the playtime is the
     * size of the entire buffer. By changing these values, the contents of the buffer are not changed
     * The AudioFormat can and will be changed, note however that you have to resubmit the
     * buffer for any changes to take effect
     */

    //This should be enough to hold 2 secs of audio data
    u32 bufferSize = defaultAudioFormat.sampleRate * sizeof(i16) * 2;

    AudioPlayBuffer sineAudioBuffer(defaultAudioFormat, 0);
    /*
     * By setting this flag to true, we will tell the audio player to ignore the
     * playCursor and playLength values when submitting the data.
     */
    sineAudioBuffer.setPlayFullBuffer(true);

    //Copies our sine data to the buffer
    auto sinePCMAudioBuffer = createSineWaveBuffer(bufferSize);
    sineAudioBuffer.getRawData() = sinePCMAudioBuffer;

    /*
     * This will submit our buffer to the player and optionally start playing from it.
     * Note that the contents of the buffer are COPIED, you can reuse or clear the initial buffer
     * right away. This also means that if you make any changes to the buffer, you will have to
     * resubmit again. DO NOT submit buffers frequently. This is a very costly operation.
     * The player contains it's own soundBuffer (henceforth: Frontbuffer). PlayAudioBuffer
     * may change properties like the sample rate of the frontbuffer if needed. In the future, there will
     * be some method that will allow you to copy raw data without any other changes
     * being made. Right now you can set the start parameter to false and adjust a
     * audio info data before you start playing if you want to.
     * If a buffer is currently playing, playback will be stopped when submitting a buffer and (optionally) resumed
     * You can't change the data of a buffer while it's playing.
     */
    audioPlayer.playAudioBuffer(sineAudioBuffer, true);

    /*
     * Wait until the buffer has finished playing
     */
    al_ErrorInfo("Waiting...");
    WaitForSingleObjectEx(audioPlayer.bufferEndEvent, INFINITE, FALSE);
    al_ErrorInfo("Done!");

    //We can puase/stop playback at all times
    audioPlayer.pause();

    //Let's also play an audio file

    /*
     * This WILL Change.
     * Constructing a decoder will also load all of the required dependencies. Right now, if there is
     * an error while loading the libs, an exception will be raised.
     * However there is no fallback and thus, you will not be able to use the decoder, if the vorbis
     * deps can't be loaded (for whatever reason)
     */
    AudioDecoder decoder;

    //Path is always releative to exe right now. This will change in the future
    //auto audioFile = decoder.loadAudioFile(L"../../../example_playback/song.mp3");
    auto audioFile = decoder.loadAudioFile(L"song.mp3");

    AudioPlayBuffer songBuffer;
    decoder.decodeAudioFile(audioFile, songBuffer);

    //We can also set how much of the buffer to play
    songBuffer.setPlayFullBuffer(false);

    //Start at ca 10secs
    songBuffer.setPlayCursor(10 * songBuffer.getAudioFormat().sampleRate); //Start 5 secs in

    //Play ca 5 secs
    songBuffer.setPlayLength(5 * songBuffer.getAudioFormat().sampleRate); //Play 5 secs

    songBuffer.setLoopCount(0);
    songBuffer.setLoop(false);

    audioPlayer.playAudioBuffer(songBuffer, false);

    //Let's lower the volume
    audioPlayer.setMasterVolume(0.5);

    //Start playing
    audioPlayer.play();
    //Let's also write the decoded audio file to harddrive
    DebugWaveFileWriter::writeBufferToWaveFile(L"test.wav", songBuffer);

    for(;;) {
        Sleep(5000);
    }
    return 1;
}