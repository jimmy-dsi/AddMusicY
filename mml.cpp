#include "byte.h"
#include "mml.h"

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <set>

using namespace std;

Mml::Mml()
{
    isError = false;

    data = "";

    // Set up macros

    // Nope!

    // Set up built-in variables

    variables.insert(pair<string, string>("h", "0"));
    variables.insert(pair<string, string>("i", "0"));
    variables.insert(pair<string, string>("j", "0"));
    variables.insert(pair<string, string>("k", "0"));
    variables.insert(pair<string, string>("l", "12"));
    variables.insert(pair<string, string>("m", "0,0,0"));
    variables.insert(pair<string, string>("n", "0,0,0"));
    variables.insert(pair<string, string>("o", "4"));
    variables.insert(pair<string, string>("p", "0,0"));
    variables.insert(pair<string, string>("q", "7F"));
    variables.insert(pair<string, string>("Q", "127"));
    variables.insert(pair<string, string>("t", ""));
    variables.insert(pair<string, string>("u", "0,0"));
    variables.insert(pair<string, string>("v", ""));
    variables.insert(pair<string, string>("w", ""));
    variables.insert(pair<string, string>("x", ""));
    variables.insert(pair<string, string>("y", ""));
    variables.insert(pair<string, string>("z", ""));
    variables.insert(pair<string, string>("chan", "0"));
    variables.insert(pair<string, string>("ins", ""));
    variables.insert(pair<string, string>("pb", ""));

    variables.insert(pair<string, string>("transpose", "0"));
    variables.insert(pair<string, string>("fine", "0"));
    variables.insert(pair<string, string>("songKey", "0"));
    variables.insert(pair<string, string>("chanKey", "0"));
    variables.insert(pair<string, string>("length", "12"));
    variables.insert(pair<string, string>("portaDelay", "0"));
    variables.insert(pair<string, string>("portaLength", "0"));
    variables.insert(pair<string, string>("portaNote", "0"));
    variables.insert(pair<string, string>("octave", "4"));
    variables.insert(pair<string, string>("vibDelay", "0"));
    variables.insert(pair<string, string>("vibFreq", "0"));
    variables.insert(pair<string, string>("vibDepth", "0"));
    variables.insert(pair<string, string>("legato", "7"));
    variables.insert(pair<string, string>("velocity", "15"));
    variables.insert(pair<string, string>("tempo", ""));
    variables.insert(pair<string, string>("tremDelay", "0"));
    variables.insert(pair<string, string>("tremFreq", "0"));
    variables.insert(pair<string, string>("tremDepth", "0"));
    variables.insert(pair<string, string>("chanVolume", ""));
    variables.insert(pair<string, string>("songVolume", ""));
    variables.insert(pair<string, string>("echoFlags", ""));
    variables.insert(pair<string, string>("echoRightVol", ""));
    variables.insert(pair<string, string>("echoLeftVol", ""));
    variables.insert(pair<string, string>("pan", ""));
    variables.insert(pair<string, string>("echoDelay", ""));
    variables.insert(pair<string, string>("echoFeedback", ""));
    variables.insert(pair<string, string>("echoFIR", ""));
    variables.insert(pair<string, string>("pbDelay", ""));
    variables.insert(pair<string, string>("pbLength", ""));
    variables.insert(pair<string, string>("pbNote", ""));

    variables.insert(pair<string, string>("seed", "0"));
    variables.insert(pair<string, string>("loopData", ""));
    
    // Pseudo-built-in variables
    
    userDefined.insert(pair<string, string>("_LAST_CONDITION_", "0"));
    
    listIDs.insert(make_pair(0, 0));
    userDefined.insert(pair<string, string>("_CONDITION_STACK_", "0"));
    listIDs.insert(make_pair(1, 0));
    userDefined.insert(pair<string, string>("_MACRO_STACK_", "1"));
    listIDs.insert(make_pair(2, 0));
    userDefined.insert(pair<string, string>("_ARG_LIST_", "2"));

    //

    isAssignment = false;
    isQuote = false;
    isEvalMacro = false;
    preAssignIndex = 0;
    assignLength = 0;
    isInQuotes = false;

    lineNum = 1;
    currentChan = -1;

    patternStart = 0x0000;
    isPatternDefined = false;
    
    sampleSlots = 0;
    sampleDataOffset = SAMPLE_DATA_ARAM;

    currentLabel = "";

    shouldLoop = true;
    hasIntro = false;

    command = '\0';
    octave = 4;
    transpose = 0;
    defaultLength = 16;
    prevTicks = 0xFF;
    dynamic = 0x7F;
    prevDynamic = 0xFF;
    isTriplet = false;
    isLoop = false;
}

Mml::~Mml()
{
    for (map<string, Macro*>::iterator it = macros.begin(); it != macros.end(); it++)
    {
        delete it->second;
    }
}

void Mml::load_file(string filename)
{
    ifstream fin;
    fin.open(filename.c_str());

    while (!fin.eof())
    {
        string fileLine;
        getline(fin, fileLine);
        data += fileLine + '\n';
    }

    fin.close();
}

bool Mml::preprocess()
{
    lineNum = 1;
    bool ret;

    remove_comments();
    replace();
    ret = add_defines();
    lineNum = 1;
    ret = ret && expand_macros(data);
    ret = ret && expand_kits();
    //print_variables(); // Let's test to see if the variables were written to properly.

    ofstream fout;
    fout.open("expand.mml");
    fout << data;
    fout.close();

    return ret;
}

bool Mml::interpret(Music& music)
{
    lineNum = 1;

    for (unsigned i = 0; i < data.size(); )
    {
        if (data[i] == '#')
        {
            enter_file(data, i);
        }
    
        char chr = tolower(data[i]);
        command = chr;

        if (chr == '\n')
        {
            lineNum++;
        }

        END_OF_FILE = "Unexpected end of file.";
        NO_VALUE = "No value specified for \'";
        NO_VALUE += command;
        NO_VALUE += "\'.";
        INVALID_VALUE = "Invalid value specified for \'";
        INVALID_VALUE += command;
        INVALID_VALUE += "\'.";

        switch (chr)
        {
            case '!':
            {
                string commandStr;

                if (!iterate(chr, i))
                {
                    print_error(END_OF_FILE);
                    return false;
                }
                update_line(chr);
                
                while (isalnum(chr) || chr == '_')
                {
                    commandStr += chr;
                    iterate(chr, i);
                }
                
                if (commandStr == "")
                {
                    print_error(NO_VALUE);
                    return false;
                }
                
                if (commandStr == "print")
                {
                    while (is_whitespace(chr))
                    {
                        commandStr += chr;
                        iterate(chr, i);
                        
                        update_line(chr);
                    }
                    
                    if (chr != '\"')
                    {
                        print_error("Print string must be placed in quotation marks.");
                    }
                    else
                    {
                        iterate(chr, i);
                        
                        string printStr;
                        
                        while (chr != '\"')
                        {
                            printStr += data[i];
                            
                            iterate(chr, i);
                            update_line(chr);
                        }
                        
                        cout << " - " << printStr << " -" << endl;
                    }
                }
                else
                {
                    print_error(INVALID_VALUE);
                    return false;
                }
            
                break;
            }
        
            case '#':
            {
                string defineStr;

                if (!iterate(chr, i))
                {
                    print_error(END_OF_FILE);
                    return false;
                }
                update_line(chr);

                while (!is_whitespace(chr))
                {
                    defineStr += chr;
                    iterate(chr, i);
                }

                if (defineStr == "")
                {
                    print_error(NO_VALUE);
                    return false;
                }

                if (defineStr[0] >= '0' && defineStr[0] <= '9')
                {
                    // Don't write byte 0x00 if it's a continuation track.
                    if (currentChan >= 0 && !(defineStr.size() > 9 && defineStr.substr(defineStr.size() - 9) == "=continue"))
                    {
                        music.write(isLoop, 0x00);
                    }

                    init(); // Initialize parameters such as octave, transposition, etc upon new channel definition.

                    currentChan = string_to_num(defineStr);

                    if (patterns.size() != 0 && tracks.size() == 0)
                    {
                        print_error("Definition of \"#patterns\" requires definition of \"#tracks\".");
                        return false;
                    }
                    else if (patterns.size() == 0 && tracks.size() != 0)
                    {
                        print_error("Definition of \"#tracks\" requires definition of \"#patterns\".");
                        return false;
                    }

                    // Check whether or not #patterns is used in the song
                    if (patterns.size() == 0)
                    {
                        if (!isPatternDefined)
                        {
                            /* Default pattern list, if #pattern isn't defined.
                             * It has two patterns; one for the intro and one for the "looped" bit. */

                            music.pattern_write(music.offsetARAM + 8);
                            unsigned short loopAddress = music.offsetARAM;
                            music.pattern_write(music.offsetARAM + 22); // It'd normally be 24 but we need to take into the account that the ARAM address changed just now.

                            music.pattern_write(0x00FF);
                            music.pattern_write(loopAddress);

                            for (unsigned t = 0; t < 16; t++)
                            {
                                music.track_write(0x0000); // 8 channels, 2 patterns
                            }

                            isPatternDefined = true;
                        }

                        music.track_overwrite(currentChan * 2, music.offsetARAM);
                    }
                    else
                    {
                        // Search for the track number and overwrite with current pointer.
                        for (unsigned j = 0; j < tracks.size(); j++)
                        {
                            for (unsigned k = 0; k < tracks[j].size(); k++)
                            {
                                //cout << j * 8 + k << endl;
                                if (currentChan == tracks[j][k])
                                {
                                    music.track_overwrite((short)((j * 8 + k) * 2), music.offsetARAM);
                                }
                            }
                        }
                    }
                }
                else if (defineStr == "patterns")
                {
                    if (!iterate(chr, i))
                    {
                        print_error(END_OF_FILE);
                        return false;
                    }
                    update_line(chr);

                    while (chr != '{')
                    {
                        if (!is_whitespace(chr))
                        {
                            print_error("Unexpected character before \'{\' token.");
                            return false;
                        }

                        iterate(chr, i);
                        update_line(chr);
                    }

                    iterate(chr, i);
                    update_line(chr);
                    string numStr;

                    unsigned short loopPoint = ARAM_START;

                    while (chr != '}')
                    {
                        if (is_digit(chr))
                        {
                            numStr += chr;
                        }
                        else if (is_whitespace(chr) || chr == '/')
                        {
                            if (numStr != "")
                                patterns.push_back(string_to_num(numStr));
                            numStr = "";

                            if (chr == '/')
                            {
                                if (loopPoint == ARAM_START)
                                {
                                    loopPoint += (short)(patterns.size() * 2);
                                }
                                else
                                {
                                    print_error("Multiple declarations of song loop placement: \"/\".");
                                    return false;
                                }
                            }
                        }
                        else
                        {
                            print_error(string("Unexpected character: \'") + chr + "\'.");
                            return false;
                        }

                        iterate(chr, i);
                        update_line(chr);
                    }

                    iterate(chr, i);
                    update_line(chr);

                    // Write pattern binary data based on the info we've gathered from #patterns

                    unsigned short patternStart = ARAM_START;
                    bool isSongLooped = false;

                    if (patterns[patterns.size() - 1] == 0)
                    {
                        patternStart += patterns.size() * 2;
                    }
                    else
                    {
                        patternStart += (patterns.size() * 2) + 4;
                        isSongLooped = true;
                    }

                    for (unsigned i = 0; i < patterns.size(); i++)
                    {
                        if (patterns[i] == 0)
                        {
                            music.pattern_write(0x0000);
                        }
                        else
                        {
                            music.pattern_write(((patterns[i] - 1) * 16) + patternStart); // Each pattern takes up 16 bytes
                        }
                    }

                    if (isSongLooped)
                    {
                        music.pattern_write(0x00FF); // Loop song command
                        music.pattern_write(loopPoint);
                    }
                }
                else if (defineStr == "tracks")
                {
                    if (!iterate(chr, i))
                    {
                        print_error(END_OF_FILE);
                        return false;
                    }
                    update_line(chr);

                    while (chr != '{')
                    {
                        if (!is_whitespace(chr))
                        {
                            print_error("Unexpected character before \'{\' token.");
                            return false;
                        }

                        iterate(chr, i);
                        update_line(chr);
                    }

                    iterate(chr, i);
                    update_line(chr);
                    string numStr;

                    unsigned pat = 0;

                    {
                        vector<int> v;
                        tracks.push_back(v);
                    }

                    while (chr != '}')
                    {
                        if (is_digit(chr) || chr == '-')
                        {
                            numStr += chr;
                        }
                        else if (is_whitespace(chr))
                        {
                            if (numStr != "")
                                tracks[pat].push_back(string_to_num(numStr));
                            numStr = "";
                        
                            if (chr == '\n')
                            {
                                if (tracks[pat].size() > 0)
                                {
                                    if (tracks[pat].size() != 8)
                                    {
                                        print_error("Each pattern must have exactly 8 tracks.");
                                        return false;
                                    }

                                    pat++;

                                    vector<int> v;
                                    tracks.push_back(v);
                                }
                            }
                        }
                        else
                        {
                            print_error(string("Unexpected character: \'") + chr + "\'.");
                            return false;
                        }

                        iterate(chr, i);
                        update_line(chr);
                    }

                    iterate(chr, i);
                    update_line(chr);

                    for (unsigned j = 0; j < tracks.size(); j++)
                    {
                        for (unsigned k = 0; k < tracks[j].size(); k++)
                        {
                            // Write a bunch of filler NULL pointers to be updated later for each track in each pattern.
                            music.track_write(0x0000);
                        }
                    }
                }
                else if (defineStr == "instruments")
                {
                    unsigned short insAram = INSTRUMENT_ARAM;
                    
                    const unsigned DEFAULT_SIZE = 144;
                    const unsigned LOCAL_SIZE = 24;
                    
                    // Initialize global instrument data;
                    byte defaultData[DEFAULT_SIZE] = {
                        0x00, 0xFF, 0xE0, 0xB8, 0x02, 0xD6,
                        0x01, 0xFF, 0xE0, 0xB8, 0x03, 0xD4,
                        0x02, 0xFF, 0xE0, 0xB8, 0x07, 0xA8,
                        0x03, 0xFF, 0xF6, 0xB8, 0x03, 0xFF,
                        0x04, 0xFF, 0xEC, 0xB8, 0x0A, 0x02,
                        0x05, 0xFF, 0xE0, 0xB8, 0x03, 0xC8,
                        0x06, 0xFF, 0xE0, 0xB8, 0x03, 0x00,
                        0x07, 0xFF, 0xE0, 0xB8, 0x05, 0x40,
                        0x08, 0xFF, 0xE0, 0xB8, 0x0A, 0x12,
                        0x09, 0xFF, 0xE0, 0xB8, 0x0A, 0x4D,
                        0x0A, 0xFF, 0xE0, 0xB8, 0x03, 0x74,
                        0x0B, 0xFF, 0xEB, 0xB8, 0x01, 0xED,
                        0x0C, 0xFF, 0xE0, 0xB8, 0x03, 0xD0,
                        0x0D, 0xFF, 0xF0, 0xB8, 0x02, 0x08,
                        0x0E, 0x8F, 0x19, 0xB8, 0x03, 0x00,
                        0x0F, 0x8F, 0xE0, 0xB8, 0x07, 0xA8,
                        0x10, 0x8D, 0xE0, 0xB8, 0x03, 0x00,
                        0x11, 0x8F, 0xF1, 0xB8, 0x07, 0xA8,
                        0x12, 0x8B, 0xE0, 0xB8, 0x03, 0x00,
                        0x13, 0xFE, 0xF3, 0xB8, 0x04, 0xFF,
                        0x14, 0xFF, 0xE0, 0xB8, 0x02, 0x7A,
                        0x15, 0xFF, 0xE0, 0xB8, 0x05, 0x5A,
                        0x16, 0xFF, 0xE0, 0xB8, 0x02, 0x3F,
                        0x17, 0xFD, 0xE0, 0xB8, 0x07, 0xA8
                    };
                    
                    byte fgData[LOCAL_SIZE] = {
                        0x18, 0xFF, 0xE0, 0xB8, 0x07, 0xA8,
                        0x19, 0xFF, 0xE0, 0xB8, 0x03, 0xB8,
                        0x1A, 0xFF, 0xE0, 0xB8, 0x03, 0xC8,
                        0x1B, 0xFF, 0xE2, 0xB8, 0x03, 0x00
                    };
                
                    if (!iterate(chr, i))
                    {
                        print_error(END_OF_FILE);
                        return false;
                    }
                    update_line(chr);

                    while (chr != '{')
                    {
                        if (!is_whitespace(chr))
                        {
                            print_error("Unexpected character before \'{\' token.");
                            return false;
                        }

                        iterate(chr, i);
                        update_line(chr);
                    }
                    
                    iterate(chr, i);
                    update_line(chr);
                    
                    while (chr != '}')
                    {
                        if (chr == '$')
                        {
                            iterate(chr, i);
                            
                            string hexStr;
                            
                            while (is_digit(chr) || (chr >= 'a' && chr <= 'f'))
                            {
                                hexStr += chr;
                                iterate(chr, i);
                            }
                            
                            i--;
                            chr = tolower(data[i]);
                            
                            unsigned val = hex_to_dec(hexStr);
                            music.instrument_write((byte)val);
                        }
                        else if (chr == '\"')
                        {
                            string brrStr;
                            
                            for (iterate(chr, i); i < data.size() && chr != '\"' && chr != '\n'; iterate(chr, i))
                            {
                                brrStr += chr;
                            }
                            
                            if (chr != '\"')
                            {
                                print_error("Expected end quote.");
                                return false;
                            }
                            
                            if (samples.find(brrStr) == samples.end())
                            {
                                print_error(string("The sample: \"") + brrStr + "\" has not been inserted (yet?)");
                                return false;
                            }
                            else
                            {
                                music.instrument_write(samples[brrStr].get_slot());
                            }
                        }
                        else if (chr == '#')
                        {
                            string setStr;
                            
                            for (iterate(chr, i); i < data.size() && !is_whitespace(chr); iterate(chr, i))
                            {
                                setStr += chr;
                            }
                            
                            i--;
                            chr = tolower(data[i]);
                            
                            if (setStr == "default")
                            {
                                for (unsigned j = 0; j < DEFAULT_SIZE; j++)
                                {
                                    music.instrument_write(defaultData[j]);
                                }
                            }
                            else if (setStr == "fg_set")
                            {
                                for (unsigned j = 0; j < LOCAL_SIZE; j++)
                                {
                                    music.instrument_write(fgData[j]);
                                }
                            }
                            else
                            {
                                print_error(string("Unknown instrument set: \"") + setStr + "\".");
                                return false;
                            }
                        }
                        else if (!is_whitespace(chr))
                        {
                            print_error(string("Unexpected character: \'") + chr + "\'.");
                            return false;
                        }
                    
                        iterate(chr, i);
                        update_line(chr);
                    }
                    
                    iterate(chr, i);
                    update_line(chr);
                    
                    // Set upload start and address
                    music.insStart[0] = (byte)(music.instrumentSize & 255);
                    music.insStart[1] = (byte)((music.instrumentSize >> 8) & 255);
                    music.insStart[2] = (byte)(insAram & 255);
                    music.insStart[3] = (byte)((insAram >> 8) & 255);
                }
                else if (defineStr == "local_samples")
                {
                    music.samplesAreDefined = true;
                    
                    unsigned short samplePtrAram = SAMPLE_PTR_ARAM;
                    unsigned short sampleDataAram = SAMPLE_DATA_ARAM;
                    
                    // Todo: put code for configuration of upload start here
                    
                    music.samplePtrStart[2] = (byte)(samplePtrAram & 255);
                    music.samplePtrStart[3] = (byte)((samplePtrAram >> 8) & 255);
                    
                    music.sampleDataStart[2] = (byte)(sampleDataAram & 255);
                    music.sampleDataStart[3] = (byte)((sampleDataAram >> 8) & 255);
                    
                    byte slot = 0x18;
                
                    if (!iterate(chr, i))
                    {
                        print_error(END_OF_FILE);
                        return false;
                    }
                    update_line(chr);

                    while (chr != '{')
                    {
                        if (!is_whitespace(chr))
                        {
                            print_error("Unexpected character before \'{\' token.");
                            return false;
                        }

                        iterate(chr, i);
                        update_line(chr);
                    }
                    
                    iterate(chr, i);
                    update_line(chr);
                    
                    while (chr != '}')
                    {
                        if (chr == '\"')
                        {
                            string brrStr;
                            
                            for (iterate(chr, i); i < data.size() && chr != '\"' && chr != '\n'; iterate(chr, i))
                            {
                                brrStr += chr;
                            }
                            
                            if (chr != '\"')
                            {
                                print_error("Expected end quote.");
                                return false;
                            }
                            
                            Brr brr(brrStr);
                            if (brr.has_error())
                            {
                                print_error(string("Could not find the file: \"") + brrStr + "\".");
                                return false;
                            }
                            else if (brr.size() == 0 || brr.size() % 9 != 0)
                            {
                                print_error(string("The file: \"") + brrStr + "\" is in the wrong format.");
                                return false;
                            }
                            
                            // Create new sample if it doesn't exist.
                            if (samples.find(brrStr) == samples.end())
                            {
                                unsigned short loopAram = sampleDataAram + brr.get_loop_point();
                                
                                Sample sample(slot, sampleDataAram, loopAram);
                                samples.insert(make_pair(brrStr, sample));
                                
                                // Write sample pointers
                                music.sample_ptr_write(sampleDataAram);
                                music.sample_ptr_write(loopAram);
                                
                                // Write sample data
                                for (unsigned j = 0; j < brr.size(); j++)
                                {
                                    music.sample_data_write(brr.read_data(j));
                                }
                                
                                sampleDataAram += brr.size();
                            }
                            else // if the sample already exists, duplicate the pointers, but don't make new data for it.
                            {
                                Sample sample;
                                sample = samples[brrStr];
                            
                                music.sample_ptr_write(sample.get_start_address());
                                music.sample_ptr_write(sample.get_loop_address());
                            }
                            
                            slot++;
                        }
                        else if (!is_whitespace(chr))
                        {
                            print_error(string("Unexpected character: \'") + chr + "\'.");
                            return false;
                        }
                    
                        iterate(chr, i);
                        update_line(chr);
                    }
                    
                    iterate(chr, i);
                    update_line(chr);
                    
                    music.samplePtrStart[0] = (byte)(music.samplePtrSize & 255);
                    music.samplePtrStart[1] = (byte)((music.samplePtrSize >> 8) & 255);
                    
                    music.sampleDataStart[0] = (byte)(music.sampleDataSize & 255);
                    music.sampleDataStart[1] = (byte)((music.sampleDataSize >> 8) & 255);
                }
                else
                {
                    print_error(INVALID_VALUE);
                    return false;
                }

                break;
            }

            case '/':
            {
                if (isLoop)
                {
                    print_error("Cannot declare song loop within loop brackets.");
                    return false;
                }

                if (patterns.size() != 0)
                {
                    print_error("Song loop must be specified within the patterns definition.");
                    return false;
                }

                music.write(isLoop, 0x00);
                music.track_overwrite(16 + currentChan * 2, music.offsetARAM);

                pattern_init(); // Force the note length and dynamic to reset

                hasIntro = true;

                iterate(chr, i);

                break;
            }

            case '(':
            {
                if (isLoop)
                {
                    print_error("Cannot call loop patterns within loops.");
                    return false;
                }

                if (!iterate(chr, i))
                {
                    print_error("Expected label closing parentheses \'(\'");
                    return false;
                }

                string labelStr;

                while (is_digit(chr) || (chr >= 'a' && chr <= 'z') || chr == '_')
                {
                    labelStr += data[i]; // We use data[i] instead of chr because this is case-sensitive.

                    if (!iterate(chr, i))
                    {
                        print_error("Expected label closing parentheses \'(\'");
                        return false;
                    }
                }

                if (chr != ')')
                {
                    print_error("Invalid label name.");
                    return false;
                }

                if (loopLabels.find(labelStr) == loopLabels.end())
                {
                    if (iterate(chr, i))
                    {
                        if (chr != '[')
                        {
                            print_error("Label has not been defined (yet?)");
                            return false;
                        }

                        currentLabel = labelStr;
                    }
                    else
                    {
                        print_error("Label has not been defined (yet?)");
                        return false;
                    }
                }
                else
                {
                    string loopStr;
                    unsigned loopCount = 1;

                    if (!iterate(chr, i))
                    {
                        print_error(END_OF_FILE);
                        return false;
                    }

                    while (is_digit(chr))
                    {
                        loopStr += chr;
                        iterate(chr, i);
                    }

                    if (loopStr != "")
                    {
                        loopCount = string_to_num(loopStr);
                    }

                    if (loopCount == 0)
                    {
                        print_warning("Having a loop count of 0 may lead to undesired/unexpected results.");
                    }
                    else if (loopCount > 255)
                    {
                        print_error("Invalid loop count.");
                        return false;
                    }

                    music.write(isLoop, 0xEF);

                    Loop newLoop(music.size, loopLabels.find(labelStr)->second);
                    loopOrigins.push_back(newLoop);

                    music.write(isLoop, 0x00);
                    music.write(isLoop, 0x00);
                    music.write(isLoop, loopCount);

                    // Force re-initialization of note length data after a loop call.
                    pattern_init();
                    //
                }

                break;
            }

            case '[':
            {
                if (isLoop)
                {
                    print_error("Nested loop brackets.");
                    return false;
                }

                music.write(isLoop, 0xEF); // Loop call command.

                if (currentLabel != "")
                {
                    loopLabels.insert(pair<string, unsigned>(currentLabel, loopDestinations.size()));
                    currentLabel = "";
                }

                Loop newLoop(music.size, loopDestinations.size());
                loopDestinations.push_back((short)music.loopSize);
                loopOrigins.push_back(newLoop);

                for (unsigned n = 0; n < 3; n++)
                {
                    music.write(isLoop, 0x00); // Write 3 temporary bytes for the loop address and loop count. They will be changed later.
                }

                isLoop = true;
                i++;

                prevTicks = 0xFF; // Force the note length to be re-declared at the start of the loop data

                break;
            }

            case ']':
            {
                if (!isLoop)
                {
                    print_error("Unexpected loop closing bracket.");
                    return false;
                }

                music.write(isLoop, 0x00);

                string loopStr;
                unsigned loopCount = 1;

                if (!iterate(chr, i))
                {
                    print_error(END_OF_FILE);
                    return false;
                }

                while (is_digit(chr))
                {
                    loopStr += chr;
                    iterate(chr, i);
                }

                if (loopStr != "")
                {
                    loopCount = string_to_num(loopStr);
                }

                if (loopCount == 0)
                {
                    print_warning("Having a loop count of 0 may lead to undesired/unexpected results.");
                }
                else if (loopCount > 255)
                {
                    print_error("Invalid loop count.");
                    return false;
                }

                music.overwrite_byte(loopOrigins[loopOrigins.size() - 1].dataIndex + 2, (byte)loopCount);

                // Force re-initialization of note length data after a loop call.
                pattern_init();
                //

                isLoop = false;
                //i++;

                break;
            }

            case '*':
            {
                if (isLoop)
                {
                    print_error("Cannot call loop patterns within loops.");
                    return false;
                }

                music.write(isLoop, 0xEF); // Loop call command.

                Loop newLoop(music.size, loopDestinations.size() - 1);
                loopOrigins.push_back(newLoop);

                for (unsigned n = 0; n < 2; n++)
                {
                    music.write(isLoop, 0x00); // Write 2 temporary bytes for the loop address. They will be changed later.
                }

                string loopStr;
                unsigned loopCount = 1;

                if (!iterate(chr, i))
                {
                    print_error(END_OF_FILE);
                    return false;
                }

                while (is_digit(chr))
                {
                    loopStr += chr;
                    iterate(chr, i);
                }

                if (loopStr != "")
                {
                    loopCount = string_to_num(loopStr);
                }

                if (loopCount == 0)
                {
                    print_warning("Having a loop count of 0 may lead to undesired/unexpected results.");
                }
                else if (loopCount > 255)
                {
                    print_error("Invalid loop count.");
                    return false;
                }

                music.write(isLoop, loopCount);

                // Force re-initialization of note length data after a loop call.
                prevTicks = 0xFF;
                prevDynamic = 0xFF;
                //

                break;
            }

            case '{':
                if (isTriplet)
                {
                    print_error("Nested triplet braces.");
                    return false;
                }

                isTriplet = true;
                i++;
                break;

            case '}':
                if (!isTriplet)
                {
                    print_error("Unexpected triplet closing brace.");
                    return false;
                }

                isTriplet = false;
                i++;
                break;

            case '<':
                octave--;
                i++;
                break;

            case '>':
                octave++;
                i++;
                break;

            case '$':
            {
                string hexStr;

                if (!iterate(chr, i))
                {
                    print_error(END_OF_FILE);
                    return false;
                }

                while (is_nybble(chr))
                {
                    hexStr += chr;
                    if (!iterate(chr, i))
                    {
                        break;
                    }
                }

                if (hexStr == "")
                {
                    print_error("No hex value specified.");
                    return false;
                }
                else if (hexStr.size() > 2)
                {
                    print_error("Invalid hex value specified.");
                    return false;
                }

                byte hex = hex_to_dec(hexStr);

                music.write(isLoop, hex);

                break;
            }

            case 'h':
            {
                string transposeStr;

                if (!iterate(chr, i))
                {
                    print_error(END_OF_FILE);
                    return false;
                }

                while (is_digit(chr) || chr == '-')
                {
                    transposeStr += chr;
                    if (!iterate(chr, i))
                    {
                        break;
                    }
                }

                if (transposeStr == "")
                {
                    print_error(NO_VALUE);
                    return false;
                }
                else if (!is_integer(transposeStr))
                {
                    print_error(INVALID_VALUE);
                    return false;
                }

                transpose = string_to_num(transposeStr);

                break;
            }

            case 'i':
            {
                const byte hexCommand = 0xF4;
                byte parameter;

                parameter = (byte)get_parameter(0, DECIMAL, chr, i);
                if (isError)
                {
                    return false;
                }

                music.write(isLoop, hexCommand);
                music.write(isLoop, parameter);

                break;
            }

            case 'j':
            {
                const byte hexCommand = 0xE9;
                byte parameter;

                parameter = (byte)get_parameter(0, SIGNED_DECIMAL, chr, i);
                if (isError)
                {
                    return false;
                }

                music.write(isLoop, hexCommand);
                music.write(isLoop, parameter);

                break;
            }

            case 'k':
            {
                const byte hexCommand = 0xEA;
                byte parameter;

                parameter = (byte)get_parameter(0, SIGNED_DECIMAL, chr, i);
                if (isError)
                {
                    return false;
                }

                music.write(isLoop, hexCommand);
                music.write(isLoop, parameter);

                break;
            }

            case 'l':
            {
                string lengthStr = "";

                if (!iterate(chr, i))
                {
                    print_error(END_OF_FILE);
                    return false;
                }

                while (is_digit(chr))
                {
                    lengthStr += chr;
                    if (!iterate(chr, i))
                    {
                        break;
                    }
                }

                if (lengthStr == "")
                {
                    print_error(NO_VALUE);
                    return false;
                }

                defaultLength = string_to_num(lengthStr);

                break;
            }

            case 'm':
            {
                const byte hexCommand = 0xF1;
                byte parameters[3];
                Parameter types[3] = {DECIMAL, DECIMAL, SIGNED_DECIMAL};

                for (unsigned p = 0; p < 3; p++)
                {
                    parameters[p] = (byte)get_parameter(p, types[p], chr, i);
                    if (isError)
                    {
                        return false;
                    }
                }

                music.write(isLoop, hexCommand);
                music.write(isLoop, parameters[0]);
                music.write(isLoop, parameters[1]);
                music.write(isLoop, parameters[2]);

                break;
            }

            case 'n':
            {
                const byte hexCommand = 0xF2;
                byte parameters[3];
                Parameter types[3] = {DECIMAL, DECIMAL, SIGNED_DECIMAL};

                for (unsigned p = 0; p < 3; p++)
                {
                    parameters[p] = (byte)get_parameter(p, types[p], chr, i);
                    if (isError)
                    {
                        return false;
                    }
                }

                music.write(isLoop, hexCommand);
                music.write(isLoop, parameters[0]);
                music.write(isLoop, parameters[1]);
                music.write(isLoop, parameters[2]);

                break;
            }

            case 'o':
            {
                string octaveStr = "";

                if (!iterate(chr, i))
                {
                    print_error(END_OF_FILE);
                    return false;
                }

                while (is_digit(chr))
                {
                    octaveStr += chr;
                    if (!iterate(chr, i))
                    {
                        break;
                    }
                }

                if (octaveStr == "")
                {
                    print_error(NO_VALUE);
                    return false;
                }

                octave = (byte)string_to_num(octaveStr);

                if (octave < 1 || octave > 6)
                {
                    print_error("Invalid octave.");
                    return false;
                }

                break;
            }

            case 'p':
            {
                const byte hexCommand = 0xE3;
                byte parameters[3];
                Parameter types[3] = {DECIMAL, DECIMAL, DECIMAL};

                for (unsigned p = 0; p < 2; p++)
                {
                    parameters[p] = (byte)get_parameter(p, types[p], chr, i);
                    if (isError)
                    {
                        return false;
                    }
                }

                // Make the first (delay) parameter optional.
                if (chr == ',')
                {
                    parameters[2] = (byte)get_parameter(2, types[2], chr, i);
                    if (isError)
                    {
                        return false;
                    }
                }
                else
                {
                    parameters[2] = parameters[1];
                    parameters[1] = parameters[0];
                    parameters[0] = 0;
                }
                //

                music.write(isLoop, hexCommand);
                music.write(isLoop, parameters[0]);
                music.write(isLoop, parameters[1]);
                music.write(isLoop, parameters[2]);

                break;
            }

            case 'q':
            {
                string dynamicStr;

                if (!iterate(chr, i))
                {
                    print_error(END_OF_FILE);
                    return false;
                }

                while (is_nybble(chr))
                {
                    dynamicStr += chr;
                    if (!iterate(chr, i))
                    {
                        break;
                    }
                }

                if (dynamicStr == "")
                {
                    print_error(NO_VALUE);
                    return false;
                }

                unsigned dyn = hex_to_dec(dynamicStr.substr(0, 2));
                if (dynamicStr.size() > 2 || dyn > 127)
                {
                    print_error(INVALID_VALUE);
                    return false;
                }

                prevDynamic = 0xFF;
                dynamic = (byte)dyn;

                break;
            }

            case 's':
            {
                byte parameters[3];
                Parameter types[3];
                types[0] = DECIMAL;

                parameters[0] = (byte)get_parameter(0, types[0], chr, i);
                if (isError)
                {
                    return false;
                }

                unsigned numParams;
                byte hexCommand;

                if (chr == ',')
                {
                    iterate(chr, i);

                    if ((chr >= 'h' && chr <= 'z' && chr != 'r') || chr == '@' || chr == '&')
                    {
                        switch (chr)
                        {
                            case 'p':
                                hexCommand = 0xF0;
                                numParams = 0;
                                break;

                            case 't':
                                hexCommand = 0xE8;
                                numParams = 1;
                                types[1] = DECIMAL;
                                break;

                            case 'v':
                                hexCommand = 0xEE;
                                numParams = 1;
                                types[1] = DECIMAL;
                                break;

                            case 'w':
                                hexCommand = 0xE6;
                                numParams = 1;
                                types[1] = DECIMAL;
                                break;

                            case 'y':
                                hexCommand = 0xE2;
                                numParams = 1;
                                types[1] = DECIMAL;
                                break;

                            case 'x':
                                hexCommand = 0xF8;
                                numParams = 2;
                                types[1] = SIGNED_DECIMAL;
                                types[2] = SIGNED_DECIMAL;
                                break;

                            default:
                            {
                                string err = "Special fade command \'s\' does not apply to \'";
                                print_error(err + command + "\'.");
                                return false;

                                break;
                            }
                        }
                    }
                    else
                    {
                        print_error("Not enough parameters for \'s\'.");
                        return false;
                    }
                }
                else
                {
                    print_error("Not enough parameters for \'s\'.");
                    return false;
                }

                for (unsigned p = 0; p < numParams; p++)
                {
                    parameters[p + 1] = (byte)get_parameter(p, types[p + 1], chr, i);
                    if (isError)
                    {
                        return false;
                    }
                }

                if (numParams == 0)
                {
                    iterate(chr, i);
                }

                music.write(isLoop, hexCommand);
                music.write(isLoop, parameters[0]);
                for (unsigned p = 0; p < numParams; p++)
                {
                    music.write(isLoop, parameters[p + 1]);
                }

                break;
            }

            case 't':
            {
                const byte hexCommand = 0xE7;
                byte parameter;

                parameter = (byte)get_parameter(0, DECIMAL, chr, i);
                if (isError)
                {
                    return false;
                }

                music.write(isLoop, hexCommand);
                music.write(isLoop, parameter);

                break;
            }

            case 'u':
            {
                const byte hexCommand = 0xEB;
                byte parameters[3];
                Parameter types[3] = {DECIMAL, DECIMAL, DECIMAL};

                for (unsigned p = 0; p < 2; p++)
                {
                    parameters[p] = (byte)get_parameter(p, types[p], chr, i);
                    if (isError)
                    {
                        return false;
                    }
                }

                // Make the first (delay) parameter optional.
                if (chr == ',')
                {
                    parameters[2] = (byte)get_parameter(2, types[2], chr, i);
                    if (isError)
                    {
                        return false;
                    }
                }
                else
                {
                    parameters[2] = parameters[1];
                    parameters[1] = parameters[0];
                    parameters[0] = 0;
                }
                //

                music.write(isLoop, hexCommand);
                music.write(isLoop, parameters[0]);
                music.write(isLoop, parameters[1]);
                music.write(isLoop, parameters[2]);

                break;
            }

            case 'v':
            {
                const byte hexCommand = 0xED;
                byte parameter;

                parameter = (byte)get_parameter(0, DECIMAL, chr, i);
                if (isError)
                {
                    return false;
                }

                music.write(isLoop, hexCommand);
                music.write(isLoop, parameter);

                break;
            }

            case 'w':
            {
                const byte hexCommand = 0xE5;
                byte parameter;

                parameter = (byte)get_parameter(0, DECIMAL, chr, i);
                if (isError)
                {
                    return false;
                }

                music.write(isLoop, hexCommand);
                music.write(isLoop, parameter);

                break;
            }

            case 'x':
            {
                const byte hexCommand = 0xF5;
                byte parameters[3];
                Parameter types[3] = {BINARY, SIGNED_DECIMAL, SIGNED_DECIMAL};

                for (unsigned p = 0; p < 3; p++)
                {
                    parameters[p] = (byte)get_parameter(p, types[p], chr, i);
                    if (isError)
                    {
                        return false;
                    }
                }

                music.write(isLoop, hexCommand);
                music.write(isLoop, parameters[0]);
                music.write(isLoop, parameters[1]);
                music.write(isLoop, parameters[2]);

                break;
            }

            case 'y':
            {
                const byte hexCommand = 0xE1;
                byte parameters[3];
                Parameter types[3] = {DECIMAL, BINARY, BINARY};

                parameters[0] = (byte)get_parameter(0, types[0], chr, i);
                if (isError)
                {
                    return false;
                }

                if (parameters[0] > 20)
                {
                    print_warning("Panning value is out of range for command \'y\'.");
                }

                // Make the next 2 parameters optional.
                if (chr == ',')
                {
                    for (unsigned p = 1; p < 3; p++)
                    {
                        parameters[p] = (byte)get_parameter(p, types[p], chr, i);
                        if (isError)
                        {
                            return false;
                        }

                        if (parameters[p] > 1)
                        {
                            print_error("Invalid extra parameters in \'y\' command (Must be either 0 or 1).");
                            return false;
                        }
                    }

                    music.write(isLoop, hexCommand);
                    music.write(isLoop, parameters[0] | (parameters[1] * 0x40) | (parameters[2] * 0x80));
                }
                else
                {
                    music.write(isLoop, hexCommand);
                    music.write(isLoop, parameters[0]);
                }
                //

                break;
            }

            case 'z':
            {
                const byte hexCommand = 0xF7;
                byte parameters[3];
                Parameter types[3] = {DECIMAL, SIGNED_DECIMAL, DECIMAL};

                for (unsigned p = 0; p < 3; p++)
                {
                    parameters[p] = (byte)get_parameter(p, types[p], chr, i);
                    if (isError)
                    {
                        return false;
                    }
                }

                music.write(isLoop, hexCommand);
                music.write(isLoop, parameters[0]);
                music.write(isLoop, parameters[1]);
                music.write(isLoop, parameters[2]);

                break;
            }

            case '@':
            {
                const byte hexCommand = 0xE0;
                byte parameter;

                parameter = (byte)get_parameter(0, DECIMAL, chr, i);
                if (isError)
                {
                    return false;
                }

                music.write(isLoop, hexCommand);
                music.write(isLoop, parameter);

                break;
            }

            case '&':
            {
                const byte hexCommand = 0xF9;
                byte parameters[3];
                Parameter types[3] = {DECIMAL, DECIMAL, DECIMAL};

                for (unsigned p = 0; p < 3; p++)
                {
                    parameters[p] = (byte)get_parameter(p, types[p], chr, i);
                    if (isError)
                    {
                        return false;
                    }
                }

                if (parameters[2] > 71)
                {
                    print_warning("Note value is out of range for command \'&\'.");
                }

                music.write(isLoop, hexCommand);
                music.write(isLoop, parameters[0]);
                music.write(isLoop, parameters[1]);
                music.write(isLoop, parameters[2] + 128);

                break;
            }

            case '~':
            {
                string commandStr = "";
                char subCommand;

                if (!iterate(chr, i))
                {
                    print_error(END_OF_FILE);
                    return false;
                }

                if ((chr >= 'h' && chr <= 'z' && chr != 'r') || chr == '@' || chr == '&')
                {
                    commandStr += chr;
                    if (!iterate(chr, i))
                    {
                        break;
                    }
                }

                if (commandStr == "")
                {
                    print_error(NO_VALUE);
                    return false;
                }

                subCommand = commandStr[0];

                switch (subCommand)
                {
                    case 'm':
                    case 'n':
                        music.write(isLoop, 0xF3);
                        break;

                    case 'p':
                        music.write(isLoop, 0xE4);
                        break;

                    case 'u':
                        music.write(isLoop, 0xEC);
                        break;

                    case 'x':
                    case 'z':
                        music.write(isLoop, 0xF6);
                        break;

                    default:
                    {
                        string err = "\'~\' does not apply to the command \'";
                        print_error(err + subCommand + "\'.");
                        return false;

                        break;
                    }
                }

                break;
            }

            case 'a':
            case 'b':
            case 'c':
            case 'd':
            case 'e':
            case 'f':
            case 'g':
            case 'r':
            case '^':
            {
                byte note;

                if (chr == '^')
                {
                    note = TIE;
                }
                else if (chr == 'r')
                {
                    note = REST;
                }
                else
                {
                    note = BASE_NOTE + NOTE_TABLE[chr - 'a'];

                    if (note < BASE_NOTE)
                        note += 12;

                    note += (octave - 1) * 12;

                    if (iterate(chr, i))
                    {
                        if (chr == '+')
                        {
                            note++;
                        }
                        else if (chr == '-')
                        {
                            note--;
                        }
                        else
                        {
                            i--;
                            chr = tolower(data[i]);
                        }
                    }
                    else
                    {
                        i--;
                        chr = tolower(data[i]);
                    }

                    note += (signed)transpose;

                    if (note < BASE_NOTE)
                    {
                        print_error("Note interval is too low.");
                        return false;
                    }
                    else if (note > CAP_NOTE)
                    {
                        print_error("Note interval is too high.");
                        return false;
                    }
                }

                unsigned length;
                unsigned ticks[2] = {0, 0};

                if (iterate(chr, i))
                {
                    if (chr == '=')
                    {
                        command = '=';

                        if (!iterate(chr, i))
                        {
                            print_error(END_OF_FILE);
                            return false;
                        }

                        string tickStr;

                        while (is_digit(chr))
                        {
                            tickStr += chr;
                            if (!iterate(chr, i))
                            {
                                break;
                            }
                        }

                        if (tickStr == "")
                        {
                            print_error(NO_VALUE);
                            return false;
                        }

                        ticks[0] = string_to_num(tickStr);
                    }
                    else
                    {
                        string lengthStr;

                        while (is_digit(chr))
                        {
                            lengthStr += chr;
                            if (!iterate(chr, i))
                            {
                                break;
                            }
                        }

                        if (lengthStr == "")
                        {
                            length = defaultLength;
                        }
                        else
                        {
                            length = string_to_num(lengthStr);
                        }

                        ticks[0] = WHOLE_NOTE / length;

                        if (isTriplet)
                        {
                            ticks[0] = (ticks[0] * 2) / 3;
                        }

                        unsigned ticksHalf = ticks[0];

                        while (chr == '.')
                        {
                            ticksHalf /= 2;
                            ticks[0] += ticksHalf;

                            if (!iterate(chr, i))
                            {
                                break;
                            }
                        }
                    }
                }
                else
                {
                    ticks[0] = WHOLE_NOTE / defaultLength;
                }

                unsigned numNotes = 1;

                if (ticks[0] > 192)
                {
                    print_error("Note duration is too long.");
                    return false;
                }
                else if (ticks[0] > 127)
                {
                    // If length is greater than 127, split it up into a note + a tie.
                    ticks[1] = ticks[0] - 96;
                    ticks[0] = 96;
                    numNotes = 2;
                }

                if (ticks[0] != 0)
                {
                    for (unsigned j = 0; j < numNotes; j++)
                    {
                        if (ticks[j] != prevTicks || dynamic != prevDynamic)
                        {
                            music.write(isLoop, ticks[j]);
                            prevTicks = ticks[j];

                            if (dynamic != prevDynamic)
                            {
                                music.write(isLoop, dynamic);
                                prevDynamic = dynamic;
                            }
                        }
                        music.write(isLoop, note);

                        note = TIE;
                    }
                }
            }

                break;

            default:
                i++;
                break;
        }
    }

    if (patterns.size() == 0 && !hasIntro)
    {
        music.pattern_overwrite(0x2, 0x00FF);
        music.pattern_overwrite(0x4, ARAM_START); // Make the song loop back to the very first pattern.
        music.pattern_overwrite(0x6, 0x0000); // Clean this up a bit.
    }

    music.write(isLoop, 0x00); // Mark end of channel data.

    for (unsigned i = 0; i < loopOrigins.size(); i++)
    {
        music.overwrite(loopOrigins[i].dataIndex, loopDestinations[loopOrigins[i].destIndex] + music.offsetARAM);
    }

    music.offsetPC += music.loopSize;
    music.offsetARAM += music.loopSize;

    // Allocate song's memory in ARAM
    unsigned short songSize = music.offsetARAM - ARAM_START;
    music.songStart[0] = (byte)(songSize & 255);
    music.songStart[1] = (byte)((songSize >> 8) & 255);

    music.offsetPC += 14; // Take the start and end of the upload into account.
    //

    return true;
}

// Private helper functions

void Mml::remove_comments()
{
    string newData;
    bool isComment = false;
    lineNum = 1;

    for (unsigned i = 0; i < data.size(); i++)
    {
        if (data[i] == '#')
        {
            newData += enter_file(data, i);
        }
    
        switch (data[i])
        {
            case ';':
            {
                if (!isComment)
                    isComment = true;

                break;
            }
            
            case '\r':
            case '\n':
            {
                if (isComment)
                    isComment = false;

                newData += data[i];
                lineNum++;
                
                break;
            }

            default:
            {
                // Open included file code goes here.
                if (!isComment)
                {
                    if (i < data.size() - 8 && data.substr(i, 8) == "#include")
                    {
                        unsigned start = i;
                    
                        data.replace(i, 8, "#file=");
                        
                        for (i += 6; i < data.size() && is_whitespace(data[i]) && data[i] != '\n'; data.erase(i, 1));
                            
                        if (data[i] != '\"')
                        {
                            print_error(string("Unexpected character: \"") + data[i] + "\".");
                            return;
                        }
                        
                        string fileName;
                        
                        for (i++; i < data.size() && data[i] != '\"'; i++)
                        {
                            fileName += data[i];
                        }
                        
                        i++;
                        //data.insert(i + 1, "\n");
                        //i++;
                        
                        string fileData;
                        ifstream fin;
                        
                        fin.open(fileName.c_str());
                        
                        if (!fin.is_open())
                        {
                            print_error("The file: \"" + fileName + "\" cannot be found.");
                            return;
                        }
                        else
                        {
                            while (!fin.eof())
                            {
                                string fileLine;
                                getline(fin, fileLine);
                                fileData += fileLine + "\n";
                            }
                            
                            fin.close();
                        }
                        
                        fileData += "#endfile\n";
                        data.insert(i + 1, fileData + "\n");
                        
                        i = start - 1;
                    }
                    else if (i < data.size() - 7 && data.substr(i, 7) == "#incbin")
                    {
                        unsigned start = i;
                        for (i += 7; i < data.size() && is_whitespace(data[i]) && data[i] != '\n'; data.erase(i, 1));
                        
                        if (data[i] != '\"')
                        {
                            print_error(string("Unexpected character: \"") + data[i] + "\".");
                            return;
                        }
                        
                        string fileName;
                        
                        for (i++; i < data.size() && data[i] != '\"'; i++)
                        {
                            fileName += data[i];
                        }
                        
                        i++;
                        data.replace(start, i - start, include_binary(fileName));
                        i = start - 1;
                    }
                    else
                    {
                        newData += data[i];
                    }
                }

                break;
            }
        }
    }
    
    swap(data, newData); // Copies contents of newData to data by pointer.
    lineNum = 1;
}

void Mml::replace()
{
    string newData;
    
    bool isVar = false;
    bool isQuote = false;
    bool isLhs = true;
    bool ignore = false;
    
    string lhs, rhs, entirety;
    map<string, string> replacements;

    for (unsigned i = 0; i < data.size(); i++)
    {
        if (data[i] == '#')
        {
            newData += enter_file(data, i);
        }
    
        if (data[i] == '!' && !isQuote) // We'd need to ignore quotes if we're passing a !print command
        {
            newData += data[i];
            ignore = true;
        }
        else if (data[i] == '\"')
        {
            isQuote = !isQuote;
            
            if (!isQuote)
            {
                if (isVar || ignore || rhs == "")
                    newData += "\"" + entirety + "\"";
                else if (replacements.find(lhs) == replacements.end())
                    replacements.insert(make_pair(lhs, rhs));
                else
                    replacements[lhs] = rhs;
                    
                ignore = false;
            }
            
            lhs = "";
            rhs = "";
            entirety = "";
            isLhs = true;
        }
        else if (data[i] == ':' && !isQuote)
        {
            newData += data[i];
            isVar = !isVar;
        }
        else if (!isQuote && !isVar && !ignore)
        {
            string match;
        
            for (auto it = replacements.begin(); it != replacements.end(); it++)
            {
                bool uniqueMatch = false;
                
                // Search for matches
                for (unsigned j = 0; i + j < data.size() && data[i + j] == it->first[j]; j++)
                {
                    if (j == it->first.size() - 1)
                    {
                        uniqueMatch = true;
                        match = it->first;
                        
                        for (auto it2 = it; it2 != replacements.end(); it2++) // Check to make sure this word isn't a subset of a longer one.
                        {
                            if (it != it2)
                            {
                                if (it2->first.substr(0, match.size()) == match)
                                    uniqueMatch = false;
                            }
                        }
                        
                        break;
                    }
                }
                
                if (uniqueMatch)
                    break;
            }
            
            if (match != "")
            {
                newData += replacements[match];
                i += match.size() - 1;
            }
            else
            {
                newData += data[i];
            }
        }
        else if (isQuote)
        {
            if (data[i] == '=')
                isLhs = false;
            else
            {
                if (isLhs)
                    lhs += data[i];
                else
                    rhs += data[i];
            }
            
            entirety += data[i];
        }
        else
        {
            newData += data[i];
        }
    }

    swap(data, newData); // Copies contents of newData to data by pointer.
}

bool Mml::add_defines()
{
    string newData;
    bool isDefine = false;
    bool isInsideBrace = false;
    string directive;
    string defineType;
    string alias;
    string insStr;
    
    for (unsigned i = 0; i < data.size(); i++)
    {
        if (data[i] == '#')
        {
            newData += enter_file(data, i);
        }
    
        if (!isDefine && data[i] == '#')
        {
            string defName;
            
            if (data[i] == '\n')
            {
                newData += '\n';
            }
            
            for (i++; i < data.size() && !is_whitespace(data[i]); i++)
            {
                defName += data[i];
            }
            
            directive = defName;
            
            if (defName == "define" || defName == "undef" || defName == "ifndef" || defName == "ifdef")
            {
                isDefine = true;
                isInsideBrace = false;
            }
            else if (defName == "endif")
            {
            }
            else
            {
                newData += string("#") + defName + " ";
            }
            
            if (data[i] == '\n')
            {
                newData += '\n';
            }
        }
        else if (isDefine)
        {
            if (!isInsideBrace)
            {
                defineType = "";
                
                for (; i < data.size() && is_whitespace(data[i]); i++)
                {
                    if (data[i] == '\n')
                    {
                        newData += '\n';
                    }
                }
                for (; i < data.size() && !is_whitespace(data[i]); i++)
                {
                    defineType += data[i];
                }
                
                alias = "";
                
                if (defineType == "kit" || defineType == "list" || defineType == "macro")
                {
                    for (; i < data.size() && is_whitespace(data[i]); i++)
                    {
                        if (data[i] == '\n')
                        {
                            newData += '\n';
                        }
                    }
                    for (; i < data.size() && !is_whitespace(data[i]); i++)
                    {
                        alias += data[i];
                    }
                
                    for (; i < data.size() && is_whitespace(data[i]); i++)
                    {
                        if (data[i] == '\n')
                        {
                            newData += '\n';
                        }
                    }
                    
                    newData += '\n';
                }
                else
                {
                    if (data[i] == '\n')
                    {
                        newData += '\n';
                    }
                    
                    i--;
                }
                
                isInsideBrace = true;
            }
            else if (directive == "define")
            {
                int braceStack = 1;
            
                if (defineType == "list")
                {
                    string listData;
                    
                    if (data[i] == '\n')
                    {
                        newData += '\n';
                        lineNum++;
                    }
                    
                    for (i++; i < data.size() && braceStack > 0; i++)
                    {
                        if (data[i] == '}')
                        {
                            braceStack--;
                            
                            if (braceStack > 0)
                            {
                                listData += data[i];
                            }
                        }
                        else if (!is_whitespace(data[i]))
                        {
                            listData += data[i];
                        }
                        else if (data[i] == '\n')
                        {
                            newData += '\n';
                        }
                    }
                    
                    newData += ":?" + alias + " = \"";
                    newData += "%list_create(" + listData + ")"; // Create a variable that holds the list ID.
                    newData += "\":";
                }
                else if (defineType == "kit")
                {
                    string noteData;
                    string defaultData = "r";
                    string modifierStr;
                    
                    vector<string> kitData(72, "");
                    unsigned index = 0;
                    bool isQuote = false;
                    
                    if (data[i] == '\n')
                    {
                        newData += '\n';
                    }
                    
                    for (i++; i < data.size() && braceStack > 0; i++)
                    {
                        if (data[i] == '}')
                        {
                            braceStack--;
                            
                            if (isQuote && braceStack > 0)
                            {
                                noteData += data[i];
                            }
                        }
                        else if (data[i] == '\n')
                        {
                            newData += '\n';
                            
                            if (modifierStr == "#default")
                            {
                                kitData[index] = defaultData;
                                index++;
                                modifierStr.clear();
                            }
                            else if (modifierStr.substr(0, 7) == "#start=")
                            {
                                if (!is_integer(modifierStr.substr(7)))
                                {
                                    lineNum = get_line_num(data, i);
                                    print_error("Invalid start value.");
                                    return false;
                                }
                                else
                                {
                                    unsigned start = (unsigned)string_to_num(modifierStr.substr(7));
                                    for (; index < 72 && index < start; index++)
                                    {
                                        kitData[index] = defaultData;
                                    }
                                }
                                
                                modifierStr.clear();
                            }
                            else if (modifierStr != "")
                            {
                                print_error(string("Invalid modifier: \"") + modifierStr + "\".");
                                return false;
                            }
                        }
                        else
                        {
                            if (!isQuote)
                            {
                                if (data[i] != '\"')
                                {
                                    if (!is_whitespace(data[i]))
                                        modifierStr += data[i];
                                }
                                else
                                {
                                    isQuote = true;
                                }
                            }
                            else
                            {
                                if (data[i] != '\"')
                                {
                                    noteData += data[i];
                                }
                                else
                                {
                                    isQuote = false;
                                    
                                    if (modifierStr == "#default=")
                                    {
                                        defaultData = noteData;
                                        modifierStr.clear();
                                    }
                                    else if (modifierStr == "")
                                    {
                                        kitData[index] = noteData;
                                        index++;
                                    }
                                    else
                                    {
                                        lineNum = get_line_num(data, i);
                                        print_error(string("Invalid modifier: \"") + modifierStr + "\".");
                                        return false;
                                    }
                                    
                                    noteData.clear();
                                }
                            }
                        }
                    }
                    
                    // Repeat kit notes
                    unsigned start = 0;
                    for (unsigned i = 0; i < kitData.size(); i++)
                    {
                        if (kitData[i] == "")
                        {
                            kitData[i] = kitData[start];
                            start++;
                        }
                    }
                    
                    kits.insert(make_pair(alias, kitData));
                }
                else if (defineType == "macro")
                {
                    Macro* macro = new Macro;
                
                    string macroname;
                    string params;
                    
                    int found = alias.find('(');
                
                    if (found == -1)
                    {
                        macroname = alias;
                        params = "()";
                    }
                    else
                    {
                        macroname = alias.substr(0, found);
                        params = alias.substr(found);
                    }
                    
                    if (params[params.size() - 1] != ')')
                    {
                        lineNum = get_line_num(data, i);
                        print_error("Missing closing parenthesis");
                        return false;
                    }
                    
                    if (macros.find(macroname) != macros.end())
                    {
                        lineNum = get_line_num(data, i);
                        print_error("Redefinition of macro: \"" + macroname + "\".");
                        return false;
                    }
                    
                    macros.insert(make_pair(macroname, macro));
                    
                    string currentParam;
                    
                    for (unsigned j = 1; j < params.size() - 1; j++)
                    {
                        if (params[j] == ',')
                        {
                            macro->parameters.push_back(currentParam);
                            currentParam = "";
                        }
                        else if (params[j] == '?' || isalnum(params[j]) || params[j] == '_')
                        {
                            currentParam += params[j];
                        }
                        else
                        {
                            lineNum = get_line_num(data, i);
                            print_error(string("Unexpected character: \"") + params[j] + "\".");
                            return false;
                        }
                    }
                    
                    if (params.size() - 2 != 0)
                        macro->parameters.push_back(currentParam);
                    
                    if (data[i] == '\n')
                    {
                        newData += '\n';
                    }
                        
                    braceStack = 1;
                
                    for (i++; i < data.size() && braceStack > 0; i++)
                    {
                        if (data[i] == '}')
                        {
                            braceStack--;
                        }
                        else if (data[i] == '{')
                        {
                            braceStack++;
                        }
                        
                        if (data[i] == '\n')
                        {
                            newData += '\n';
                            if (braceStack > 0)
                                macro->body += "\\";
                        }
                        else
                        {
                            if (braceStack > 0)
                                macro->body += data[i];
                        }
                    }
                    
                    string preserve;
                    
                    for (int j = 0; j < macro->parameters.size(); j++)
                    {
                        string cp = macro->parameters[j];
                        preserve += string(":") + cp + " = \"?_ARG" + num_to_string(j) + "_\":"
                        + "%list_push(?_MACRO_STACK_, " + cp + ") ";
                    }
                    
                    macro->body = preserve + macro->body;
                    
                    for (int j = macro->parameters.size() - 1; j >= 0; j--)
                    {
                        string cp = macro->parameters[j];
                        macro->body += string(":") + cp + " = \"%list_top(?_MACRO_STACK_)\": %list_pop(?_MACRO_STACK_) ";
                    }
                }
                else
                {
                    defines.insert(defineType);
                    //i--;
                }
                
                isDefine = false;
                isInsideBrace = false;
                
                //if (data[i] == '\n')
                //{
                //    newData += '\n';
                //}
            }
            else if (directive == "ifdef")
            {
                if (defines.find(defineType) == defines.end())
                {
                    //if (data[i] == '\n')
                    //{
                    //    newData += data[i];
                    //}
                
                    for (; i < data.size() && !(i < data.size() - 7 && data.substr(i + 1, 6) == "#endif"); i++)
                    {
                        if (data[i] == '\n')
                        {
                            newData += data[i];
                        }
                    }
                }
                
                isDefine = false;
                isInsideBrace = false;
                
                //i--;
            }
            else if (directive == "ifndef")
            {
                if (defines.find(defineType) != defines.end())
                {
                    //if (data[i] == '\n')
                    //{
                    //    newData += data[i];
                    //}
                
                    for (; i < data.size() && !(i < data.size() - 7 && data.substr(i + 1, 6) == "#endif"); i++)
                    {
                        if (data[i] == '\n')
                        {
                            newData += data[i];
                        }
                    }
                }
                
                isDefine = false;
                isInsideBrace = false;
                
                //i--;
            }
            else if (directive == "undef")
            {
                if (defines.find(defineType) == defines.end())
                {
                    lineNum = get_line_num(data, i);
                    print_error("\"" + defineType + "\" is already undefined.");
                    return false;
                }
                
                defines.erase(defineType);
                
                isDefine = false;
                isInsideBrace = false;
                
                //i--;
            }
        }
        else
        {
            newData += data[i];
            
            if (data[i] == '\n')
            {
            }
            else if (data[i] == '@')
            {
                insStr = "@";
            }
            else if (insStr.size() > 0 && insStr[0] == '@')
            {
                if (!is_whitespace(data[i]))
                {
                    insStr += data[i];
                }
                else
                {
                    if (insStr.size() > 1 && ((tolower(insStr[1]) >= 'a' && tolower(insStr[1]) <= 'z') || insStr[1] == '_')) // Check if kit instrument is delcared
                        newData += "o?o q?q ";
                    insStr.clear();
                }
            }
        }
    }
    
    lineNum = 1;
    
    swap(data, newData); // Copies contents of newData to data by pointer.
    return true;
}

bool Mml::write_variables(string& dt, unsigned index, bool& isLabel)
{
    if (dt[index + 1] == '?' || dt[index + 1] == '%')
        return true;

    string contents;
    char var = tolower(dt[index]);
    string varName;
    varName += var;

    if (var == '(')
    {
        isLabel = true;
    }
    else if (var == ')')
    {
        isLabel = false;
    }

    if (!isLabel)
    {
        switch (var)
        {
            case 'h':
            case 'i':
            case 'j':
            case 'k':
            case 'l':
            case 'm':
            case 'n':
            case 'o':
            case 'p':
            case 't':
            case 'u':
            case 'v':
            case 'w':
            case 'x':
            case 'y':
            case 'z':
            case '@':
            case '&':
            case '#':
            {
                string params[3];
                unsigned p = 0;

                for (unsigned i = index + 1; i < dt.size() && ((dt[i] >= '0' && dt[i] <= '9') || dt[i] == ',' || dt[i] == '-'); i++)
                {
                    contents += dt[i];

                    if (dt[i] != ',' && p < 3)
                        params[p] += dt[i];
                    else
                        p++;
                }

                if (contents == "")
                    break;

                if (tolower(dt[index]) == '@')
                {
                    variables["ins"] = contents;
                }
                else if (var == '&')
                {
                    variables["pb"] = contents;

                    variables["pbDelay"] = params[0];
                    variables["pbLength"] = params[1];
                    variables["pbNote"] = params[2];
                }
                else if (var == '#')
                {
                    variables["chan"] = contents;
                    variables["o"] = "4";
                    variables["octave"] = "4";
                    variables["h"] = "0";
                    variables["transpose"] = "0";
                    variables["q"] = "7F";
                    variables["Q"] = "127";
                    variables["velocity"] = "15";
                    variables["legato"] = "7";
                }
                else
                {
                    variables[varName] = contents;

                    switch (var)
                    {
                        case 'h':
                            variables["transpose"] = contents;
                            break;

                        case 'i':
                            variables["fine"] = contents;
                            break;

                        case 'j':
                            variables["songKey"] = contents;
                            break;

                        case 'k':
                            variables["chanKey"] = contents;
                            break;

                        case 'l':
                            variables["length"] = contents;
                            break;

                        case 'm':
                        case 'n':
                            variables["portaDelay"] = params[0];
                            variables["portaLength"] = params[1];
                            variables["portaNote"] = params[2];
                            break;

                        case 'o':
                            variables["octave"] = contents;
                            break;

                        case 'p':
                            if (params[2] != "")
                            {
                                variables["vibDelay"] = params[0];
                                variables["vibFreq"] = params[1];
                                variables["vibDepth"] = params[2];
                            }
                            else
                            {
                                variables["vibDelay"] = "0";
                                variables["vibFreq"] = params[0];
                                variables["vibDepth"] = params[1];
                            }

                            break;

                        case 't':
                            variables["tempo"] = contents;
                            break;

                        case 'u':
                            if (params[2] != "")
                            {
                                variables["tremDelay"] = params[0];
                                variables["tremFreq"] = params[1];
                                variables["tremDepth"] = params[2];
                            }
                            else
                            {
                                variables["tremDelay"] = "0";
                                variables["tremFreq"] = params[0];
                                variables["tremDepth"] = params[1];
                            }

                            break;

                        case 'v':
                            variables["chanVolume"] = contents;
                            break;

                        case 'w':
                            variables["songVolume"] = contents;
                            break;

                        case 'x':
                            variables["echoFlags"] = params[0];
                            variables["echoRightVol"] = params[1];
                            variables["echoLeftVol"] = params[2];
                            break;

                        case 'y':
                            variables["pan"] = contents;
                            break;

                        case 'z':
                            variables["echoDelay"] = params[0];
                            variables["echoFeedback"] = params[1];
                            variables["echoFIR"] = params[2];
                            break;
                    }
                }

                break;
            }

            case 'q':
            {
                for (unsigned i = index + 1; i < dt.size() && ((dt[i] >= '0' && dt[i] <= '9') || (tolower(dt[i]) >= 'a' && tolower(dt[i]) <= 'f')); i++)
                {
                    contents += dt[i];
                }

                variables["q"] = contents;
                unsigned q = hex_to_dec(contents);
                unsigned legato = (q & ~15) >> 4;
                unsigned velocity = q & 15;

                ostringstream ssout;

                ssout << q;
                variables["Q"] = ssout.str();
                ssout.str("");
                ssout.clear();

                ssout << legato;
                variables["legato"] = ssout.str();
                ssout.str("");
                ssout.clear();

                ssout << velocity;
                variables["velocity"] = ssout.str();
                ssout.str("");
                ssout.clear();

                break;
            }

            case '<':
            {
                string octStr = variables["o"];
                for (unsigned i = 0; i < octStr.size(); i++)
                {
                    if (!is_digit(octStr[i]))
                    {
                        print_error(string("The implicit assignment of built-in variable: \"?octave\" has\nfailed. This is likely due to an improper use of the \'o\' command. (Current\ncontents of \"?octave\": \"") + octStr + "\").");
                        return false;
                    }
                }

                int oct = string_to_num(octStr);
                oct--;
                ostringstream ssout;
                ssout << oct;

                variables["o"] = ssout.str();
                variables["octave"] = ssout.str();

                break;
            }

            case '>':
            {
                string octStr = variables["o"];
                for (unsigned i = 0; i < octStr.size(); i++)
                {
                    if (!is_digit(octStr[i]))
                    {
                        print_error(string("The implicit assignment of built-in variable: \"?octave\" has\nfailed. This is likely due to an improper use of the \'o\' command. (Current\ncontents of \"?octave\": \"") + octStr + "\").");
                        return false;
                    }
                }

                int oct = string_to_num(octStr);
                oct++;
                ostringstream ssout;
                ssout << oct;

                variables["o"] = ssout.str();
                variables["octave"] = ssout.str();

                break;
            }

            default:
                break;
        }
    }

    return true;
}

bool Mml::read_variables(string& dt, unsigned index)
{
    if (index >= dt.size())
        return true;

    if (dt[index] == '?')
    {
        string varName;

        for (unsigned i = index + 1; i < dt.size() && ((dt[i] >= '0' && dt[i] <= '9') || (tolower(dt[i]) >= 'a' && tolower(dt[i]) <= 'z') || dt[i] == '_'); i++)
        {
            varName += dt[i];
        }

        map<string, string>::iterator it, it1, it2, it3;

        it1 = variables.find(varName);
        it2 = userDefined.find(varName);
        it3 = constants.find(varName);

        if (it1 != variables.end())
            it = it1;
        else if (it2 != userDefined.end())
            it = it2;
        else if (it3 != constants.end())
            it = it3;
        else
        {
            print_error(string("Unknown variable: \'") + varName + "\'");
            return false;
        }

        string contents = it->second;
        if (contents == "" && it != it2)
        {
            print_error(string("Uninitialized variable: \'") + varName + "\'");
            return false;
        }

        dt.replace(index, varName.size() + 1, contents);
    }

    return true;
}

bool Mml::assign_variables(string& dt, unsigned& index)
{
    string varName;
    string expr;
    
    isAssignment = true;
    isQuote = false;
    
    for (index++; index < dt.size() && dt[index] != '?'; index++)
    {
        if (dt[index] != ' ' && dt[index] != '\t')
        {
            print_error(string("Unexpected character: \'") + dt[index] + "\' during variable assignment.");
            return false;
        }
    }

    for (index++; index < dt.size() && ((dt[index] >= '0' && dt[index] <= '9') || (tolower(dt[index]) >= 'a' && tolower(dt[index]) <= 'z') || dt[index] == '_'); index++)
    {
        varName += dt[index];
    }

    for (index++; index < dt.size() && dt[index] != '='; index++)
    {
        if (dt[index] != ' ' && dt[index] != '\t')
        {
            print_error(string("Unexpected character: \'") + dt[index] + "\' during variable assignment.");
            return false;
        }
    }
    
    for (index++; index < dt.size() && is_whitespace(dt[index]); index++);
    
    vector<string> expressions;

    for (; index < dt.size() && dt[index] != ':'; index++)
    {
        if (dt[index] == '\n')
        {
            print_error("Unexpected linebreak in variable assignment.");
            return false;
        }
        
        if (is_whitespace(dt[index]))
        {
            if (isQuote)
                expr += dt[index];
        }
        else if (dt[index] == '\"')
        {
            isQuote = !isQuote;
            if (expr != "")
                expressions.push_back(expr);
            expr = "";
        }
        else
        {
            if (expr != "" && is_whitespace(dt[index - 1]))
                expr += " ";
        
            expr += dt[index];
        }
    }
    
    if (expr != "")
        expressions.push_back(expr);
    
    string betaExpr;

    for (unsigned i = 0; i < expressions.size(); i++)
    {
        string ex = expressions[i];
    
        if (!expand_macros(ex)) // e now holds the expanded data
            return false;
        
        betaExpr += ex;
    }
    
    string fullExpr;
    
    for (unsigned i = 0; i < betaExpr.size(); i++)
    {
        if (betaExpr[i] != '\0')
            fullExpr += betaExpr[i];
    }
    
    auto it1 = variables.find(varName);
    auto it2 = constants.find(varName);
    auto it3 = userDefined.find(varName);
    
    if (it1 != variables.end() || it2 != constants.end())
    {
        print_error("Cannot assign to built-in variable: \"" + varName + "\".");
        return false;
    }
    else if (it3 != userDefined.end())
    {
        userDefined[varName] = fullExpr;
    }
    else
    {
        userDefined.insert(make_pair(varName, fullExpr));
    }
    
    isAssignment = false;

    //index--;
    return true;
}

bool Mml::set_variable(string key, string value)
{
    if (variables.find(key) != variables.end())
    {
        print_error(string("Cannot assign to built-in variable: \"?") + key + "\". Use the equivalent MML command instead!");
        return false;
    }
    else if (constants.find(key) != constants.end())
    {
        print_error(string("Cannot assign to built-in constant: \"?") + key + "\".");
        return false;
    }

    if (userDefined.find(key) == userDefined.end())
    {
        // Create variable if it does not exist.
        userDefined.insert(pair<string, string>(key, value));
    }
    else
    {
        // Otherwise just replace it.
        userDefined[key] = value;
    }

    return true;
}

void Mml::print_variables()
{
    cout << "-- VARIABLES --" << endl;

    for (map<string, string>::iterator it = variables.begin(); it != variables.end(); it++)
    {
        cout << "?" << it->first << " - " << it->second << endl;
    }

    cout << "-- CONSTANTS --" << endl;

    for (map<string, string>::iterator it = constants.begin(); it != constants.end(); it++)
    {
        cout << "?" << it->first << " - " << it->second << endl;
    }

    cout << "-- USER DEFINED --" << endl;

    for (map<string, string>::iterator it = userDefined.begin(); it != userDefined.end(); it++)
    {
        cout << "?" << it->first << " - " << it->second << endl;
    }

    cout << "--------------" << endl;
}

vector<int> Mml::get_macro_parameters(string args)
{
    vector<int> parameters;
    vector<string> tokens;
    string tokenStr;
    
    for (unsigned i = 0; i < args.size(); i++)
    {
        if (args[i] == ')' || (args[i] == '(' && i == 0))
            tokens.push_back(string() + args[i]);
        else if (args[i] == '\'')
        {
            unsigned start = i;
            string quoteStr;
        
            do {
                quoteStr += args[i];
                i++;
            }
            while (args[i] != '\'');
            
            quoteStr += args[i];
            quoteStr = quoteStr.substr(1, quoteStr.size() - 2); // Strip quotes
            
            if (!expand_macros(quoteStr)) // If a variable or macro is in the quotes, evaluate it first.
            {
                return parameters;
            }
            
            string quoteData;
            
            for (unsigned j = 0; j < quoteStr.size(); j++)
            {
                char chr = quoteStr[j];
                quoteData += string() + num_to_string(chr);
                
                if (j != quoteStr.size() - 1)
                    quoteData += string() + ",";
            }
            
            unsigned length = i - start + 1;
            args.replace(start, length, quoteData);
            i = start - 1;
        }
        else if (args[i] == ',')
            tokens.push_back(string() + args[i]);
        else if (args[i] == '+' || args[i] == '*' || args[i] == '/' || args[i] == '&' || args[i] == '|' || args[i] == '^' || args[i] == '~')
            tokens.push_back(string() + args[i]);
        else if (args[i] == '=' || args[i] == '!' || args[i] == '<' || args[i] == '>')
        {
            if (args[i + 1] == '=')
            {
                tokens.push_back(string() + args[i] + "=");
                i++;
            }
            else
            {
                tokens.push_back(string() + args[i]);
            }
        }
        else if (is_digit(args[i]) || args[i] == '-')
        {
            if (args[i] == '-' && is_integer(tokens[tokens.size() - 1]))
            {
                tokens.push_back(string() + args[i]);
            }
            else
            {
                do {
                    tokenStr += args[i];
                    i++;
                }
                while (is_digit(args[i]));
                
                i--;
                tokens.push_back(tokenStr);
                tokenStr = "";
            }
        }
        else if (args[i] == '$')
        {
            do {
                tokenStr += tolower(args[i]);
                i++;
            }
            while (is_digit(args[i]) || (tolower(args[i]) >= 'a' && tolower(args[i]) <= 'f'));
            
            i--;
            tokens.push_back(tokenStr);
            tokenStr = "";
        }
        else if (tolower(args[i]) >= 'a' && tolower(args[i]) <= 'g' && !(tolower(args[i + 1]) >= 'a' && tolower(args[i + 1]) <= 'z'))
        {
            tokenStr += tolower(args[i]);
            i++;
            if (args[i] == '+' || args[i] == '-')
                tokenStr += args[i];
            else
                i--;
                
            tokens.push_back(tokenStr);
            tokenStr = "";
        }
        else if (args[i] == '?')
        {
            unsigned start = i;
            string varString;
            
            do {
                varString += args[i];
                i++;
            }
            while (is_digit(args[i]) || (tolower(args[i]) >= 'a' && tolower(args[i]) <= 'z') || args[i] == '_');
            
            unsigned length = i - start;
            
            if (!read_variables(varString, 0))
                return parameters;
                
            args.replace(start, length, varString);
            i = start - 1;
        }
        else if (args[i] == '%' || args[i] == '(')
        {
            unsigned start = i;
            string macroStr;

            if (args[i] == '%')
            {
                do {
                    macroStr += args[i];
                    i++;
                }
                while (i < args.size() && args[i] != '(');
            }
            else if (args[i] == '(')
            {
                macroStr += "%";
            }
        
            int stack = 1;
            macroStr += args[i];
        
            for (i++; i < args.size() && stack > 0; i++)
            {
                macroStr += args[i];
            
                if (args[i] == '(')
                    stack++;
                else if (args[i] == ')')
                    stack--;
            }
            
            if (stack > 0)
            {
                print_error("Missing closing parentheses.");
                return parameters;
            }
            
            unsigned length = i - start;
            
            unsigned discard = 1;
            string macroData = evaluate_macro(macroStr, discard);
            
            if (isError)
                return parameters;
                
            args.replace(start, length, macroData);
            i = start - 1;
        }
        else if (!is_whitespace(args[i]))
        {
            do {
                tokenStr += args[i];
                i++;
            }
            while (i < args.size() && !is_whitespace(args[i]));
            
            i--;
            tokens.push_back(tokenStr);
            tokenStr = "";
        }
    }
    
    string numModifier = "";
    string prevNum = "";
    string prevOperation = "";
    string prevToken = "";
    
    for (unsigned i = 0; i < tokens.size(); i++)
    {
        if (is_integer(tokens[i]) || tokens[i][0] == '$' || (tolower(tokens[i][0]) >= 'a' && tolower(tokens[i][0]) <= 'g' && (tokens[i].size() == 1 || (!(tolower(tokens[i][1]) >= 'a' && tolower(tokens[i][1]) <= 'z')))))
        {
            int num = 0;
        
            if (tokens[i][0] == '$')
            {
                num = hex_to_dec(tokens[i].substr(1));
            }
            else if (tokens[i][0] >= 'a' && tokens[i][0] <= 'g')
            {
                num = NOTE_TABLE[tokens[i][0] - 'a'];
                
                if (tokens[i].size() > 1)
                {
                    if (tokens[i][1] == '+')
                        num++;
                    else if (tokens[i][1] == '-')
                        num--;
                }
            }
            else
            {
                num = string_to_num(tokens[i]);
            }
            
            if (numModifier == "~")
                num = ~num;
            else if (numModifier == "!")
                num = (int)(!num);
            
            if (prevNum != "" && prevOperation != "")
            {
                int lhs = string_to_num(prevNum);
            
                if (prevOperation == "+")
                    num = lhs + num;
                else if (prevOperation == "-")
                    num = lhs - num;
                else if (prevOperation == "*")
                    num = lhs * num;
                else if (prevOperation == "/")
                    num = lhs / num;
                else if (prevOperation == "mod")
                    num = lhs % num;
                else if (prevOperation == "&")
                    num = lhs & num;
                else if (prevOperation == "|")
                    num = lhs | num;
                else if (prevOperation == "^")
                    num = lhs ^ num;
                else if (prevOperation == "=" || prevOperation == "==")
                    num = (int)(lhs == num);
                else if (prevOperation == "!=")
                    num = (int)(lhs != num);
                else if (prevOperation == ">")
                    num = (int)(lhs > num);
                else if (prevOperation == "<")
                    num = (int)(lhs < num);
                else if (prevOperation == ">=")
                    num = (int)(lhs >= num);
                else if (prevOperation == "<=")
                    num = (int)(lhs <= num);
                else if (prevOperation == "and")
                    num = (int)(lhs && num);
                else if (prevOperation == "or")
                    num = (int)(lhs || num);
                else if (prevOperation == "xor")
                    num = (int)((!!lhs) != (!!num));
            }
            
            prevNum = num_to_string(num);
            prevOperation = "";
            numModifier = "";
        }
        else if (tokens[i] == "+" || tokens[i] == "-" || tokens[i] == "*" || tokens[i] == "/" || tokens[i] == "mod" || tokens[i] == "&" || tokens[i] == "|" || tokens[i] == "^" || tokens[i] == "=" || tokens[i] == "==" || tokens[i] == "!=" || tokens[i] == ">" || tokens[i] == "<" || tokens[i] == ">=" || tokens[i] == "<=" || tokens[i] == "and" || tokens[i] == "or" || tokens[i] == "xor")
        {
            if (prevOperation != "" || prevToken == ",")
            {
                print_error("Unexpected token: \"" + tokens[i] + "\".");
                return parameters;
            }
            
            prevOperation = tokens[i];
        }
        else if (tokens[i] == "~")
        {
            if (numModifier != "")
            {
                print_error("Unexpected token: \"" + tokens[i] + "\".");
                return parameters;
            }
            
            numModifier = tokens[i];
        }
        else if (tokens[i] == "!")
        {
            if (numModifier == "!")
                numModifier = "";
            else
            {
                if (numModifier != "")
                {
                    print_error("Unexpected token: \"" + tokens[i] + "\".");
                    return parameters;
                }
            
                numModifier = tokens[i];
            }
        }
        else if (tokens[i] == "=" || tokens[i] == "==" || tokens[i] == "!=")
        {
            if (prevOperation != "" || prevToken == ",")
            {
                print_error("Unexpected token: \"" + tokens[i] + "\".");
                return parameters;
            }
        
            prevOperation = tokens[i];
        }
        else if (tokens[i] == "," || tokens[i] == ")")
        {
            if (prevToken == "," || prevToken == "+" || prevToken == "-" || prevToken == "*" || prevToken == "/" || prevToken == "&" || prevToken == "|" || prevToken == "^" || prevToken == "~")
            {
                print_error("Unexpected token: \"" + tokens[i] + "\".");
                return parameters;
            }
            
            if (prevNum != "")
                parameters.push_back(string_to_num(prevNum));
            prevToken = tokens[i];
        }
        else if (tokens[i] == "(")
        {
            
        }
        else
        {
            print_error("Invalid token: \"" + tokens[i] + "\".");
            return parameters;
        }
        
        prevToken = tokens[i];
    }
    
    return parameters;
}

string Mml::evaluate_macro(string& dt, unsigned& index)
{
    string newData;
    string name;
    string paramData;
    vector<int> parameters;
    unsigned start = index - 1;
    
    unsigned i;
    
    for (i = index; i < dt.size() && ((tolower(dt[i]) >= 'a' && tolower(dt[i]) <= 'z') || is_digit(dt[i]) || dt[i] == '_'); i++)
    {
        name += dt[i];
    }
    
    for (; i < dt.size() && is_whitespace(dt[i]); i++);
    
    if (dt[i] != '(')
    {
        print_error(string("Unexpected character: \"") + dt[i] + "\".");
        return "";
    }

    int parensLine = lineNum;
    int stack = 1;
    
    paramData = dt[i];

    for (i++; i < dt.size() && stack > 0; i++)
    {
        if (dt[i] == '(')
            stack++;
        else if (dt[i] == ')')
            stack--;
            
        paramData += dt[i];
    }
    
    if (stack > 0)
    {
        lineNum = parensLine;
        print_error("Missing closing parentheses.");
        return "";
    }
    else
    {
        parameters = get_macro_parameters(paramData);
    }
    
    unsigned temp = i;
    
    if (name == "if" || name == "else" || name == "else_if")
    {
        evaluate_condition(name, parameters, dt, i); // This function writes to dt.
    }
    else if (name == "while")
    {
        string macroCall = dt.substr(start, i - start);
        string loopedData = evaluate_condition(name, parameters, dt, i); // i holds the index of the end of dt's replaced data
        string loopedPart = strip_newlines(macroCall + loopedData);
        
        if (loopedData != "")
            dt.insert(i - 2, loopedPart);
        else
            dt.insert(i - 2, "\n");
    }
    else if (name == "repeat")
    {
        int loopCount = 1;
        
        if (parameters.size() > 0)
            loopCount = parameters[0];
    
        string loopedData = evaluate_condition(name, parameters, dt, i); // i holds the index of the end of dt's replaced data
        string loopedPart = strip_newlines("%repeat(" + num_to_string(loopCount - 1) + ")" + loopedData);
        
        if (loopedData != "")
            dt.insert(i - 2, loopedPart);
        else
            dt.insert(i - 2, "\n");
    }
    else if (name == "")
    {
        if (parameters.size() > 0)
            newData = num_to_string(parameters[0]);
    }
    else
    {
        string empty;
        newData = get_macro_data(empty, parameters, name);
    }
    
    i = temp;
    
    index = i - 1;
    return newData;
}

string Mml::evaluate_condition(string macroname, const vector<int>& parameters, string &dt, unsigned& index)
{
    string oldData;
    string newData;
    string charpair, chartriplet;
    bool lastCondition, thisCondition = false;
    lastCondition = string_to_num(userDefined["_LAST_CONDITION_"]);
    
    int stack = 0;

    if (macroname == "if")
    {
        if (parameters.size() != 1)
        {
            print_error("In macro %if: Incorrect number of parameters.");
            return "";
        }
    
        thisCondition = parameters[0];
        lastCondition = false;
    }
    else if (macroname == "else")
    {
        if (parameters.size() != 0)
        {
            print_error("In macro %else: Incorrect number of parameters.");
            return "";
        }
        
        thisCondition = true;
    }
    else if (macroname == "else_if")
    {
        if (parameters.size() != 1)
        {
            print_error("In macro %else_if: Incorrect number of parameters.");
            return "";
        }
        
        thisCondition = parameters[0];
    }
    else if (macroname == "while")
    {
        if (parameters.size() != 1)
        {
            print_error("In macro %while: Incorrect number of parameters.");
            return "";
        }
        
        thisCondition = parameters[0];
        lastCondition = false;
    }
    else if (macroname == "repeat")
    {
        if (parameters.size() != 1)
        {
            print_error("In macro %repeat: Incorrect number of parameters.");
            return "";
        }
        
        thisCondition = (parameters[0] > 0);
        lastCondition = false;
    }
    
    unsigned start = index;
    
    for (unsigned i = index - 1; i < dt.size() - 1 && !(charpair == "}}" && stack == 0); i++)
    {
        charpair = dt.substr(i, 2);
        if (i < dt.size() - 2)
            chartriplet = dt.substr(i, 3);
        else
            chartriplet = charpair + " ";
        
        if (charpair == "{{")
        {
            stack++;
        }
        else if (charpair == "}}")
        {
            stack--;
        }
            
        if (stack > 0 || (charpair[1] != '{' && charpair[1] != '}') || (chartriplet.substr(1) != "{{" && chartriplet.substr(1) != "}}"))
        {
            if (thisCondition && !lastCondition)
            {
                if ((charpair != "{{" || stack > 1) && (charpair != "}}" || stack > 0))
                {
                    newData += charpair[1];
                }
                else if (charpair[1] == '\n')
                {
                    newData += "\n";
                }
            }
            else
            {
                if (charpair[1] == '\n')
                    newData += "\n";
            }
            
            index = i + 3;
        }
        else if (charpair[1] == '\n')
        {
            newData += "\n";
        }
        
        if (stack == 0 && charpair != "}}" && charpair[1] != '{' && !is_whitespace(charpair[1]))
        {
            print_error(string("Unexpected character: \"") + charpair[1] + "\".");
            return "";
        }
    }
    
    if (macroname == "if")
    {
        lastCondition = thisCondition;
    }
    else if (macroname == "else_if")
    {
        if (!lastCondition)
            lastCondition = thisCondition;
    }
    else if (macroname == "else")
    {
        lastCondition = false;
    }
    else if (macroname == "while")
    {
        lastCondition = thisCondition;
    }
    else if (macroname == "repeat")
    {
        lastCondition = thisCondition;
    }
    
    userDefined["_LAST_CONDITION_"] = num_to_string(lastCondition);
    
    while (newData[newData.size() - 1] == '}')
        newData.erase(newData.size() - 1, 1);
    
    newData = "%list_push(?_CONDITION_STACK_, ?_LAST_CONDITION_) " + newData + ":?_LAST_CONDITION_ = \"%list_top(?_CONDITION_STACK_)\": %list_pop(?_CONDITION_STACK_) ";

    if (macroname != "while" && macroname != "repeat")
        newData += "\n";
    else
        newData += "\\";
    
    oldData = dt.substr(start, index - start);
    dt.replace(start, index - start, newData);
    index = start + newData.size();
    
    if (thisCondition)
        return oldData;
        
    return "";
}

bool Mml::expand_macros(string& dt)
{
    string newData;
    bool isMacro = false;
    bool isLabel = false;

    for (unsigned i = 0; i < dt.size(); i++)
    {
        if (dt[i] == '#')
        {
            newData += enter_file(dt, i);
        }
    
        if (!isMacro && dt[i] == '\n')
            lineNum++; // Increase line number before ANY calls which could potentially output errors.
        
        if (dt[i] == '\"')
        {
            //newData += dt[i];
            isQuote = !isQuote;
        }
        
        if (!isQuote)
        {
            if (dt[i] == '?')
            {
                if (!read_variables(dt, i))
                    return false;
                    
                if (!isAssignment && !isEvalMacro && i != 0)
                {
                    if (!write_variables(dt, i - 1, isLabel))
                        return false;
                }
            }
            
            if (dt[i] == ':')
            {
                if (!assign_variables(dt, i))
                    return false;
            }
            else if (dt[i] == '%')
            {
                string before = "";
                if (i != 0)
                    before += dt[i - 1];
                
                i++;
                unsigned savedLine = lineNum;
                
                bool temp = isEvalMacro;
                isEvalMacro = true;
                
                string mData = evaluate_macro(dt, i);
                newData += mData;
                string varString = before + mData;
                
                if (!temp)
                    isEvalMacro = false;
                
                lineNum = savedLine;
                
                if (!isAssignment && !isEvalMacro)
                {
                    if (!write_variables(varString, 0, isLabel))
                        return false;
                }
            }
            else if (dt[i] == '[')
            {
                if (i < dt.size() - 1 && dt[i + 1] == '[')
                {
                    if (!expand_loops(dt, i)) // Replace the data inside the brackets with the expanded loops.
                        return false;
                }
                else
                {
                    newData += dt[i];
                }
            }
            else
            {
                newData += dt[i];
            
                if (!isAssignment && !isEvalMacro)
                {
                    if (!write_variables(dt, i, isLabel))
                        return false;
                }
            }
        }
        else
        {
            newData += dt[i];
        }
    }

    swap(dt, newData); // Copies contents of newData to data by pointer.
    return true;
}

bool Mml::expand_loops(string& dt, unsigned index)
{
    string s;
    string loopData;
    
    int loopDepth = 0;
    unsigned loopCount = 1;
    unsigned length = 0;
    unsigned numLines = 1;
    
    for (unsigned i = index; i < dt.size(); i++)
    {
        if (dt[i] == '#')
        {
            s += enter_file(dt, i);
        }
    
        if (dt[i] == '[')
        {
            i++;
            if (i < dt.size() && dt[i] == '[')
            {
                if (loopDepth == 0)
                {
                    loopData += " o" + variables["o"] + " h" + variables["h"] + " "; // Preserve octave and transposition.
                }
                else
                {
                    loopData += "[[";
                }
            
                loopDepth++;
            }
            else
            {
                i--;
                loopData += dt[i];
            }
        }
        else if (dt[i] == ']')
        {
            i++;
            if (i < dt.size() && dt[i] == ']')
            {
                loopDepth--;
                if (loopDepth == 0)
                {
                    string countStr;
                
                    for (i++; i < dt.size() && is_digit(dt[i]); i++) // Get loop count.
                    {
                        countStr += dt[i];
                    }
                    
                    if (countStr != "")
                    {
                        loopCount = string_to_num(countStr);
                    }
                    else if (dt[i] == '?')
                    {
                        string varStr = "?";
                        
                        for (i++; i < dt.size() && (is_digit(dt[i]) || (tolower(dt[i]) >= 'a' && tolower(dt[i]) <= 'z') || dt[i] == '_'); i++)
                        {
                            varStr += dt[i];
                        }
                        
                        if (!read_variables(varStr, 0))
                        {
                            return false;
                        }
                        
                        if (is_integer(varStr))
                        {
                            loopCount = string_to_num(varStr);
                        }
                    }
                    else if (dt[i] == '%')
                    {
                        string macroStr = "%";
                        int parensDepth = 0;
                        
                        for (i++; i < dt.size(); i++)
                        {
                            macroStr += dt[i];
                        
                            if (dt[i] == '(')
                            {
                                parensDepth++;
                            }
                            else if (dt[i] == ')')
                            {
                                parensDepth--;
                                if (parensDepth <= 0)
                                {
                                    i++;
                                    break;
                                }
                            }
                        }
                        
                        if (!expand_macros(macroStr))
                        {
                            return false;
                        }
                        
                        if (is_integer(macroStr))
                        {
                            loopCount = string_to_num(macroStr);
                        }
                    }
                    
                    length = i - index;
                    break;
                }
                else
                {
                    loopData += "]]";
                }
            }
            else
            {
                i--;
                loopData += dt[i];
            }
        }
        else
        {
            if (dt[i] == '\n')
            {
                numLines++;
                loopData += ' ';
            }
            else
            {
                loopData += dt[i];
            }
        }
    }
    
    if (length == 0)
    {
        print_error("Expected closing double-bracket.");
        return false;
    }
    
    for (unsigned i = 0; i < numLines; i++)
    {
        s += "\n";
    }
    
    for (unsigned i = 0; i < loopCount; i++)
    {
        s += " " + loopData;
    }
    
    dt.replace(index, length, s);
    
    return true;
}

string Mml::get_macro_data(string& dt, const vector<int>& p, string macroname)
{
    if (macroname == "list_push")
        return list_push(p);
    else if (macroname == "list_pop")
        return list_pop(p);
    else if (macroname == "list_top")
        return list_top(p);
    else if (macroname == "list_get")
        return list_get(p);
    else if (macroname == "list_set")
        return list_set(p);
    else if (macroname == "list_size")
        return list_size(p);
    else if (macroname == "list")
        return list(p);
    else if (macroname == "list_create")
        return list_create(p);
    else if (macroname == "list_destroy")
        return list_destroy(p);
    else if (macroname == "list_exists")
        return list_exists(p);
    else if (macroname == "rand")
    {
        if (p.size() > 0)
            srand(p[0]);
            
        string r = num_to_string(rand() % 65536);
        variables["seed"] = num_to_string(rand() % 65536);
        
        return r;
    }
    else
    {
        if (macros.find(macroname) == macros.end())
        {
            print_error("Undefined macro: \"" + macroname + "\".");
            return "";
        }
        
        Macro* theMacro = macros[macroname];

        for (unsigned i = 0; i < p.size() && i < theMacro->parameters.size(); i++)
        {
            string paramVar = theMacro->parameters[i].substr(1);
            if (userDefined.find(paramVar) != userDefined.end())
            {
                paramStack.push_back(userDefined[paramVar]);
            }
            else
            {
                paramStack.push_back("");
            }
            
            string argVar = "_ARG" + num_to_string(i) + "_";
        
            if (userDefined.find(argVar) == userDefined.end())
                userDefined.insert(make_pair(argVar, num_to_string(p[i]))); // ?_ARGx_ = parameter contests
            else
                userDefined[argVar] = num_to_string(p[i]);
        }
    
        string macrodata = theMacro->body;
        
        bool tmp = isEvalMacro;
        isEvalMacro = false;
        
        if (!expand_macros(macrodata))
            return "";
            
        isEvalMacro = tmp;
            
        int i = p.size() - 1;
        if (p.size() > theMacro->parameters.size())
            i = theMacro->parameters.size() - 1;
            
        for (; i >= 0; i--)
        {
            string paramVar = theMacro->parameters[i].substr(1);
            userDefined[paramVar] = paramStack.back();
            paramStack.pop_back();
        }
            
        return remove_whitespace(macrodata);
    }
}

string Mml::list_push(const std::vector<int> &p)
{
    if (p.size() != 2)
    {
        print_error("In macro: %list_push: Incorrect number of parameters.");
        return "";
    }
    
    if (find_list_id(p[0]) == -1)
    {
        print_error("In macro: %list_push: The specified list does not exist.");
        return "";
    }
    
    int id = p[0];
    int val = p[1];
    lists[id].push_back(val);

    return "";
}

string Mml::list_pop(const std::vector<int> &p)
{
    if (p.size() != 1)
    {
        print_error("In macro: %list_pop: Incorrect number of parameters.");
        return "";
    }
    
    if (find_list_id(p[0]) == -1)
    {
        print_error("In macro: %list_pop: The specified list does not exist.");
        return "";
    }
    
    int id = p[0];
    lists[id].pop_back();
    
    return "";
}

string Mml::list_top(const std::vector<int> &p)
{
    if (p.size() != 1)
    {
        print_error("In macro: %list_top: Incorrect number of parameters.");
        return "";
    }
    
    if (find_list_id(p[0]) == -1)
    {
        print_error("In macro: %list_top: The specified list does not exist.");
        return "";
    }
    
    int id = p[0];
    return num_to_string(lists[id][lists[id].size() - 1]);
}

string Mml::list_get(const vector<int>& p)
{
    if (p.size() != 2)
    {
        print_error("In macro: %list_get: Incorrect number of parameters.");
        return "";
    }
    
    if (find_list_id(p[0]) == -1)
    {
        print_error("In macro: %list_get: The specified list does not exist.");
        return "";
    }
    
    int id = p[0];
    int index = p[1];
    
    if (index < 0 || index >= lists[id].size())
    {
        print_error("In macro: %list_get: Index out of range.");
        return "";
    }

    return num_to_string(lists[id][index]);
}

string Mml::list_set(const vector<int>& p)
{
    if (p.size() != 3)
    {
        print_error("In macro: %list_set: Incorrect number of parameters.");
        return "";
    }
    
    if (find_list_id(p[0]) == -1)
    {
        print_error("In macro: %list_set: The specified list does not exist.");
        return "";
    }
    
    int id = p[0];
    int index = p[1];
    int val = p[2];
    
    if (index < 0 || index >= lists[id].size())
    {
        print_error("In macro: %list_set: Index out of range.");
        return "";
    }
    
    lists[id][index] = val;
    return "";
}

string Mml::list_size(const vector<int>& p)
{
    if (p.size() != 1)
    {
        print_error("In macro: %list_size: Incorrect number of parameters.");
        return "";
    }
    
    if (find_list_id(p[0]) == -1)
    {
        print_error("In macro: %list_size: The specified list does not exist.");
        return "";
    }
    
    int id = p[0];
    return num_to_string(lists[id].size());
}

string Mml::list(const vector<int>& p)
{
    if (p.size() != 1)
    {
        print_error("In macro: %list: Incorrect number of parameters.");
        return "";
    }
    
    if (find_list_id(p[0]) == -1)
    {
        print_error("In macro: %list: The specified list does not exist.");
        return "";
    }
    
    int id = p[0];
    
    string s;
    
    for (unsigned i = 0; i < lists[id].size(); i++)
    {
        s += num_to_string(lists[id][i]);
        if (i < lists[id].size() - 1)
            s += ",";
    }
    
    return s;
}

string Mml::list_create(const vector<int>& p)
{
    if (p.size() == 0)
    {
        print_error("In macro: %list_create: Not enough parameters.");
        return "";
    }
    
    string s;
    
    unsigned slot = 0;
    for (; find_list_id(slot) != -1; slot++); // Find empty slot
    listIDs.insert(make_pair(slot, 0));
    
    s = num_to_string(slot); // Return the new list ID.
    
    if (slot >= lists.size())
    {
        while (slot >= lists.size())
            lists.push_back(p);
    }
    else
        lists[slot] = p;
    
    return s;
}

string Mml::list_destroy(const vector<int>& p)
{
    if (p.size() != 1)
    {
        print_error("In macro: %list_destroy: Incorrect number of parameters.");
        return "";
    }
    
    int id = p[0];
    int foundit = find_list_id(id);
    
    if (foundit != -1)
    {
        lists[id].clear();
        listIDs.erase(id);
    }
    else
    {
        print_error("In macro: %list_destroy: The specified list does not exist.");
        return "";
    }
    
    return "";
}

string Mml::list_exists(const vector<int>& p)
{
    if (p.size() != 1)
    {
        print_error("In macro: %list_destroy: Incorrect number of parameters.");
        return "";
    }
    
    int id = p[0];
    
    if (find_list_id(id) != -1)
        return "1";
    else
        return "0";
}

int Mml::find_list_id(unsigned n)
{
    if (listIDs.find(n) != listIDs.end())
        return 1;
    
    return -1;
}

bool Mml::expand_kits()
{
    string newData;
    int oct = 4;
    int art = 127;
    int prevIns = -1;
    int prevArt = -1;
    bool isKit = false;
    string kitStr;
    
    bool isHex = false;
    bool isLabel = false;
    
    for (unsigned i = 0; i < data.size(); i++)
    {
        if (data[i] == '#')
        {
            newData += enter_file(data, i);
        }
    
        if (data[i] == '\n')
        {
            lineNum++;
            newData += '\n';
            continue;
        }
        
        if (data[i] == '@')
        {
            kitStr = "";
        
            for (i++; i < data.size() && !is_whitespace(data[i]); i++)
            {
                kitStr += data[i];
            }
            
            i--;
            
            if (kitStr != "" && !is_digit(kitStr[0]))
            {
                if (kitStr == "nokit")
                {
                    oct = 4;
                    art = 127;
                    prevIns = -1;
                    prevArt = -1;
                    
                    isKit = false;
                    
                    continue;
                }
                
                if (kits.find(kitStr) == kits.end())
                {
                    print_error(string("Undefined kit: \"") + kitStr + "\".");
                    return false;
                }
                
                isKit = true;
            }
            else
            {
                newData += "@" + kitStr;
                
                oct = 4;
                art = 127;
                prevIns = -1;
                prevArt = -1;
                
                isKit = false;
                
                continue;
            }
        }
        else if (data[i] == '#')
        {
            newData += data[i];
            
            oct = 4;
            art = 127;
            prevIns = -1;
            prevArt = -1;
            
            isKit = false;
            
            continue;
        }
        else if (!isKit)
        {
            newData += data[i];
            continue;
        }
        else if (isKit)
        {
            char chr = tolower(data[i]);
            
            if (isHex && (!is_digit(chr)) && (chr < 'a' || chr > 'z'))
            {
                isHex = false;
            }
            
            switch(chr)
            {
                case 'a':
                case 'b':
                case 'c':
                case 'd':
                case 'e':
                case 'f':
                case 'g':
                {
                    if (!isHex && !isLabel)
                    {
                        int noteVal = NOTE_TABLE[chr - 'a'];

                        i++;
                        
                        if (i < data.size() && (data[i] == '+' || data[i] == '-'))
                        {
                            noteVal++;
                        }
                        else
                        {
                            i--;
                        }
                        
                        noteVal += ((oct - 1) * 12);
                        string newNote;
                        newNote += " " + kits[kitStr][noteVal % 72];
                        
                        // Remove duplicate instruments and fix q command
                        string newNote2;
                        
                        for (unsigned i = 0; i < newNote.size(); i++)
                        {
                            if (newNote[i] == '@')
                            {
                                string iStr;
                                
                                for (i++; i < newNote.size() && is_digit(newNote[i]); i++)
                                {
                                    iStr += newNote[i];
                                }
                                
                                i--;
                                
                                if (is_integer(iStr) && string_to_num(iStr) != prevIns)
                                {
                                    newNote2 += '@' + iStr;
                                    prevIns = string_to_num(iStr);
                                }
                            }
                            else if (newNote[i] == 'q')
                            {
                                string artStr;
                                
                                for (i++; i < newNote.size() && (is_digit(newNote[i]) || (tolower(newNote[i]) >= 'a' && tolower(newNote[i]) <= 'f')); i++)
                                {
                                    artStr += newNote[i];
                                }
                                
                                i--;
                                
                                int newArt = hex_to_dec(artStr);
                                
                                int newLegato = ((newArt & ~15) & 255) / 16;
                                int newVelocity = newArt & 15;
                                
                                int legato = ((art & ~15) & 255) / 16;
                                int velocity = art & 15;
                                
                                newArt = (((legato * newLegato) / 7) * 16) | ((velocity * newVelocity) / 15);
                                
                                if (newArt != prevArt)
                                {
                                    ostringstream ssout;
                                    ssout << uppercase << hex << newArt;
                                    string newArtStr = ssout.str();
                                    
                                    newNote2 += 'q' + newArtStr;
                                    prevArt = newArt;
                                }
                            }
                            else
                            {
                                newNote2 += newNote[i];
                            }
                        }
                        
                        newData += newNote2;
                    }
                    else
                    {
                        newData += data[i];
                    }
                
                    break;
                }
                
                case 'o':
                {
                    if (!isLabel)
                    {
                        string octStr;
                        for (i++; i < data.size() && is_digit(data[i]); i++)
                        {
                            octStr += data[i];
                        }
                        
                        i--;
                        
                        if (is_integer(octStr))
                        {
                            oct = string_to_num(octStr);
                        }
                    }
                    else
                    {
                        newData += data[i];
                    }
                
                    break;
                }
                
                case '<':
                {
                    oct--;
                    break;
                }
                
                case '>':
                {
                    oct++;
                    break;
                }
                
                case '(':
                {
                    isLabel = true;
                    newData += data[i];
                    break;
                }
                
                case ')':
                {
                    isLabel = false;
                    newData += data[i];
                    break;
                }
                
                case '$':
                {
                    isHex = true;
                    newData += data[i];
                    break;
                }
                
                case 'q':
                {
                    string artStr;
                
                    for (i++; i < data.size() && (is_digit(data[i]) || (tolower(data[i]) >= 'a' && tolower(data[i]) <= 'f')); i++)
                    {
                        artStr += data[i];
                    }
                    
                    i--;
                    
                    art = hex_to_dec(artStr);
                    
                    break;
                }
                
                default:
                {
                    newData += data[i];
                    break;
                }
            }
        }
    }
    
    swap(data, newData); // Copies contents of newData to data by pointer.
    return true;
}

void Mml::init()
{
    octave = 4;
    transpose = 0;
    defaultLength = 16;
    prevTicks = 0xFF;
    dynamic = 0x7F;
    prevDynamic = 0xFF;
}

void Mml::pattern_init()
{
    prevTicks = 0xFF;
    prevDynamic = 0xFF;
}

int Mml::note_to_num(string note)
{
    if (tolower(note[0]) < 'a' || tolower(note[0]) > 'g' || note.size() > 2)
    {
        return -1; // -1 indicates some sort of error.
    }

    int noteVal;

    noteVal = NOTE_TABLE[tolower(note[0]) - 'a'];

    if (noteVal < 0)
        noteVal += 12;

    if (note.size() > 1)
    {
        if (note[1] == '+')
            noteVal++;
        else if (note[1] == '-')
            noteVal--;
        else
        {
            return -1;
        }
    }

    return noteVal;
}

int Mml::char_to_num(string chr)
{
    if (chr[0] != '\'' || chr[chr.size() - 1] != '\'' || chr.size() < 3 || chr[1] == '\'')
    {
        return -1; // -1 indicates some sort of error.
    }
    
    return (int)(chr[1]);
}

bool Mml::is_integer(const string& s)
{
    for (unsigned i = 0; i < s.size(); i++)
    {
        if (s[i] < '0' || s[i] > '9')
        {
            if (i != 0 || s[i] != '-' || s.size() == 1)
                return false;
        }
    }

    if (s != "")
        return true;
    else
        return false;
}

bool Mml::is_whitespace(char c)
{
    /*if (c == '\n')
        lineNum++;*/
    return (c == ' ' || c == '\t' || c == '\r' || c == '\n' || c == '\\');
}

bool Mml::is_bit(char c)
{
    return (c == '0' || c == '1');
}

bool Mml::is_digit(char c)
{
    return (c >= '0' && c <= '9');
}

bool Mml::is_nybble(char c)
{
    return (c >= '0' && c <= '9') || (toupper(c) >= 'A' && toupper(c) <= 'F');
}

int Mml::string_to_num(const string& s)
{
    int n;
    istringstream ssin;
    ssin.str(s);
    ssin >> n;

    return n;
}

string Mml::num_to_string(int n)
{
    ostringstream ssout;
    ssout << n;
    return ssout.str();
}

unsigned Mml::hex_to_dec(const string& hex)
{
    unsigned dec = 0;

    for (int i = hex.size() - 1; i >= 0; i--)
    {
        string nybble;
        nybble += toupper(hex[i]);
        unsigned digit;

        if (is_digit(nybble[0]))
        {
            digit = string_to_num(nybble);
        }
        else if (is_nybble(nybble[0]))
        {
            digit = nybble[0] - 'A' + 10;
        }
        else
        {
            return 0;
        }

        dec += (digit << (4 * (hex.size() - i - 1)));
    }

    return dec;
}

unsigned Mml::bin_to_dec(const string& bin)
{
    unsigned dec = 0;

    for (int i = bin.size() - 1; i >= 0; i--)
    {
        string bit;
        bit += bin[i];
        unsigned digit;

        if (is_bit(bit[0]))
        {
            digit = string_to_num(bit);
        }
        else
        {
            return 0;
        }

        dec += (digit << (bin.size() - i - 1));
    }

    return dec;
}

bool Mml::iterate(char& chr, unsigned& i)
{
    i++;

    if (i >= data.size())
        return false;

    chr = tolower(data[i]);

    return true;
}

void Mml::update_line(char& c)
{
    if (c == '\n')
        lineNum++;
}

unsigned Mml::get_line_num(const std::string& dt, unsigned index)
{
    lineNum = 1;

    for (unsigned i = 0; i < index; i++)
    {
        if (dt[i] == '#')
        {
            enter_file(dt, i);
        }
    
        if (dt[i] == '\n')
            lineNum++;
    }
    
    return lineNum;
}

void Mml::print_error(const string& s)
{
    isError = true;
    
    cout << dec << "Error: ";
    if (file != "")
        cout << "In file \"" << file << "\": ";
    cout << "Line " << lineNum << ": " << s << endl;
}

void Mml::print_warning(const string& s)
{
    cout << dec << "Warning: ";
    if (file != "")
        cout << "In file \"" << file << "\": ";
    cout << "Line " << lineNum << ": " << s << endl;
}

int Mml::get_parameter(unsigned paramNum, Parameter type, char& chr, unsigned& i)
{
    string paramStr;
    int num;

    if (paramNum > 0)
    {
        if (chr != ',')
        {
            string err = "Not enough parameters for command \'";
            print_error(err + command + "\'.");
            return 0;
        }
    }

    if (!iterate(chr, i))
    {
        print_error(END_OF_FILE);
        return 0;
    }

    if ((type & SIGNED) && ((type & (~1)) == DECIMAL))
    {
        while (is_digit(chr) || chr == '-')
        {
            paramStr += chr;
            if (!iterate(chr, i))
            {
                break;
            }
        }

        if (paramStr == "")
        {
            print_error(NO_VALUE);
            return 0;
        }
        else if (!is_integer(paramStr))
        {
            print_error(INVALID_VALUE);
            return 0;
        }

        num = string_to_num(paramStr);

        if (num < -128 || num > 127)
        {
            string err = "Parameter's value is out of range in command \'";
            print_error(err + command + "\'.");
        }
    }
    else if (!(type & SIGNED))
    {
        if ((type & (~1)) == DECIMAL)
        {
            while (is_digit(chr))
            {
                paramStr += chr;
                if (!iterate(chr, i))
                {
                    break;
                }
            }

            if (paramStr == "")
            {
                print_error(NO_VALUE);
                return 0;
            }

            num = string_to_num(paramStr);
        }
        else if ((type & (~1)) == HEXADECIMAL)
        {
            while (is_nybble(chr))
            {
                paramStr += chr;
                if (!iterate(chr, i))
                {
                    break;
                }
            }

            if (paramStr == "")
            {
                print_error(NO_VALUE);
                return 0;
            }

            num = (signed)hex_to_dec(paramStr);
        }
        else if ((type & (~1)) == BINARY)
        {
            while (is_bit(chr))
            {
                paramStr += chr;
                if (!iterate(chr, i))
                {
                    break;
                }
            }

            if (paramStr == "")
            {
                print_error(NO_VALUE);
                return 0;
            }

            num = (signed)bin_to_dec(paramStr);
        }

        if (num < 0 || num > 255)
        {
            string err = "Parameter's value is out of range in command \'";
            print_error(err + command + "\'.");
        }
    }

    return num;
}

std::string Mml::remove_whitespace(std::string s)
{
    string ret;
    
    // Strip whitespace from beginning and end.
    while (s.size() > 0 && is_whitespace(s[0]))
        s = s.substr(1);
    while (s.size() > 0 && is_whitespace(s[s.size() - 1]))
        s = s.substr(0, s.size() - 1);
        
    // For the middle part of the string, only remove consecutive whitespaces.
    for (unsigned i = 0; i < s.size(); i++)
    {
        if (is_whitespace(s[i]))
        {
            if (i > 0 && !is_whitespace(s[i - 1]))
                ret += ' ';
        }
        else
        {
            ret += s[i];
        }
    }
    
    return ret;
}

string Mml::strip_newlines(string s)
{
    string newStr;
    
    for (unsigned i = 0; i < s.size(); i++)
    {
        if (s[i] != '\n')
            newStr += s[i];
        else
            newStr += "\\";
    }
    
    return newStr;
}

string Mml::enter_file(const std::string& dt, unsigned& i)
{
    string newData;

    if (i < dt.size() - 6 && dt.substr(i, 6) == "#file=")
    {
        string fname;
        newData += dt.substr(i, 7);
    
        for (i += 7; i < dt.size() && dt[i] != '\"'; i++)
        {
            fname += dt[i];
            newData += dt[i];
        }
        
        newData += dt[i];
        i++; // iterate past the newline
        newData += dt[i];
        i++;
        
        lineStack.push_back(lineNum);
        fileStack.push_back(file);
        
        lineNum = 1; // I guess the line count was wrong? Fuck it, this fixes the problem.
        file = fname;
    }
    else if (i < dt.size() - 8 && dt.substr(i, 8) == "#endfile")
    {
        newData += dt.substr(i, 8);
        i += 8; // iterate past the newline
        lineNum = lineStack[lineStack.size() - 1] - 1; // Subtract 1 to compensate for the mysterious case of mismatched line numbers.
        lineStack.pop_back();
        file = fileStack[fileStack.size() - 1];
        fileStack.pop_back();
    }
    
    return newData;
}

string Mml::include_binary(string filename)
{
    string binData;
    unsigned numBytes = 0;
    
    FILE* pFile;
    pFile = fopen(filename.c_str(), "rb");
    
    if (pFile == NULL)
    {
        print_error("The file: \"" + filename + "\" cannot be found.");
        return "";
    }
    
    byte buffer[1];
    int bytesRead = fread(buffer, 1, 1, pFile);
    
    while (bytesRead > 0)
    {
        numBytes++;
        ostringstream ssout;
        ssout << '$' << setw(2) << setfill('0') << uppercase << hex << (unsigned)(buffer[0]);
        
        binData += ssout.str();
        if (numBytes % 16 == 0)
            binData += "\\";
        else
            binData += " ";
        
        bytesRead = fread(buffer, 1, 1, pFile);
    }
    
    fclose(pFile);
    
    return binData;
}
