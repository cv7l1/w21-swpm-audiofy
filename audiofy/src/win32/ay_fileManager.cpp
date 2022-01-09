//
// Created by Jonathan on 30.12.2021.
//

#include "ay_fileManager.h"
#include "al_debug.h"
#include "al_error.h"

Win32FileItemDialog::Win32FileItemDialog(std::function<HRESULT(FileItem)> onAccept, DialogType type) : _cRef(1), _callback(onAccept) , dialogType(type) {
    CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    throwIfFailed(CoCreateInstance(type == ayFile_Open ? CLSID_FileOpenDialog : CLSID_FileSaveDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&this->fileDialog)));
        
    throwIfFailed(fileDialog->Advise(this, &_cookie));

    if (type == DialogType::ayFile_Save) {
        const COMDLG_FILTERSPEC rgSaveTypes[] = { {L"Wave File (*.wav)", L"*.wav"} };
        throwIfFailed(fileDialog->SetFileTypes(ARRAYSIZE(rgSaveTypes), rgSaveTypes));
        throwIfFailed(fileDialog->SetFileTypeIndex(0));
        throwIfFailed(fileDialog->SetDefaultExtension(L"wav"));
    }

    DWORD flags;
    throwIfFailed(fileDialog->GetOptions(&flags));
    throwIfFailed(fileDialog->SetOptions(flags | FOS_FORCEFILESYSTEM));

    //TODO: Limit to audio files only
    //throwIfFailed(fileDialog->SetDefaultExtension(L"wav"));

}

void Win32FileItemDialog::show() {
    auto result = fileDialog->Show(nullptr);
}

IFACEMETHODIMP Win32FileItemDialog::OnFileOk(IFileDialog *dialog) {
    IShellItem* ret;
    auto result = dialog->GetResult(&ret);
    if(FAILED(result)) {return S_FALSE;}
    _callback(FileItem(ret));

    return S_OK;
}

std::optional<FileItem> Win32FileItemDialog::getResult() {
    IShellItem* ret;
    auto result = fileDialog->GetResult(&ret);
    if(FAILED(result)) {
        return std::nullopt;
    }

    return FileItem(ret);
}

