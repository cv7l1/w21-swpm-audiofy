//
// Created by Jonathan on 30.12.2021.
//

#ifndef AUDIOLIB_EXAMPLES1_AY_FILEMANAGER_H
#define AUDIOLIB_EXAMPLES1_AY_FILEMANAGER_H
#include "types.h"
#include "al_io.h"
#include <ShlObj.h>
#include <objbase.h>      // For COM headers
#include <ShObjIdl.h>     // for IFileDialogEvents and IFileDialogControlEvents
#include <Shlwapi.h>
#include <KnownFolders.h> // for KnownFolder APIs/datatypes/function headers
#include <propvarutil.h>  // for PROPVAR-related functions
#include <propkey.h>      // for the Property key APIs/datatypes
#include <PropIdl.h>      // for the Property System APIs
#include <strsafe.h>      // for StringCchPrintfW
#include <shtypes.h>      // for COMDLG_FILTERSPEC
#include <functional>
#include <al_error.h>

class FileItem {
public:
    FileItem(IShellItem* shellItem) : _shellItem(shellItem) {};

    wchar_t* getFullFilePath() {
        wchar_t* ret = nullptr;
        throwIfFailed(_shellItem->GetDisplayName(SIGDN_FILESYSPATH, &ret));
        return ret;
    }
    wchar_t* getDisplayName() {
        wchar_t* ret = nullptr;
        throwIfFailed(_shellItem->GetDisplayName(SIGDN_NORMALDISPLAY, &ret));
        return ret;
    }
private:
    IShellItem* _shellItem;
};

class OpenFileItemDialog : public IFileDialogEvents {
public:
    IFACEMETHODIMP QueryInterface(REFIID riid, void** ppv) {
        static const QITAB qit[] = {
                QITABENT(OpenFileItemDialog, IFileDialogEvents ),
                {0},

        };
        return QISearch(this, qit, riid, ppv);
    };

    IFACEMETHODIMP_(ULONG) AddRef() {return InterlockedIncrement(&_cRef);}
    IFACEMETHODIMP_(ULONG) Release() {
        long cRef = InterlockedDecrement(&_cRef);
        if(!cRef)
            delete this;
        return cRef;
    }

    IFACEMETHODIMP OnFileOk(IFileDialog* dialog) override;
    IFACEMETHODIMP OnFolderChange(IFileDialog *) { return S_OK; };
    IFACEMETHODIMP OnFolderChanging(IFileDialog *, IShellItem *) { return S_OK; };
    IFACEMETHODIMP OnSelectionChange(IFileDialog *) { return S_OK; };
    IFACEMETHODIMP OnShareViolation(IFileDialog *, IShellItem *, FDE_SHAREVIOLATION_RESPONSE *) { return S_OK; };
    IFACEMETHODIMP OnTypeChange(IFileDialog *pfd) {return S_OK;}
    IFACEMETHODIMP OnOverwrite(IFileDialog *, IShellItem *, FDE_OVERWRITE_RESPONSE *) { return S_OK; };

    explicit OpenFileItemDialog(std::function<void(FileItem)> onAccept);

    void setFileSelectedCallback(std::function<void(FileItem)> callback) {_callback = callback;}
    void show();
    std::optional<FileItem> getResult();
    ~OpenFileItemDialog() {
        fileDialog->Unadvise(_cookie);
        fileDialog->Release();
    }

private:
    DWORD _cookie;
    long _cRef;
    IFileDialog* fileDialog;
    std::function<void(FileItem)> _callback;;
};

#endif //AUDIOLIB_EXAMPLES1_AY_FILEMANAGER_H
