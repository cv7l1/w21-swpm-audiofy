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

    IFACEMETHODIMP OnFileOk(IFileDialog* dialog);
    IFACEMETHODIMP OnFolderChange(IFileDialog *) { return S_OK; };
    IFACEMETHODIMP OnFolderChanging(IFileDialog *, IShellItem *) { return S_OK; };
    IFACEMETHODIMP OnHelp(IFileDialog *) { return S_OK; };
    IFACEMETHODIMP OnSelectionChange(IFileDialog *) { return S_OK; };
    IFACEMETHODIMP OnShareViolation(IFileDialog *, IShellItem *, FDE_SHAREVIOLATION_RESPONSE *) { return S_OK; };
    IFACEMETHODIMP OnTypeChange(IFileDialog *pfd);
    IFACEMETHODIMP OnOverwrite(IFileDialog *, IShellItem *, FDE_OVERWRITE_RESPONSE *) { return S_OK; };

    IFACEMETHODIMP OnItemSelected(IFileDialogCustomize *pfdc, DWORD dwIDCtl, DWORD dwIDItem);
    IFACEMETHODIMP OnButtonClicked(IFileDialogCustomize *, DWORD) { return S_OK; };
    IFACEMETHODIMP OnCheckButtonToggled(IFileDialogCustomize *, DWORD, BOOL) { return S_OK; };
    IFACEMETHODIMP OnControlActivating(IFileDialogCustomize *, DWORD) { return S_OK; };

    OpenFileItemDialog(std::function<FileItem()> onAccept);

    void setFileSelectedCallback(std::function<FileItem()> callback) {_callback = callback;}
    void show();
private:

    long _cRef;
    IFileDialog* fileDialog;
    std::function<void(IShellItem*)> _callback;

    ~OpenFileItemDialog() {};
};

#endif //AUDIOLIB_EXAMPLES1_AY_FILEMANAGER_H
