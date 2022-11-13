#include <windows.h>
#include <shlobj.h>
#include <shellapi.h>
#include <tchar.h>
#include <Shobjidl_core.h>

// Ripped from https://www.dirkbertels.net/computing/managedCpp.php
// This simple function takes a path and gets a shell UI object from it. 
// We convert the path to a pidl with SHParseDisplayName, then bind to the 
// pidl's parent with SHBindToParent, then ask the parent for the UI object 
// of the child with IShellFolder::GetUIObjectOf
HRESULT GetUIObjectOfFile(HWND hwnd, LPCWSTR pszPath, REFIID riid, void** ppv)
{
    *ppv = NULL;
    HRESULT hr;
    LPITEMIDLIST pidl;
    SFGAOF sfgao;
    // Translates a Shell namespace object's display name into an item identifier list 
    // and returns the attributes of the object. This function is the preferred method 
    // to convert a string to a PIDL.
    hr = SHParseDisplayName(pszPath, NULL, &pidl, SFGAO_CANCOPY, &sfgao);
    if (SUCCEEDED(hr)) {
        IShellFolder* psf;
        LPCITEMIDLIST pidlChild;
        if (SUCCEEDED(hr = SHBindToParent(pidl, IID_IShellFolder, (void**)&psf, &pidlChild))) {
            hr = psf->GetUIObjectOf(hwnd, 1, &pidlChild, riid, NULL, ppv);
            psf->Release();
        }
        CoTaskMemFree(pidl);
    }
    return hr;
}

// Ripped from https://devblogs.microsoft.com/oldnewthing/20080724-00/?p=21483
int __cdecl wmain(int argc, WCHAR** argv)
{
    if (argc == 3 && SUCCEEDED(CoInitialize(NULL))) {
        IDataObject* pdto;
        if (SUCCEEDED(GetUIObjectOfFile(NULL, argv[1],
            IID_IDataObject, (void**)&pdto))) {
            IDropTarget* pdt;
            if (SUCCEEDED(GetUIObjectOfFile(NULL, argv[2],
                IID_IDropTarget, (void**)&pdt))) {
                POINTL pt = { 0, 0 };
                DWORD dwEffect = DROPEFFECT_COPY | DROPEFFECT_LINK;
                if (SUCCEEDED(pdt->DragEnter(pdto, MK_LBUTTON,
                    pt, &dwEffect))) {
                    dwEffect &= DROPEFFECT_COPY | DROPEFFECT_LINK;
                    if (dwEffect) {
                        pdt->Drop(pdto, MK_LBUTTON, pt, &dwEffect);
                    }
                    else {
                        pdt->DragLeave();
                    }
                }
                pdt->Release();
            }
            pdto->Release();
        }
        CoUninitialize();
    }
    return 0;
}