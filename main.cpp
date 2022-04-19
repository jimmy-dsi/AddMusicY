#include "global.h"

#include "byte.h"
#include "music.h"
#include "mml.h"

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <cstdio>
#include <vector>

//#include <cmath>

using namespace std;

int main()
{
    cout << "Specify MML file: ";
    string file;
    cin >> file;

    vector<string> files;
    files.push_back(file);

    // Before we do anything, check for missing files.
    for (unsigned i = 0; i < files.size(); i++)
    {
        ifstream fin;
        fin.open(files[i].c_str());

        if (!fin.is_open())
        {
            cout << "Error: Cannot locate the file: \"" << files[i] << "\"" << endl;
            return 0;
        }

        fin.close();
    }

    // Now we can start writing shit
    for (unsigned i = 0; i < files.size(); i++)
    {
        Music music(START_OFFSET, ARAM_START);

        Mml mml;
        mml.load_file(files[i]);
        
        cout << endl << "Beginning preprocessing phase..." << endl;

        if (mml.preprocess()) // Before anything, delete comments and expand macros. Don't interpret the data if there's an error here.
        {
            cout << "Success!" << endl << endl;
        
            //Scale scale(7, -4, 0, WHOLE_TONE, 1, 7);
            //cout << "Testing %scale macro: " << scale.macro() << endl;

            //cout << "Starting PC offset: 0x" << setfill('0') << hex << uppercase << setw(8) << music.offsetPC << endl
            //     << "Starting ARAM offset: 0x" << setfill('0') << hex << uppercase << setw(4) << music.offsetARAM << endl << endl;
            
            cout << "Inserting music data..." << endl;
                 
            byte* hexData = NULL;

            if (mml.interpret(music))
            {
                hexData = music.get_entire_data();
                
                cout << "Success!" << endl;
            
                // Test music data
                //for (unsigned j = 0; j < music.total_size(); j++)
                //{
                //    cout << setfill('0') << hex << uppercase << setw(2) << (unsigned)hexData[j] << ' ';
                //}

                //cout << endl << endl;

                //cout << "Ending PC offset: 0x" << setfill('0') << hex << uppercase << setw(8) << music.offsetPC << endl
                //     << "Ending ARAM offset: 0x" << setfill('0') << hex << uppercase << setw(4) << music.offsetARAM << endl;
            }

            FILE* pFile;
            pFile = fopen("mml.bin", "wb");
            
            fwrite(hexData, 1, music.total_size(), pFile);

            fclose(pFile);
        }
    }

    return 0;
}

