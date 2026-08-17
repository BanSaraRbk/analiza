#include "TF1.h"
#include "TRint.h"
#include "TCanvas.h"
#include "TFile.h"

int main(int argc, char **argv)
{
    TRint app("app", &argc, argv);

    std::cout << "Insert the name of the file to be opened: ";
    std::string filename;
    std::cin >> filename;
    TFile *file = TFile::Open(filename.c_str());

    std::cout << "File opened: " << file->GetName() << std::endl;

    return 0;
}