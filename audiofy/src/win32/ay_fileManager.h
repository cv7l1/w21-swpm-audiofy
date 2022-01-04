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
#include <stdexcept>
class FileItem {
public:
    FileItem(IShellItem* shellItem) : _shellItem(shellItem) {
        if(!shellItem) {throw std::exception("Invalid shell item");}
        throwIfFailed(_shellItem->GetDisplayName(SIGDN_FILESYSPATH, &_sysPath));

        SHGetFileInfoW(_sysPath, 0, &_shellFileInfo, sizeof(SHFILEINFOW), SHGFI_DISPLAYNAME | SHGFI_TYPENAME );
        if(0 == GetFileAttributesExW(getFullFilePath(), GetFileExInfoStandard, &_fileAttributes)) {
            throw std::exception("Unable to retrieve file info");
        }
    };

    wchar_t* getFullFilePath() {
        return _sysPath;
    }
    wchar_t* getDisplayName() {
        return _shellFileInfo.szDisplayName;
    }
    wchar_t* getFileTypeDescription() {
        return _shellFileInfo.szTypeName;
    }
    float getFileSize() const {
        return ((((_fileAttributes.nFileSizeHigh << 16) | _fileAttributes.nFileSizeLow) / 1024.0f) / 1024.0f);
    }

private:
    wchar_t* _sysPath;
    SHFILEINFOW _shellFileInfo;

    WIN32_FILE_ATTRIBUTE_DATA _fileAttributes;

    size_t _shellFileInfoSize;
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

    explicit OpenFileItemDialog(std::function<HRESULT (FileItem)> onAccept);

    void setFileSelectedCallback(std::function<HRESULT(FileItem)> callback) {_callback = callback;}
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
    std::function<HRESULT(FileItem)> _callback;;
};

#endif //AUDIOLIB_EXAMPLES1_AY_FILEMANAGER_H
