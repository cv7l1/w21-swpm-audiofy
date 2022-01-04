//
// Created by Jonathan on 30.12.2021.
//

#include "ay_fileManager.h"
#include "al_debug.h"
#include "al_error.h"

void ciUtil(REFIID riid, void **ppv) {

}

OpenFileItemDialog::OpenFileItemDialog(std::function<void(FileItem)> onAccept) : _cRef(1), _callback(onAccept)  {
    CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    throwIfFailed(CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&this->fileDialog)));

    throwIfFailed(fileDialog->Advise(this, &_cookie));

    DWORD flags;
    throwIfFailed(fileDialog->GetOptions(&flags));
    throwIfFailed(fileDialog->SetOptions(flags | FOS_FORCEFILESYSTEM));

    //TODO: Limit to audio files only
    throwIfFailed(fileDialog->SetDefaultExtension(L"wav"));

}

void OpenFileItemDialog::show() {
    auto result = fileDialog->Show(nullptr);
}

IFACEMETHODIMP OpenFileItemDialog::OnFileOk(IFileDialog *dialog) {
    IShellItem* ret;
    auto result = dialog->GetResult(&ret);
    if(FAILED(result)) {return S_FALSE;}

    _callback(FileItem(ret));

    return S_OK;
}

std::optional<FileItem> OpenFileItemDialog::getResult() {
    IShellItem* ret;
    auto result = fileDialog->GetResult(&ret);
    if(FAILED(result)) {
        return std::nullopt;
    }

    return FileItem(ret);
}

