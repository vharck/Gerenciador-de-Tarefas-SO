#ifdef _WIN32
#include <windows.h>
#include <commdlg.h>
#endif

#include <string>
#include <iostream>

using namespace std;

string openFileDialog() {
#ifdef _WIN32
    char filename[MAX_PATH];
    ZeroMemory(filename, sizeof(filename));

    OPENFILENAMEA ofn;
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    ofn.lpstrFilter = "Arquivos de Texto\0*.txt\0Todos os Arquivos\0*.*\0";
    ofn.lpstrFile = filename;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
    ofn.lpstrTitle = "Selecione o arquivo de tarefas";
    ofn.lpstrDefExt = "txt";

    if (GetOpenFileNameA(&ofn)) {
        return string(filename);
    }
#endif
    cout << ">> Selecao cancelada, buscando ./tarefas.txt padrao..." << endl;
    return "./tarefas.txt";
}