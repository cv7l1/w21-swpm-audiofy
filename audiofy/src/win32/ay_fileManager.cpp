//
// Created by Jonathan on 30.12.2021.
//

#include "ay_fileManager.h"
#include "al_debug.h"
#include "al_error.h"

void ciUtil(REFIID riid, void **ppv) {

}

OpenFileItemDialog::OpenFileItemDialog(std::function<FileItem()> onAccept) : _cRef(1), _callback(onAccept)  {
    throwIfFailed(CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&this->fileDialog)));

    DWORD cookie;
    throwIfFailed(fileDialog->Advise(this, &cookie));

    DWORD flags;
    throwIfFailed(fileDialog->GetOptions(&flags));
    throwIfFailed(fileDialog->SetOptions(flags | FOS_FORCEFILESYSTEM));

    //TODO: Limit to audio files only
    throwIfFailed(fileDialog->SetDefaultExtension(L"wav"));
}

void OpenFileItemDialog::show() {
    throwIfFailed(fileDialog->Show(nullptr));
}

IFACEMETHODIMP OpenFileItemDialog::OnFileOk(IFileDialog *dialog) {
    IShellItem* ret;
    throwIfFailed(dialog->GetResult(&ret));
    _callback(FileItem(ret));
    return S_OK;
}

