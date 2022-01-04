//
// Created by Jonathan on 27.11.2021.
//
#pragma once
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN

class VorbisDecoderFileApi;

#endif
#pragma comment(lib, "runtimeobject.lib")

#include <sal.h>
#include "types.h"
#include<mfidl.h>
#include<mfreadwrite.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mmdeviceapi.h>
#include <functiondiscoverykeys_devpkey.h>
#include <wrl/client.h>
#include <xaudio2.h>
#include <utility>
#include <xtr1common>
#include <type_traits>
#include "vorbisfile.h"
#include <optional>
#include <vector>
#include <memory>
#include <functional>

