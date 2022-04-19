#include "macros.h"

#include <iostream>
#include <iomanip>
#include <vector>
#include <map>
#include <sstream>
#include <cmath>
#include <stdlib.h>

using namespace std;

bool Macro::is_error()
{
    return (error != "");
}

bool Macro::has_braces()
{
    return hasBraces;
}

void Macro::clear_error()
{
    error = "";
}

string Macro::get_implicit_params()
{
    return implicitParams;
}

string Macro::get_error()
{
    return string("In macro %") + name + ": " + error;
}

int Macro::string_to_num(const string& s)
{
    for (unsigned i = 0; i < s.size(); i++)
    {
        if (i == 0 && s[i] == '-')
        {
        }
        else if (s[i] < '0' || s[i] > '9')
        {
            error = string("Could not convert \"") + s + "\" to an integer.";
            return 0;
        }
    }

    int n = 0;
    istringstream ssin;
    ssin.str(s);
    ssin >> n;

    return n;
}

string Macro::num_to_string(int n)
{
    ostringstream ssout;
    ssout << n;
    return ssout.str();
}

string Macro::dec_to_hex(int n)
{
    ostringstream ssout;
    ssout << uppercase << hex << n;
    return ssout.str();
}

int Macro::bin_to_dec(const string& bin)
{
    int dec = 0;

    for (int i = bin.size() - 1; i >= 0; i--)
    {
        string bit;
        bit += bin[i];
        int digit;

        if (bit[0] == '0' || bit[0] == '1')
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

unsigned Macro::hex_to_dec(const string& hex)
{
    unsigned dec = 0;

    for (int i = hex.size() - 1; i >= 0; i--)
    {
        string nybble;
        nybble += toupper(hex[i]);
        unsigned digit;

        if (nybble[0] >= '0' && nybble[0] <= '9')
        {
            digit = string_to_num(nybble);
        }
        else if ((nybble[0] >= '0' && nybble[0] <= '9') || (tolower(nybble[0]) >= 'a' && tolower(nybble[0]) <= 'f'))
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

string Macro::char_to_string(char c)
{
    char charray[2] = { c, '\0' };
    return string(charray);
}

Scale::Scale()
{
    Macro::hasBraces = false;
    Macro::name = "scale";
    Macro::implicitParams = "";
}

string Scale::macro(map<string, string> &v, const vector<int>& p, string data)
{
    data = "";


    if (p.size() != 6)
    {
        error = "Incorrect number of parameters.";
        return "";
    }
    else
    {
        key = p[0];
        offset = p[1];
        direction = p[2];
        type = (Scale::Type)p[3];
        noteLength = p[4];
        numNotes = p[5];

        if (direction != 0 && direction != 1)
        {
            error = "Direction must be either 0 or 1.";
            return "";
        }
        else if (type < 0 || type > 6)
        {
            error = "Invalid scale type.";
            return "";
        }
        else if (noteLength <= 0 || noteLength > 192)
        {
            error = "Invalid note length.";
            return "";
        }
        else if (numNotes < 0)
        {
            error = "The number of notes must be positive.";
            return "";
        }
        else if (numNotes > 72)
        {
            error = "Scales may not be longer than 72 notes.";
            return "";
        }
    }

    string s;

    int major[7] = { 0, 2, 4, 5, 7, 9, 11 };
    int minorHarm[7] = { 0, 2, 3, 5, 7, 8, 11 };
    int majorPent[5] = { 0, 2, 5, 7, 9 };
    int minorPent[5] = { 0, 3, 5, 8, 10 };
    int chromatic[12] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 };
    int wholeTone[6] = { 0, 2, 4, 6, 8, 10 };
    int minorMel[7] = { 0, 2, 3, 5, 7, 9, 11 };

    int octave = 7;
    int prevOctave = octave;
    int initOctave = octave;
    const int BASE_NOTE = 84; // This is to prevent the note or octave from ever getting below 0; makes calculations easier

    for (int i = 0; i < numNotes; i++)
    {
        int note = BASE_NOTE;
        int oct = 0;
        int interval = offset;
        prevOctave = octave;

        switch (type)
        {
            case MAJOR:
            {
                if (interval < 0)
                    oct = ((interval - 6) / 7);
                else
                    oct = interval / 7;

                while (interval < 0)
                    interval += 7;

                note = BASE_NOTE + key + major[interval % 7] + oct * 12;
                break;
            }

            case HARMONIC_MINOR:
            {
                if (interval < 0)
                    oct = ((interval - 6) / 7);
                else
                    oct = interval / 7;

                while (interval < 0)
                    interval += 7;

                note = BASE_NOTE + key + minorHarm[interval % 7] + oct * 12;
                break;
            }

            case MAJOR_PENTATONIC:
            {
                if (interval < 0)
                    oct = ((interval - 4) / 5);
                else
                    oct = interval / 5;

                while (interval < 0)
                    interval += 5;

                note = BASE_NOTE + key + majorPent[interval % 5] + oct * 12;
                break;
            }

            case MINOR_PENTATONIC:
            {
                if (interval < 0)
                    oct = ((interval - 4) / 5);
                else
                    oct = interval / 5;

                while (interval < 0)
                    interval += 5;

                note = BASE_NOTE + key + minorPent[interval % 5] + oct * 12;
                break;
            }

            case CHROMATIC:
            {
                if (interval < 0)
                    oct = ((interval - 11) / 12);
                else
                    oct = interval / 12;

                while (interval < 0)
                    interval += 12;

                note = BASE_NOTE + key + chromatic[interval % 12] + oct * 12;
                break;
            }

            case WHOLE_TONE:
            {
                if (interval < 0)
                    oct = ((interval - 5) / 6);
                else
                    oct = interval / 6;

                while (interval < 0)
                    interval += 6;

                note = BASE_NOTE + key + wholeTone[interval % 6] + oct * 12;
                break;
            }

            case MELODIC_MINOR:
            {
                if (interval < 0)
                    oct = ((interval - 6) / 7);
                else
                    oct = interval / 7;

                while (interval < 0)
                    interval += 7;

                note = BASE_NOTE + key + minorMel[interval % 7] + oct * 12;
                break;
            }

            default:
                note = 0;
                break;
        }

        if (note < 0)
        {
            error = "Interval offset is too low.";
            return "";
        }
        else if (note > 2 * BASE_NOTE)
        {
            error = "Interval offset is too high.";
            return "";
        }

        octave = note / 12;
        note %= 12;

        if (octave > prevOctave)
        {
            s += string(octave - prevOctave, '>');
        }
        else if (octave < prevOctave)
        {
            s += string(prevOctave - octave, '<');
        }

        ostringstream sout;
        sout << noteLength;
        s += NOTE_STRINGS[note] + "=" + sout.str();

        if (direction == 0)
            offset++;
        else
            offset--;
    }

    // Preserve the initial octave
    if (initOctave > octave)
    {
        s += string(initOctave - octave, '>');
    }
    else if (initOctave < octave)
    {
        s += string(octave - initOctave, '<');
    }
    //

    return s;
}

Arp::Arp()
{
    Macro::hasBraces = false;
    Macro::name = "arp";
    Macro::implicitParams = "";
}

string Arp::macro(map<string, string> &v, const vector<int>& p, string data)
{
    data = "";

    if (p.size() < 3)
    {
        error = "Not enough parameters.";
        return "";
    }
    else
    {
        root = p[0];
        noteLength = p[1];

        for (unsigned i = 2; i < p.size(); i++)
        {
            offsets.push_back(p[i]);
        }

        if (noteLength <= 0 || noteLength > 192)
        {
            error = "Invalid note length.";
            return "";
        }
    }

    string s;

    int octave = 7;
    int prevOctave = octave;
    int initOctave = octave;
    const int BASE_NOTE = 84; // This is to prevent the note or octave from ever getting below 0; makes calculations easier

    for (unsigned i = 0; i < offsets.size(); i++)
    {
        int note = BASE_NOTE;
        int interval = offsets[i];
        prevOctave = octave;

        note = BASE_NOTE + root + interval;

        if (note < 0)
        {
            error = "Interval offset is too low.";
            return "";
        }
        else if (note > 2 * BASE_NOTE)
        {
            error = "Interval offset is too high.";
            return "";
        }

        octave = note / 12;
        note %= 12;

        if (octave > prevOctave)
        {
            s += string(octave - prevOctave, '>');
        }
        else if (octave < prevOctave)
        {
            s += string(prevOctave - octave, '<');
        }

        ostringstream sout;
        sout << noteLength;
        s += NOTE_STRINGS[note] + "=" + sout.str();
    }

    // Preserve the initial octave
    if (initOctave > octave)
    {
        s += string(initOctave - octave, '>');
    }
    else if (initOctave < octave)
    {
        s += string(octave - initOctave, '<');
    }
    //

    offsets.clear(); // Free up note data so that this macro can be used properly again.
    return s;
}

LoopM::LoopM()
{
    Macro::hasBraces = true;
    Macro::name = "loop";
    Macro::implicitParams = "?o, ?h, ";
}

string LoopM::macro(map<string, string> &v, const vector<int>& p, string data)
{
    if (p.size() != 3)
    {
        error = "Incorrect number of parameters.";
        return "";
    }
    else
    {
        initOctave = p[0];
        initTranspose = p[1];
        iterations = p[2];

        if (iterations < 0)
        {
            error = "Invalid loop count.";
            return "";
        }
    }

    string s;
    string loopData;
    unsigned numNewlines = 0;
    bool isLabelData = false;

    loopData += "o" + num_to_string(initOctave) + " h" + num_to_string(initTranspose) + " ";

    for (unsigned i = 0; i < data.size(); i++)
    {
        if (data[i] == '\n' || data[i] == '\r')
        {
            s += data[i];
            loopData += " "; // Replace newlines with spaces; we don't want to change the number of lines as that would potentially lead to misleading error messages.

            if (data[i] == '\n')
            {
                numNewlines++;
            }
        }
        else
        {
            if (i < data.size() - 1)
            {
                if ((data[i] != '{' || data[i + 1] != '{') && (data[i] != '}' || data[i + 1] != '}'))
                {
                    s += data[i];

                    // Don't repeat label definitions, but call them.
                    if (!isLabelData)
                    {
                        loopData += data[i];

                        if (data[i] == ')' && data[i + 1] == '[')
                        {
                            isLabelData = true;
                        }
                    }
                    else
                    {
                        if (data[i] == ']')
                        {
                            isLabelData = false;
                        }
                    }
                }
                else
                {
                    i++; // Skip an iteration
                }
            }
        }
    }

    v["loopData"] = loopData;

    if (iterations > 0)
    {
        for (int i = 1; i < iterations; i++)
        {
             s += " " + loopData;
        }
    }
    else
    {
        s = string(numNewlines, '\n');
    }

    return s;
}

Double::Double()
{
    Macro::hasBraces = true;
    Macro::name = "double";
    Macro::implicitParams = "";
}

string Double::macro(map<string, string> &v, const vector<int>& p, string data)
{
    if (p.size() != 0)
    {
        error = "Incorrect number of parameters.";
        return "";
    }

    string s;
    bool isLabelName = false;
    bool isHex = false;
    bool isTriplet = false;

    for (unsigned i = 0; i < data.size(); i++)
    {
        bool isBrace = false;

        if (i < data.size() - 1)
        {
            if ((data[i] != '{' || data[i + 1] != '{') && (data[i] != '}' || data[i + 1] != '}'))
            {
                s += data[i];
                isBrace = false;
            }
            else
            {
                i++; // Skip an iteration
                isBrace = true;
            }
        }
        else
        {
            s += data[i];
            isBrace = false;
        }

        if (isHex)
        {
            if ((data[i] < '0' || data[i] > '9') && (tolower(data[i]) < 'a' || tolower(data[i]) > 'f'))
            {
                isHex = false;
            }
        }

        if (!isBrace)
        {
            switch (tolower(data[i]))
            {
                case '{':
                {
                    isTriplet = true;
                    break;
                }

                case '}':
                {
                    isTriplet = false;
                    break;
                }

                case '(':
                {
                    isLabelName = true;
                    break;
                }

                case ')':
                {
                    isLabelName = false;
                    break;
                }

                case '$':
                case 'q':
                {
                    isHex = true;
                    break;
                }
                
                case '~':
                {
                    i++;
                    s += data[i];
                    
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
                    // Don't consider these as actual notes if inside a label or if the letter is actually hexadecimal
                    if (!isLabelName && !isHex)
                    {
                        i++;

                        string lenStr;
                        string dotStr;
                        bool isTicks = false;

                        while (i < data.size())
                        {
                            if ((data[i] < '0' || data[i] > '9') && data[i] != '+' && data[i] != '-' && data[i] != '=' && data[i] != '.')
                            {
                                i--;
                                break;
                            }

                            if (data[i] == '=')
                            {
                                isTicks = true;
                            }
                            else if ((data[i] >= '0' && data[i] <= '9') || data[i] == '.')
                            {
                                if (data[i] != '.')
                                    lenStr += data[i];
                                else
                                    dotStr += data[i];
                            }
                            else
                            {
                                s += data[i];
                            }

                            i++;
                        }
                        
                        if (lenStr != "")
                        {
                            s += '=';
    
                            unsigned len = string_to_num(lenStr);
    
                            if (!isTicks)
                            {
                                len = 192 / len;
                                unsigned dotLen = len;
    
                                for (unsigned j = 0; j < dotStr.size(); j++)
                                {
                                    dotLen /= 2;
                                    len += dotLen;
                                }
    
                                if (isTriplet)
                                    len = (len * 2) / 3;
                            }
    
                            len *= 2; // double the note length.
    
                            // Split the length into a tied note if it's too long.
                            if (len > 192)
                            {
                                unsigned len2 = len / 2;
                                len -= len2;
    
                                ostringstream ssout;
                                ssout << len << "^=" << len2;
                                s += ssout.str();
                            }
                            else
                            {
                                ostringstream ssout;
                                ssout << len;
                                s += ssout.str();
                            }
                        }
                    }

                    break;
                }

                case 'l':
                {
                    // Don't consider these as actual notes if inside a label
                    if (!isLabelName)
                    {
                        i++;

                        string lenStr;

                        while (i < data.size())
                        {
                            if (data[i] < '0' || data[i] > '9')
                                break;

                            if (data[i] >= '0' && data[i] <= '9')
                            {
                                lenStr += data[i];
                            }
                            else
                            {
                                s += data[i];
                            }

                            i++;
                        }
                        
                        i--;

                        unsigned len = string_to_num(lenStr);
                        len /= 2; // Since this is a fractional length, dividing by two doubles it.

                        ostringstream ssout;
                        ssout << len;
                        s += ssout.str();
                    }

                    break;
                }

                case 'm':
                case 'n':
                case 'p':
                case 'u':
                case '&':
                {
                    if (!isLabelName && !isHex)
                    {
                        char command = tolower(data[i]);

                        i++;

                        string params[3];
                        unsigned p = 0;

                        while (i < data.size())
                        {
                            if ((data[i] >= '0' && data[i] <= '9') || data[i] == ',' || data[i] == '-')
                            {
                                if (data[i] != ',' && p < 3)
                                {
                                    params[p] += data[i];
                                }
                                else
                                {
                                    p++;
                                }
                            }
                            else
                            {
                                break;
                            }

                            i++;
                        }

                        i--;

                        if (command == 'm' || command == 'n' || command == '&')
                        {
                            int delay = string_to_num(params[0]) * 2;
                            int length = string_to_num(params[1]) * 2;

                            if (delay > 255)
                                delay = 255;
                            if (length > 255)
                                length = 255;

                            s += num_to_string(delay) + "," + num_to_string(length) + "," + params[2];
                        }
                        else if (command == 'p' || command == 'u')
                        {
                            int delay = string_to_num(params[0]) * 2;
                            int freq = string_to_num(params[1]) / 2;

                            if (delay > 255)
                                delay = 255;
                            if (freq == 0)
                                freq = 1;

                            s += num_to_string(delay) + "," + num_to_string(freq) + "," + params[2];
                        }
                    }

                    break;
                }

                case 's':
                case 't':
                {
                    if (!isLabelName && !isHex)
                    {
                        i++;

                        string lenStr;

                        while (i < data.size())
                        {
                            if (data[i] >= '0' && data[i] <= '9')
                            {
                                lenStr += data[i];
                            }
                            else
                            {
                                break;
                            }

                            i++;
                        }

                        i--;

                        int length = string_to_num(lenStr) * 2;
                        if (length > 255)
                            length = 255;

                        s += num_to_string(length);
                    }

                    break;
                }

            }
        }
    }

    return s;
}

Half::Half()
{
    Macro::hasBraces = true;
    Macro::name = "half";
    Macro::implicitParams = "";
}

string Half::macro(map<string, string> &v, const vector<int>& p, string data)
{
    if (p.size() != 0)
    {
        error = "Incorrect number of parameters.";
        return "";
    }

    string s;
    bool isLabelName = false;
    bool isHex = false;
    bool isTriplet = false;

    float tickDebt = 0.0;

    for (unsigned i = 0; i < data.size(); i++)
    {
        bool isBrace = false;

        if (i < data.size() - 1)
        {
            if ((data[i] != '{' || data[i + 1] != '{') && (data[i] != '}' || data[i + 1] != '}'))
            {
                s += data[i];
                isBrace = false;
            }
            else
            {
                i++; // Skip an iteration
                isBrace = true;
            }
        }
        else
        {
            s += data[i];
            isBrace = false;
        }

        if (isHex)
        {
            if ((data[i] < '0' || data[i] > '9') && (tolower(data[i]) < 'a' || tolower(data[i]) > 'f'))
            {
                isHex = false;
            }
        }

        if (!isBrace)
        {
            switch (tolower(data[i]))
            {
                case '{':
                {
                    isTriplet = true;
                    break;
                }

                case '}':
                {
                    isTriplet = false;
                    break;
                }

                case '(':
                case '@':
                {
                    isLabelName = true;
                    break;
                }

                case ')':
                {
                    isLabelName = false;
                    break;
                }

                case '$':
                case 'q':
                {
                    isHex = true;
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
                    // Don't consider these as actual notes if inside a label
                    if (!isLabelName && !isHex)
                    {
                        i++;

                        string lenStr;
                        string dotStr;
                        bool isTicks = false;

                        while (i < data.size())
                        {
                            if ((data[i] < '0' || data[i] > '9') && data[i] != '+' && data[i] != '-' && data[i] != '=' && data[i] != '.')
                            {
                                i--;
                                break;
                            }

                            if (data[i] == '=')
                            {
                                isTicks = true;
                            }
                            else if ((data[i] >= '0' && data[i] <= '9') || data[i] == '.')
                            {
                                if (data[i] != '.')
                                    lenStr += data[i];
                                else
                                    dotStr += data[i];
                            }
                            else
                            {
                                s += data[i];
                            }

                            i++;
                        }
                        
                        if (lenStr != "")
                        {
                            s += '=';
                        
                            unsigned len = string_to_num(lenStr);
                            
                            if (!isTicks)
                            {
                                len = 192 / len;
                                unsigned dotLen = len;
    
                                for (unsigned j = 0; j < dotStr.size(); j++)
                                {
                                    dotLen /= 2;
                                    len += dotLen;
                                }
    
                                if (isTriplet)
                                    len = (len * 2) / 3;
                            }
    
                            if (len %2 != 0)
                            {
                                tickDebt += 0.5;
                            }
    
                            len /= 2; // half the note length.
    
                            if (tickDebt == 1.0)
                            {
                                len += 1;
                                tickDebt = 0.0;
                            }
    
                            ostringstream ssout;
                            ssout << len;
                            s += ssout.str();
                        }
                    }

                    break;
                }

                case 'l':
                {
                    // Don't consider these as actual notes if inside a label
                    if (!isLabelName)
                    {
                        i++;

                        string lenStr;

                        while (i < data.size())
                        {
                            if (data[i] < '0' || data[i] > '9')
                                break;

                            if (data[i] >= '0' && data[i] <= '9')
                            {
                                lenStr += data[i];
                            }
                            else
                            {
                                s += data[i];
                            }

                            i++;
                        }
                        
                        i--;

                        unsigned len = string_to_num(lenStr);
                        len *= 2; // Since this is a fractional length, multiplying by two halves it.

                        ostringstream ssout;
                        ssout << len;
                        s += ssout.str();
                    }

                    break;
                }

                case 'm':
                case 'n':
                case 'p':
                case 'u':
                case '&':
                {
                    if (!isLabelName && !isHex)
                    {
                        char command = tolower(data[i]);

                        i++;

                        string params[3];
                        unsigned p = 0;

                        while (i < data.size())
                        {
                            if ((data[i] >= '0' && data[i] <= '9') || data[i] == ',' || data[i] == '-')
                            {
                                if (data[i] != ',' && p < 3)
                                {
                                    params[p] += data[i];
                                }
                                else
                                {
                                    p++;
                                }
                            }
                            else
                            {
                                break;
                            }

                            i++;
                        }

                        i--;

                        if (command == 'm' || command == 'n' || command == '&')
                        {
                            int delay = string_to_num(params[0]) / 2;
                            int length = string_to_num(params[1]) / 2;

                            if (length == 0)
                                length = 1;

                            s += num_to_string(delay) + "," + num_to_string(length) + "," + params[2];
                        }
                        else if (command == 'p' || command == 'u')
                        {
                            int delay = string_to_num(params[0]) / 2;
                            int freq = string_to_num(params[1]) * 2;

                            if (freq > 255)
                                freq = 255;

                            s += num_to_string(delay) + "," + num_to_string(freq) + "," + params[2];
                        }
                    }

                    break;
                }

                case 's':
                case 't':
                {
                    if (!isLabelName && !isHex)
                    {
                        i++;

                        string lenStr;

                        while (i < data.size())
                        {
                            if (data[i] >= '0' && data[i] <= '9')
                            {
                                lenStr += data[i];
                            }
                            else
                            {
                                break;
                            }

                            i++;
                        }

                        i--;

                        int length = string_to_num(lenStr) / 2;
                        if (length == 0)
                            length = 1;

                        s += num_to_string(length);
                    }

                    break;
                }
                
                default:
                {
                    if (!(data[i] >= '0' && data[i] <= '9') && !(tolower(data[i]) >= 'a' && tolower(data[i]) <= 'z') && data[i] != '_')
                    {
                        isLabelName = false;
                    }
                
                    break;
                }
            }
        }
    }

    return s;
}

Bpm::Bpm()
{
    Macro::hasBraces = false;
    Macro::name = "bpm";
    Macro::implicitParams = "";
}

string Bpm::macro(map<string, string> &v, const vector<int>& p, string data)
{
    data = "";

    if (p.size() != 1)
    {
        error = "Incorrect number of parameters.";
        return "";
    }

    const float TEMPO = 0.4096;
    bpm = (float)p[0];
    int tempo = (int)round(bpm * TEMPO);

    string s;

    s += 't';
    ostringstream ssout;
    ssout << tempo;
    s += ssout.str();

    return s;
}

Hex::Hex()
{
    Macro::hasBraces = false;
    Macro::name = "hex";
    Macro::implicitParams = "";
}

string Hex::macro(map<string, string> &v, const vector<int>& p, string data)
{
    data = "";

    if (p.size() != 1)
    {
        error = "Incorrect number of parameters.";
        return "";
    }

    int dec = p[0];

    string s;

    ostringstream ssout;
    ssout << setfill('0') << setw(2) << uppercase << hex << dec;
    string hex = ssout.str();
    hex = hex.substr(hex.size() - 2);
    s += hex;

    return s;
}

Hexc::Hexc()
{
    Macro::hasBraces = false;
    Macro::name = "hexc";
    Macro::implicitParams = "";
}

string Hexc::macro(map<string, string> &v, const vector<int>& p, string data)
{
    data = "";

    if (p.size() != 1)
    {
        error = "Incorrect number of parameters.";
        return "";
    }

    int dec = p[0];

    string s;

    s += '$';
    ostringstream ssout;
    ssout << setfill('0') << setw(2) << uppercase << hex << dec;
    string hex = ssout.str();
    hex = hex.substr(hex.size() - 2);
    s += hex;

    return s;
}

Binary::Binary()
{
    Macro::hasBraces = false;
    Macro::name = "binary";
    Macro::implicitParams = "";
}

string Binary::macro(map<string, string> &v, const vector<int>& p, string data)
{
    data = "";

    if (p.size() != 1)
    {
        error = "Incorrect number of parameters.";
        return "";
    }

    char dec = p[0];

    string s;

    for (unsigned i = 0; i < 8; i++)
    {
        if (dec < 0)
        {
            s += '1';
        }
        else
        {
            s += '0';
        }

        dec = dec << 1;
    }

    return s;
}

Add::Add()
{
    Macro::hasBraces = false;
    Macro::name = "add";
    Macro::implicitParams = "";
}

string Add::macro(map<string, string> &v, const vector<int>& p, string data)
{
    data = "";

    if (p.size() < 2)
    {
        error = "Not enough parameters.";
        return "";
    }

    string s;

    int sum = 0;
    for (unsigned i = 0; i < p.size(); i++)
    {
        sum += p[i];
    }

    s = num_to_string(sum);

    return s;
}

Sub::Sub()
{
    Macro::hasBraces = false;
    Macro::name = "sub";
    Macro::implicitParams = "";
}

string Sub::macro(map<string, string> &v, const vector<int>& p, string data)
{
    data = "";

    if (p.size() < 2)
    {
        error = "Not enough parameters.";
        return "";
    }

    string s;

    int sum = p[0];
    for (unsigned i = 1; i < p.size(); i++)
    {
        sum -= p[i];
    }

    s = num_to_string(sum);

    return s;
}

Mul::Mul()
{
    Macro::hasBraces = false;
    Macro::name = "mul";
    Macro::implicitParams = "";
}

string Mul::macro(map<string, string> &v, const vector<int>& p, string data)
{
    data = "";

    if (p.size() < 2)
    {
        error = "Not enough parameters.";
        return "";
    }

    string s;

    int prod = p[0];
    for (unsigned i = 1; i < p.size(); i++)
    {
        prod *= p[i];
    }

    s = num_to_string(prod);

    return s;
}

Div::Div()
{
    Macro::hasBraces = false;
    Macro::name = "div";
    Macro::implicitParams = "";
}

string Div::macro(map<string, string> &v, const vector<int>& p, string data)
{
    data = "";

    if (p.size() < 2)
    {
        error = "Not enough parameters.";
        return "";
    }

    string s;

    int quot = p[0];
    for (unsigned i = 1; i < p.size(); i++)
    {
        if (p[i] == 0)
        {
            error = "Division by 0.";
            return "";
        }

        quot /= p[i];
    }

    s = num_to_string(quot);

    return s;
}

Mod::Mod()
{
    Macro::hasBraces = false;
    Macro::name = "mod";
    Macro::implicitParams = "";
}

string Mod::macro(map<string, string> &v, const vector<int>& p, string data)
{
    data = "";

    if (p.size() < 2)
    {
        error = "Not enough parameters.";
        return "";
    }

    string s;

    int rem = p[0];
    for (unsigned i = 1; i < p.size(); i++)
    {
        if (p[i] == 0)
        {
            error = "Modulus by 0.";
            return "";
        }

        rem %= p[i];
    }

    s = num_to_string(rem);

    return s;
}

And::And()
{
    Macro::hasBraces = false;
    Macro::name = "and";
    Macro::implicitParams = "";
}

string And::macro(map<string, string> &v, const vector<int>& p, string data)
{
    data = "";

    if (p.size() < 2)
    {
        error = "Not enough parameters.";
        return "";
    }

    string s;

    int logic = p[0];
    for (unsigned i = 1; i < p.size(); i++)
    {
        logic = logic & p[i];
    }

    s = num_to_string(logic);

    return s;
}

Or::Or()
{
    Macro::hasBraces = false;
    Macro::name = "or";
    Macro::implicitParams = "";
}

string Or::macro(map<string, string> &v, const vector<int>& p, string data)
{
    data = "";

    if (p.size() < 2)
    {
        error = "Not enough parameters.";
        return "";
    }

    string s;

    int logic = p[0];
    for (unsigned i = 1; i < p.size(); i++)
    {
        logic = logic | p[i];
    }

    s = num_to_string(logic);

    return s;
}

Not::Not()
{
    Macro::hasBraces = false;
    Macro::name = "not";
    Macro::implicitParams = "";
}

string Not::macro(map<string, string> &v, const vector<int>& p, string data)
{
    data = "";

    if (p.size() != 1)
    {
        error = "Incorrect number of parameters.";
        return "";
    }

    string s;
    s = num_to_string(~(p[0]));
    return s;
}

Vrand::Vrand()
{
    Macro::hasBraces = true;
    Macro::name = "vrand";
    Macro::implicitParams = "";
}

string Vrand::macro(map<string, string> &v, const vector<int>& p, string data)
{
    string s;

    if (p.size() < 3)
    {
        error = "Not enough parameters.";
        return "";
    }
    else if (p.size() > 4)
    {
        error = "Too many parameters.";
        return "";
    }

    seed = p[0];
    min = p[1];
    max = p[2];

    if (p.size() == 4)
        initVolume = p[3];
    else
        initVolume = -1;

    bool isVolDefined = false;
    int prevVol = -1;
    srand(seed);

    bool isLabel = false;
    bool isHex = false;

    for (unsigned i = 0; i < data.size(); i++)
    {
        switch (tolower(data[i]))
        {
            case 'a':
            case 'b':
            case 'c':
            case 'd':
            case 'e':
            case 'f':
            case 'g':
            {
                if (!isLabel && !isHex)
                {
                    /* We check if the volume is already defined before this note.
                     * This gives the user flexibility, in case they want most of
                     * the volumes randomized, but still be able to control the
                     * volumes of some specific notes. Plus, consecutive volume
                     * declarations right before a note is just damn redundant. */

                    if (!isVolDefined)
                    {
                        int newVol = min + rand() % (max - min + 1);
                        if (newVol != prevVol)
                        {
                            s += " v" + num_to_string(newVol) + " ";
                            prevVol = newVol;
                        }
                    }
                    else
                    {
                        isVolDefined = false;
                    }
                }

                break;
            }

            case 'v':
            {
                isVolDefined = true;
                break;
            }

            case '(':
            {
                isLabel = true;
                break;
            }

            case ')':
            {
                isLabel = false;
                break;
            }

            case '$':
            case 'q':
            {
                isHex = true;
                break;
            }
        }

        if (i < data.size() - 1)
        {
            if ((data[i] != '{' || data[i + 1] != '{') && (data[i] != '}' || data[i + 1] != '}'))
            {
                s += data[i];
            }
            else
            {
                i++; // Skip an iteration
            }
        }
        else
        {
            s += data[i];
        }
    }

    if (initVolume >= 0)
    {
        s += " v" + num_to_string(initVolume);
    }
    
    srand(seed);
    v["seed"] = num_to_string(rand());
    return s;
}


Qrand::Qrand()
{
    Macro::hasBraces = true;
    Macro::name = "qrand";
    Macro::implicitParams = "";
}

string Qrand::macro(map<string, string> &v, const vector<int>& p, string data)
{
    string s;

    if (p.size() < 3)
    {
        error = "Not enough parameters.";
        return "";
    }
    else if (p.size() > 4)
    {
        error = "Too many parameters.";
        return "";
    }

    seed = p[0];
    min = p[1];
    max = p[2];

    if (p.size() == 4)
    {
        initVelocity = p[3] & 15;
        initLegato = (p[3] & (~15)) >> 4;
    }
    else
    {
        initVelocity = -1;
        initLegato = 7;
    }

    bool isVelDefined = false;
    int prevVel = -1;
    srand(seed);

    bool isLabel = false;
    bool isHex = false;

    for (unsigned i = 0; i < data.size(); i++)
    {
        switch (tolower(data[i]))
        {
            case 'a':
            case 'b':
            case 'c':
            case 'd':
            case 'e':
            case 'f':
            case 'g':
            {
                if (!isLabel && !isHex)
                {
                    /* We check if the velocity is already defined before this note.
                     * This gives the user flexibility, in case they want most of
                     * the velocitys randomized, but still be able to control the
                     * velocitys of some specific notes. Plus, consecutive velocity
                     * declarations right before a note is just damn redundant. */

                    if (!isVelDefined)
                    {
                        int newVel = min + rand() % (max - min + 1);
                        if (newVel != prevVel)
                        {
                            s += " q" + dec_to_hex(initLegato) + dec_to_hex(newVel) + " ";
                            prevVel = newVel;
                        }
                    }
                    else
                    {
                        isVelDefined = false;
                    }
                }

                break;
            }

            case '(':
            {
                isLabel = true;
                break;
            }

            case ')':
            {
                isLabel = false;
                break;
            }

            case 'q':
            {
                isVelDefined = true;
            }
            case '$':
            {
                isHex = true;
                break;
            }
        }

        if (i < data.size() - 1)
        {
            if ((data[i] != '{' || data[i + 1] != '{') && (data[i] != '}' || data[i + 1] != '}'))
            {
                s += data[i];
            }
            else
            {
                i++; // Skip an iteration
            }
        }
        else
        {
            s += data[i];
        }
    }

    if (initVelocity >= 0)
    {
        s += " q" + dec_to_hex(initLegato) + dec_to_hex(initVelocity);
    }
    
    srand(seed);
    v["seed"] = num_to_string(rand());
    return s;
}


Yrand::Yrand()
{
    Macro::hasBraces = true;
    Macro::name = "yrand";
    Macro::implicitParams = "";
}

string Yrand::macro(map<string, string> &v, const vector<int>& p, string data)
{
    string s;

    if (p.size() < 3)
    {
        error = "Not enough parameters.";
        return "";
    }
    else if (p.size() > 4)
    {
        error = "Too many parameters.";
        return "";
    }

    seed = p[0];
    min = p[1];
    max = p[2];

    if (p.size() == 4)
        initPan = p[3];
    else
        initPan = -1;

    bool isPanDefined = false;
    int prevPan = -1;
    srand(seed);

    bool isLabel = false;
    bool isHex = false;

    for (unsigned i = 0; i < data.size(); i++)
    {
        switch (tolower(data[i]))
        {
            case 'a':
            case 'b':
            case 'c':
            case 'd':
            case 'e':
            case 'f':
            case 'g':
            {
                if (!isLabel && !isHex)
                {
                    /* We check if the volume is already defined before this note.
                     * This gives the user flexibility, in case they want most of
                     * the volumes randomized, but still be able to control the
                     * volumes of some specific notes. Plus, consecutive volume
                     * declarations right before a note is just damn redundant. */

                    if (!isPanDefined)
                    {
                        int newPan = min + rand() % (max - min + 1);
                        if (newPan != prevPan)
                        {
                            s += " y" + num_to_string(newPan) + " ";
                            prevPan = newPan;
                        }
                    }
                    else
                    {
                        isPanDefined = false;
                    }
                }

                break;
            }

            case 'y':
            {
                isPanDefined = true;
                break;
            }

            case '(':
            {
                isLabel = true;
                break;
            }

            case ')':
            {
                isLabel = false;
                break;
            }

            case '$':
            case 'q':
            {
                isHex = true;
                break;
            }
        }

        if (i < data.size() - 1)
        {
            if ((data[i] != '{' || data[i + 1] != '{') && (data[i] != '}' || data[i + 1] != '}'))
            {
                s += data[i];
            }
            else
            {
                i++; // Skip an iteration
            }
        }
        else
        {
            s += data[i];
        }
    }

    if (initPan >= 0)
    {
        s += " y" + num_to_string(initPan);
    }
    
    srand(seed);
    v["seed"] = num_to_string(rand());
    return s;
}


Portamento::Portamento()
{
    Macro::hasBraces = true;
    Macro::name = "portamento";
    Macro::implicitParams = "?o, ?h, ";
}

string Portamento::macro(map<string, string> &v, const vector<int>& p, string data)
{
    string s;

    if (p.size() != 4)
    {
        error = "Incorrect number of parameters.";
        return "";
    }

    octave = p[0];
    transposition = p[1];
    length = p[2];
    isLegato = p[3];

    if (length < 1)
    {
        length = 1;
    }

    int prevPitch = -1;
    int currentPitch = -1;

    bool isLabel = false;
    bool isHex = false;
    bool isLegatoNote = false;
    bool isPortaOn = false;

    {
        Double doubleFix;
        Half halfFix;

        vector<int> noParams;

        // Run the data through the doubling macro and then the halving macro. This fixes pitchbends for any note length greater than ^2.
        data = doubleFix.macro(v, noParams, data);
        data = halfFix.macro(v, noParams, data);
        //
    }

    for (unsigned i = 0; i < data.size(); i++)
    {
        if (isHex)
        {
            if ((data[i] < '0' || data[i] > '9') && (tolower(data[i]) < 'a' || tolower(data[i]) > 'f'))
            {
                isHex = false;
            }
        }

        switch (tolower(data[i]))
        {
            case 'a':
            case 'b':
            case 'c':
            case 'd':
            case 'e':
            case 'f':
            case 'g':
            {
                if (!isLabel && !isHex)
                {
                    switch (tolower(data[i]))
                    {
                        case 'a':
                            currentPitch = 9;
                            break;

                        case 'b':
                            currentPitch = 11;
                            break;

                        case 'c':
                            currentPitch = 0;
                            break;

                        case 'd':
                            currentPitch = 2;
                            break;

                        case 'e':
                            currentPitch = 4;
                            break;

                        case 'f':
                            currentPitch = 5;
                            break;

                        case 'g':
                            currentPitch = 7;
                            break;
                    }

                    currentPitch += (octave - 1) * 12 + transposition;

                    int noteStrLen = 1;

                    i++;
                    if (i >= data.size())
                        break;

                    if (data[i] == '+')
                    {
                        currentPitch++;
                        noteStrLen++;
                    }
                    else if (data[i] == '-')
                    {
                        currentPitch--;
                        noteStrLen++;
                    }

                    i--;

                    if (!isLegato)
                    {
                        if (prevPitch != -1 && prevPitch != currentPitch)
                        {
                            // Portamento from previous note.
                            s += " n0," + num_to_string(length) + "," + num_to_string(currentPitch - prevPitch) + " ";
                            isPortaOn = true;
                        }
                        else
                        {
                            if (isPortaOn)
                            {
                                s += " ~n ";
                                isPortaOn = false;
                            }
                        }
                    }
                    else
                    {
                        if (prevPitch != -1)
                        {
                            isLegatoNote = true;

                            // Transform all notes into ties to make it true legato.
                            s += "^";
                            i++;

                            if (noteStrLen > 1)
                            {
                                i++;
                            }

                            while (i < data.size() && ((data[i] >= '0' && data[i] <= '9') || data[i] == '=' || data[i] == '.'))
                            {
                                s += data[i];
                                i++;
                            }

                            i--;

                            if (prevPitch != currentPitch)
                            {
                                s += " &0," + num_to_string(length) + "," + num_to_string(currentPitch) + " ";
                            }
                        }
                    }

                    prevPitch = currentPitch;
                }

                break;
            }

            case 'r':
            {
                if (!isLabel)
                {
                    if (isLegato)
                    {
                        prevPitch = -1;
                    }
                }

                break;
            }

            case 'o':
            {
                if (!isLabel)
                {
                    i++;
                    if (i >= data.size())
                        break;

                    string octStr;
                    octStr += data[i];
                    i--;

                    octave = string_to_num(octStr);

                    if (is_error())
                    {
                        error = string("Bad octave: \"o") + octStr + "\".";
                        return "";
                    }
                }

                break;
            }

            case 'h':
            {
                if (!isLabel)
                {
                    unsigned prevIndex = i;
                    i++;
                    if (i >= data.size())
                        break;

                    string transStr;
                    while ((data[i] >= '0' && data[i] <= '9') || data[i] == '-')
                    {
                        transStr += data[i];
                        i++;
                    }

                    i = prevIndex;

                    transposition = string_to_num(transStr);

                    if (is_error())
                    {
                        error = string("Bad note transposition: \"h") + transStr + "\".";
                        return "";
                    }
                }

                break;
            }

            case 'v':
            {
                if (!isLabel)
                {
                    if (isLegato)
                    {
                        /* If the legato setting is enabled, convert all standard volume commands
                         * into one-tick volume fade, since YI's music engine can't do immediate
                         * volume changes mid-note apparently. */
                        if (i == 0 || data[i - 1] != ',')
                        {
                            s += "s1,";
                        }
                    }
                }

                break;
            }

            case 'y':
            {
                if (!isLabel)
                {
                    if (isLegato)
                    {
                        // Do the same conversion for immediate panning as we did for volume.
                        if (i == 0 || data[i - 1] != ',')
                        {
                            s += "s1,";
                        }
                    }
                }

                break;
            }

            case '<':
            {
                octave--;
                break;
            }

            case '>':
            {
                octave++;
                break;
            }

            case '[':
            case ']':
            {
                prevPitch = -1;
                break;
            }

            case '(':
            {
                isLabel = true;
                break;
            }

            case ')':
            {
                isLabel = false;
                prevPitch = -1;
                break;
            }

            case '$':
            case 'q':
            {
                isHex = true;
                break;
            }
        }

        if (i < data.size() - 1)
        {
            if ((data[i] != '{' || data[i + 1] != '{') && (data[i] != '}' || data[i + 1] != '}'))
            {
                if (!isLegatoNote)
                    s += data[i];
            }
            else
            {
                i++; // Skip an iteration
            }
        }
        else
        {
            if (!isLegatoNote)
                s += data[i];
        }

        isLegatoNote = false;
    }
    
    if (!isLegato)
        s += " ~n ";
        
    return s;
}


Arpeggiator::Arpeggiator()
{
    Macro::hasBraces = false;
    Macro::name = "arpeggiator";
    Macro::implicitParams = "";
}

string Arpeggiator::macro(map<string, string> &v, const vector<int>& p, string data)
{
    data = "";

    if (p.size() != 6)
    {
        error = "Incorrect number of parameters.";
        return "";
    }

    seed = p[0];
    key = p[1];
    type = (Arpeggiator::Type2)p[2];
    depth = p[3];
    noteLength = p[4];
    numNotes = p[5];

    if (type < 0 || type > 4)
    {
        error = "Invalid chord type.";
        return "";
    }
    else if (depth < 0)
    {
        error = "Chord depth cannot be negative.";
        return "";
    }
    else if (noteLength <= 0 || noteLength > 192)
    {
        error = "Invalid note length.";
        return "";
    }
    else if (numNotes < 0)
    {
        error = "The number of notes must be positive.";
        return "";
    }

    string s;

    int major[8] = { 0, 4, 7, 11, 12, 14, 17, 21 };
    int minor[8] = { 0, 3, 7, 10, 12, 14, 17, 20 };
    int diminished[8] = { 0, 3, 6, 9, 12, 15, 18, 21 };
    int halfDiminished[8] = { 0, 3, 6, 10, 12, 13, 17, 20 };
    int dominant[8] = { 0, 4, 7, 10, 12, 14, 17, 21 };

    int octave = 7;
    int prevOctave = octave;
    int initOctave = octave;
    const int BASE_NOTE = 84; // This is to prevent the note or octave from ever getting below 0; makes calculations easier
    int prevInterval = -1;

    srand(seed);

    for (int i = 0; i < numNotes; i++)
    {
        int note = BASE_NOTE;
        int oct = 0;
        int interval;

        // Get random interval
        if (i == 0)
        {
            // Make the very first note the chord's root.
            interval = 0;
        }
        else
        {
            interval = rand() % (depth + 1);

            // Prevent duplicate notes if possible
            if (depth > 1 && interval == prevInterval)
            {
                interval++;
                interval %= depth + 1;
            }
        }
        //

        prevInterval = interval;

        if (depth < 4 && interval == depth)
            interval = 4;

        prevOctave = octave;

        oct = (interval / 8) * 2;

        switch (type)
        {
            case MAJOR:
            {
                note = BASE_NOTE + key + major[interval % 8] + oct * 12;
                break;
            }

            case MINOR:
            {
                note = BASE_NOTE + key + minor[interval % 8] + oct * 12;
                break;
            }

            case DIMINISHED:
            {
                note = BASE_NOTE + key + diminished[interval % 8] + oct * 12;
                break;
            }

            case HALF_DIMINISHED:
            {
                note = BASE_NOTE + key + halfDiminished[interval % 8] + oct * 12;
                break;
            }

            case DOMINANT:
            {
                note = BASE_NOTE + key + dominant[interval % 8] + oct * 12;
                break;
            }

            default:
                note = 0;
                break;
        }

        if (note < 0)
        {
            error = "Interval offset is too low.";
            return "";
        }
        else if (note > 2 * BASE_NOTE)
        {
            error = "Interval offset is too high.";
            return "";
        }

        octave = note / 12;
        note %= 12;

        if (octave > prevOctave)
        {
            s += string(octave - prevOctave, '>');
        }
        else if (octave < prevOctave)
        {
            s += string(prevOctave - octave, '<');
        }

        ostringstream sout;
        sout << noteLength;
        s += NOTE_STRINGS[note] + "=" + sout.str();
    }

    // Preserve the initial octave
    if (initOctave > octave)
    {
        s += string(initOctave - octave, '>');
    }
    else if (initOctave < octave)
    {
        s += string(octave - initOctave, '<');
    }
    //

    return s;
}


Echo_Enable::Echo_Enable()
{
    Macro::hasBraces = false;
    Macro::name = "echo_enable";
    Macro::implicitParams = "?x, ?z, ";
}

string Echo_Enable::macro(map<string, string> &v, const vector<int>& p, string data)
{
    data = "";

    if (p.size() != 7)
    {
        error = "Incorrect number of parameters.";
        return "";
    }

    echoFlags = p[0];
    echoRightVol = p[1];
    echoLeftVol = p[2];
    echoDelay = p[3];
    echoFeedback = p[4];
    echoFIR = p[5];
    echoEnableBits = p[6];

    int newFlags = bin_to_dec(num_to_string(echoFlags));
    newFlags = newFlags | echoEnableBits;

    string s;

    s += " x";

    {
        Binary b;

        vector<int> flags(1, newFlags);
        s += b.macro(v, flags, ""); // Convert to binary string
    }
    s += "," + num_to_string(echoRightVol) + "," + num_to_string(echoLeftVol);
    s += " z" + num_to_string(echoDelay) + "," + num_to_string(echoFeedback) + "," + num_to_string(echoFIR) + " ";

    return s;
}


Echo_Disable::Echo_Disable()
{
    Macro::hasBraces = false;
    Macro::name = "echo_disable";
    Macro::implicitParams = "?x, ?z, ";
}

string Echo_Disable::macro(map<string, string> &v, const vector<int>& p, string data)
{
    data = "";

    if (p.size() != 7)
    {
        error = "Incorrect number of parameters.";
        return "";
    }

    echoFlags = p[0];
    echoRightVol = p[1];
    echoLeftVol = p[2];
    echoDelay = p[3];
    echoFeedback = p[4];
    echoFIR = p[5];
    echoEnableBits = p[6];

    int newFlags = bin_to_dec(num_to_string(echoFlags));
    newFlags = newFlags & ~echoEnableBits;

    string s;

    s += " x";

    {
        Binary b;

        vector<int> flags(1, newFlags);
        s += b.macro(v, flags, ""); // Convert to binary string
    }
    s += "," + num_to_string(echoRightVol) + "," + num_to_string(echoLeftVol);
    s += " z" + num_to_string(echoDelay) + "," + num_to_string(echoFeedback) + "," + num_to_string(echoFIR) + " ";

    return s;
}


Adsr::Adsr()
{
    Macro::hasBraces = false;
    Macro::name = "adsr";
    Macro::implicitParams = "";
}

string Adsr::macro(map<string, string> &v, const vector<int>& p, string data)
{
    data = "";

    if (p.size() != 4)
    {
        error = "Incorrect number of parameters.";
        return "";
    }

    attack = p[0];
    decay = p[1];
    sustain = p[2];
    release = p[3];

    if (attack < 0 || attack > 15)
    {
        error = "Invalid attack value.";
        return "";
    }

    if (decay < 0 || decay > 7)
    {
        error = "Invalid decay value.";
        return "";
    }

    if (sustain < 0 || sustain > 7)
    {
        error = "Invalid sustain value.";
        return "";
    }

    if (release < 0 || release > 31)
    {
        error = "Invalid release value.";
        return "";
    }

    string s;
    int da = 128 + decay * 16 + attack;
    int sr = sustain * 32 + release;

    // Convert to hex commands.
    {
        vector<int> n(1, da);

        Hexc tohex;
        s += tohex.macro(v, n, "");

        n[0] = sr;

        s += " " + tohex.macro(v, n, "") += " $00";
    }

    return s;
}


Gain::Gain()
{
    Macro::hasBraces = false;
    Macro::name = "gain";
    Macro::implicitParams = "";
}

string Gain::macro(map<string, string> &v, const vector<int>& p, string data)
{
    data = "";

    if (p.size() != 2)
    {
        error = "Incorrect number of parameters.";
        return "";
    }

    mode = p[0];
    value = p[1];

    if (mode < 0 || mode > 5)
    {
        error = "Invalid GAIN mode.";
        return "";
    }

    if (mode == 0)
    {
        if (value < 0 || value > 127)
        {
            error = "This GAIN mode requires a value between 0 and\n127.";
            return "";
        }
    }
    else
    {
        if (value < 0 || value > 31)
        {
            error = "This GAIN mode requires a value between 0 and\n31.";
            return "";
        }
    }

    string s;

    int gn;

    if (mode == 0)
        gn = value;
    else
        gn = ((mode + 3) << 5) + value;

    // Convert to hex commands.
    {
        vector<int> n(1, gn);

        Hexc tohex;
        s += "$00 $00 " + tohex.macro(v, n, "");
    }

    return s;
}


Pitch::Pitch()
{
    Macro::hasBraces = false;
    Macro::name = "pitch";
    Macro::implicitParams = "";
}

string Pitch::macro(map<string, string> &v, const vector<int>& p, string data)
{
    data = "";

    if (p.size() < 1)
    {
        error = "Not enough of parameters.";
        return "";
    }

    double pitch = (double)p[0];
    
    for (unsigned i = 1; i < p.size(); i++)
    {
        pitch += p[i] / pow(100, i);
    }
    
    double frequency = 768 * pow(2, pitch / 1200);
    int nspcTune = (int)(round(frequency));

    string s;

    {
        int highByte = nspcTune >> 8;
        int lowByte = nspcTune & 255;

        vector<int> n(1, highByte);

        Hexc tohex;
        s += tohex.macro(v, n, "");
        n[0] = lowByte;
        s += " " + tohex.macro(v, n, "");
    }

    return s;
}


Smw::Smw()
{
    Macro::hasBraces = true;
    Macro::name = "smw";
    Macro::implicitParams = "";
}

string Smw::macro(map<string, string> &v, const vector<int>& p, string data)
{
    if (p.size() != 0)
    {
        error = "Incorrect number of parameters.";
        return "";
    }

    string s;

    bool isHex = false;
    bool isLabel = false;

    for (unsigned i = 0; i < data.size(); i++)
    {
        if (isHex)
        {
            if ((data[i] < '0' || data[i] > '9') && (tolower(data[i]) < 'a' || tolower(data[i]) > 'f'))
            {
                isHex = false;
            }
        }

        switch (tolower(data[i]))
        {
            case '&':
            {
                if (!isHex && !isLabel)
                {
                    s += "$DD "; // SMW's hex command for pitch bends

                    for (unsigned p = 0; p < 3; p++)
                    {
                        i++;
                        string numStr;

                        while (i < data.size() && data[i] >= '0' && data[i] <= '9')
                        {
                            numStr += data[i];
                            i++;
                        }

                        int num = string_to_num(numStr);

                        if (p == 2)
                        {
                            num += 128; // Add 0x80 to the pitch value.
                        }
                        // Convert to hex command
                        {
                            vector<int> n(1, num);

                            Hexc tohex;
                            s += tohex.macro(v, n, "");
                            s += " ";
                        }
                    }
                }

                break;
            }
            
            case 'i':
            {
                if (!isHex && !isLabel)
                {
                    s += "$EE "; // SMW's hex command for finetune
                    
                    string numStr;
                    for (i++; i < data.size() && data[i] >= '0' && data[i] <= '9'; i++)
                    {
                        numStr += data[i];
                    }
                    
                    s += "$" + dec_to_hex(string_to_num(numStr)) + " ";
                }
                
                break;
            }
            
            case '~':
            {
                if (!isHex && !isLabel)
                {
                    i++;
                    
                    if (data[i] == 'p')
                    {
                        s += "$DF ";
                    }
                    else if (data[i] == 'u')
                    {
                        s += "$E5 $00 $00 $00 ";
                    }
                    else if (data[i] == 'm')
                    {
                        s += "$EB $00 $00 $00 ";
                    }
                    else if (data[i] == 'n')
                    {
                        s += "$EC $00 $00 $00 ";
                    }
                    else if (data[i] == 'x' || data[i] == 'z')
                    {
                        s += "$F0 ";
                    }
                    
                    i++;
                }
                
                break;
            }

            case '(':
            {
                isLabel = true;
                break;
            }

            case ')':
            {
                isLabel = false;
                break;
            }

            case '$':
            case 'q':
            {
                isHex = true;
                break;
            }
        }

        if (i < data.size() - 1)
        {
            if ((data[i] != '{' || data[i + 1] != '{') && (data[i] != '}' || data[i + 1] != '}'))
            {
                s += data[i];
            }
            else
            {
                i++; // Skip an iteration
            }
        }
        else
        {
            s += data[i];
        }
    }

    return s;
}


Qmix::Qmix()
{
    Macro::hasBraces = true;
    Macro::name = "qmix";
    Macro::implicitParams = "";
}

string Qmix::macro(map<string, string> &v, const vector<int>& p, string data)
{
    if (p.size() != 1)
    {
        error = "Incorrect number of parameters.";
        return "";
    }

    int percent = p[0];

    string s;
    
    bool isHex = false;
    bool isLabel = false;

    for (unsigned i = 0; i < data.size(); i++)
    {
        if (isHex)
        {
            if ((data[i] < '0' || data[i] > '9') && (tolower(data[i]) < 'a' || tolower(data[i]) > 'f'))
            {
                isHex = false;
            }
        }

        switch (tolower(data[i]))
        {
            case '(':
            {
                isLabel = true;
                break;
            }

            case ')':
            {
                isLabel = false;
                break;
            }

            case '$':
            {
                isHex = true;
                break;
            }
            
            case 'q':
            {
                s += data[i];
            
                if (!isHex && !isLabel)
                {
                    i++;
                    string hexStr;
                    
                    while (i < data.size() && ((data[i] >= '0' && data[i] <= '9') || (tolower(data[i]) >= 'a' && tolower(data[i]) <= 'f')))
                    {
                        hexStr += data[i];
                        i++;
                    }
                    
                    unsigned hexNum = hex_to_dec(hexStr);
                    unsigned legato = (hexNum & ~15) & 255;
                    unsigned velocity = hexNum & 15;
                    
                    velocity = (unsigned)(round((double)(velocity * ((double)percent / 100.0))));
                    
                    // Convert to hex
                    {
                        vector<int> n(1, legato | velocity);

                        Hex tohex;
                        s += tohex.macro(v, n, "");
                    }
                }
                
                break;
            }
        }

        if (i < data.size() - 1)
        {
            if ((data[i] != '{' || data[i + 1] != '{') && (data[i] != '}' || data[i + 1] != '}'))
            {
                s += data[i];
            }
            else
            {
                i++; // Skip an iteration
            }
        }
        else
        {
            s += data[i];
        }
    }

    return s;
}


Table::Table()
{
    Macro::hasBraces = true;
    Macro::name = "table";
    Macro::implicitParams = "?ins, ?i, ?Q, ?v, ?y, ";
}

string Table::macro(map<string, string> &v, const vector<int>& p, string data)
{
    table.clear();

    if (p.size() < 10)
    {
        error = "Not enough parameters.";
        return "";
    }
    
    // Implicit parameters
    instrument = p[0];
    fineTune = p[1];
    articulation = p[2];
    volume = p[3];
    pan = p[4];
    // Explicit parameters
    command = tolower((char)(p[5]));
    ticks = p[6];
    loopPoint = p[7];
    
    if (ticks < 0)
    {
        error = "Invalid tick count.";
        return "";
    }
    
    int tableSize = p[8];
    if (tableSize < 0 || (unsigned)tableSize > p.size() - 9)
    {
        error = "Broken list.";
        return "";
    }
    
    // Initialize table values
    for (unsigned i = 9; i < p.size(); i++)
    {
        for (unsigned j = 0; j < ticks; j++)
        {
            table.push_back(p[i]);
        }
    }
    
    if (loopPoint >= tableSize)
    {
        error = "Loop point is out of range. (Use a loop point of -1 if you don't want it to loop.)";
        return "";
    }
    else if (loopPoint < 0)
    {
        loopPoint = tableSize - 1;
    }
    
    loopPoint *= ticks;
    
    if (command != '@' && command != 'i' && command != 'q' && command != 'v' && command != 'y')
    {
        error = string("Invalid table command: \'") + command + "\'.";
        return "";
    }

    string s;
    data = convert_to_ticks(v, data);
    
    bool isHex = false;
    bool isLabel = false;
    unsigned tableIndex = 0;
    int prevTableVal = 0;
    bool isFirstVal = true;

    for (unsigned i = 0; i < data.size(); i++)
    {
        if (isHex)
        {
            if ((data[i] < '0' || data[i] > '9') && (tolower(data[i]) < 'a' || tolower(data[i]) > 'f'))
            {
                isHex = false;
            }
        }

        if ((data[i] != '{' || data[i + 1] != '{') && (data[i] != '}' || data[i + 1] != '}'))
        {
            char chr = tolower(data[i]);
            
            switch (chr)
            {
                case 'a':
                case 'b':
                case 'c':
                case 'd':
                case 'e':
                case 'f':
                case 'g':
                case '^':
                {
                    if (!isLabel && !isHex)
                    {
                        if (chr != '^')
                        {
                            tableIndex = 0;
                            isFirstVal = true;
                        }
                        
                        string noteStr;
                        noteStr += chr;
                    
                        for (i++; i < data.size() && (data[i] < '0' || data[i] > '9'); i++) // iterate until it gets to a tick val
                        {
                            if (data[i] != '=')
                                noteStr += data[i];
                        }
                    
                        string tickStr;
                        
                        while (i < data.size() && (data[i] >= '0' && data[i] <= '9'))
                        {
                            tickStr += data[i];
                            i++;
                        }
                        
                        i--;
                        
                        unsigned noteTicks = string_to_num(tickStr);
                        
                        for (unsigned j = 0; j < noteTicks; j++)
                        {
                            if (command == 'q')
                            {
                                int legato = (articulation & ~15) & 255;
                                int val = legato | table[tableIndex];
                                
                                s += " " + char_to_string(command) + dec_to_hex(val) + " ";
                            }
                            else
                            {
                                int val = table[tableIndex];
                                s += " " + char_to_string(command) + num_to_string(val) + " ";
                            }
                        
                            if (isFirstVal)
                            {
                                s += noteStr + "=" + "1";
                                isFirstVal = false;
                            }
                            else
                            {
                                s += "^=1";
                            }
                            
                            prevTableVal = table[tableIndex];
                            tableIndex++;
                            
                            if (tableIndex == table.size())
                            {
                                tableIndex = loopPoint;
                            }
                        }
                    }
                    else
                    {
                        s += data[i];
                    }

                    break;
                }
                
                default:
                {
                    s += data[i];
                    
                    if (chr == '(')
                    {
                        isLabel = true;
                    }
                    else if (chr == ')')
                    {
                        isLabel = false;
                    }
                    else if (chr == 'q' || chr == '$')
                    {
                        isHex = true;
                    }
                    
                    break;
                }
            }
        }
        else if (i < data.size() - 1)
        {
            i++; // Skip an iteration
        }
    }

    return s;
}


List_size::List_size()
{
    Macro::hasBraces = false;
    Macro::name = "list_size";
    Macro::implicitParams = "";
}

string List_size::macro(map<string, string> &v, const vector<int>& p, string data)
{
    data = "";
    
    if (p.size() < 1)
    {
        error = "Incorrect number of parameters.";
        return "";
    }
    else if (p[0] < 0 || (unsigned)p[0] > p.size() - 1)
    {
        error = "Broken list.";
        return "";
    }
    else if ((unsigned)p[0] < p.size() - 1)
    {
        error = "Incorrect number of parameters.";
        return "";
    }

    string s;
    s += num_to_string(p[0]);

    return s;
}


List_contents::List_contents()
{
    Macro::hasBraces = false;
    Macro::name = "list_contents";
    Macro::implicitParams = "";
}

string List_contents::macro(map<string, string> &v, const vector<int>& p, string data)
{
    data = "";
    
    if (p.size() < 1)
    {
        error = "Incorrect number of parameters.";
        return "";
    }
    else if (p[0] < 0 || (unsigned)p[0] > p.size() - 1)
    {
        error = "Broken list.";
        return "";
    }
    else if ((unsigned)p[0] < p.size() - 1)
    {
        error = "Incorrect number of parameters.";
        return "";
    }
    
    unsigned size = p[0];

    string s;
    
    for (unsigned i = 0; i < size; i++)
    {
        s += num_to_string(p[i + 1]);
        if (i != size - 1)
            s += ',';
    }

    return s;
}


List_get::List_get()
{
    Macro::hasBraces = false;
    Macro::name = "list_get";
    Macro::implicitParams = "";
}

string List_get::macro(map<string, string> &v, const vector<int>& p, string data)
{
    data = "";
    
    if (p.size() < 1)
    {
        error = "Incorrect number of parameters.";
        return "";
    }
    else if (p[0] < 0 || (unsigned)p[0] > p.size() - 1)
    {
        error = "Broken list.";
        return "";
    }
    else if ((unsigned)p[0] != p.size() - 2)
    {
        error = "Incorrect number of parameters.";
        return "";
    }
    
    unsigned size = p[0];
    if (p[size + 1] < 0 || (unsigned)p[size + 1] >= size)
    {
        error = "Index out of range.";
        return "";
    }
    
    unsigned index = p[size + 1];
    vector<int> contents;
    
    for (unsigned i = 1; i <= size; i++)
    {
        contents.push_back(p[i]);
    }
    
    string s;
    s += num_to_string(contents[index]);

    return s;
}


List_set::List_set()
{
    Macro::hasBraces = false;
    Macro::name = "list_set";
    Macro::implicitParams = "";
}

string List_set::macro(map<string, string>& v, const vector<int>& p, string data)
{
    data = "";
    
    if (p.size() < 1)
    {
        error = "Incorrect number of parameters.";
        return "";
    }
    else if (p[0] < 0 || (unsigned)p[0] > p.size() - 1)
    {
        error = "Broken list.";
        return "";
    }
    else if ((unsigned)p[0] != p.size() - 3)
    {
        error = "Incorrect number of parameters.";
        return "";
    }
    
    unsigned size = p[0];
    if (p[size + 1] < 0 || (unsigned)p[size + 1] >= size)
    {
        error = "Index out of range.";
        return "";
    }
    
    unsigned index = p[size + 1];
    int value = p[size + 2];
    vector<int> contents;
    
    for (unsigned i = 1; i <= size; i++)
    {
        contents.push_back(p[i]);
    }
    
    contents[index] = value;
    
    string s;
    
    for (unsigned i = 0; i <= size; i++)
    {
        if (i == 0)
        {
            s += num_to_string(size) + ", ";
        }
        else
        {
            s += num_to_string(contents[i - 1]);
            
            if (i != size)
                s += ',';
        }
    }

    return s;
}


List_insert::List_insert()
{
    Macro::hasBraces = false;
    Macro::name = "list_insert";
    Macro::implicitParams = "";
}

string List_insert::macro(map<string, string>& v, const vector<int>& p, string data)
{
    data = "";
    
    if (p.size() < 1)
    {
        error = "Incorrect number of parameters.";
        return "";
    }
    else if (p[0] < 0 || (unsigned)p[0] > p.size() - 1)
    {
        error = "Broken list.";
        return "";
    }
    else if ((unsigned)p[0] != p.size() - 3)
    {
        error = "Incorrect number of parameters.";
        return "";
    }
    
    unsigned size = p[0];
    if (p[size + 1] < 0 || (unsigned)p[size + 1] > size)
    {
        error = "Index out of range.";
        return "";
    }
    
    unsigned index = p[size + 1];
    int value = p[size + 2];
    vector<int> contents;
    
    for (unsigned i = 1; i <= size; i++)
    {
        contents.push_back(p[i]);
    }
    
    vector<int>::iterator it = contents.begin();
    contents.insert(it + index, value);
    
    string s;
    
    for (unsigned i = 0; i <= contents.size(); i++)
    {
        if (i == 0)
        {
            s += num_to_string(contents.size()) + ", ";
        }
        else
        {
            s += num_to_string(contents[i - 1]);
            
            if (i != contents.size())
                s += ',';
        }
    }

    return s;
}


List_delete::List_delete()
{
    Macro::hasBraces = false;
    Macro::name = "list_delete";
    Macro::implicitParams = "";
}

string List_delete::macro(map<string, string>& v, const vector<int>& p, string data)
{
    data = "";
    
    if (p.size() < 1)
    {
        error = "Incorrect number of parameters.";
        return "";
    }
    else if (p[0] < 0 || (unsigned)p[0] > p.size() - 1)
    {
        error = "Broken list.";
        return "";
    }
    else if ((unsigned)p[0] != p.size() - 2)
    {
        error = "Incorrect number of parameters.";
        return "";
    }
    
    unsigned size = p[0];
    if (p[size + 1] < 0 || (unsigned)p[size + 1] >= size)
    {
        error = "Index out of range.";
        return "";
    }
    
    unsigned index = p[size + 1];
    vector<int> contents;
    
    for (unsigned i = 1; i <= size; i++)
    {
        contents.push_back(p[i]);
    }
    
    vector<int>::iterator it = contents.begin();
    contents.erase(it + index);
    
    string s;
    
    for (unsigned i = 0; i <= contents.size(); i++)
    {
        if (i == 0)
        {
            s += num_to_string(contents.size()) + ", ";
        }
        else
        {
            s += num_to_string(contents[i - 1]);
            
            if (i != contents.size())
                s += ',';
        }
    }
    
    if (s == "0, ")
    {
        s = "0";
    }

    return s;
}


List_push::List_push()
{
    Macro::hasBraces = false;
    Macro::name = "list_push";
    Macro::implicitParams = "";
}

string List_push::macro(map<string, string>& v, const vector<int>& p, string data)
{
    data = "";
    
    if (p.size() < 1)
    {
        error = "Incorrect number of parameters.";
        return "";
    }
    else if (p[0] < 0 || (unsigned)p[0] > p.size() - 1)
    {
        error = "Broken list.";
        return "";
    }
    else if ((unsigned)p[0] != p.size() - 2)
    {
        error = "Incorrect number of parameters.";
        return "";
    }
    
    unsigned size = p[0];
    int value = p[size + 1];
    vector<int> contents;
    
    for (unsigned i = 1; i <= size; i++)
    {
        contents.push_back(p[i]);
    }
    
    contents.push_back(value);
    
    string s;
    
    for (unsigned i = 0; i <= contents.size(); i++)
    {
        if (i == 0)
        {
            s += num_to_string(contents.size()) + ", ";
        }
        else
        {
            s += num_to_string(contents[i - 1]);
            
            if (i != contents.size())
                s += ',';
        }
    }

    return s;
}


List_pop::List_pop()
{
    Macro::hasBraces = false;
    Macro::name = "list_pop";
    Macro::implicitParams = "";
}

string List_pop::macro(map<string, string>& v, const vector<int>& p, string data)
{
    data = "";
    
    if (p.size() < 1)
    {
        error = "Incorrect number of parameters.";
        return "";
    }
    else if (p[0] < 0 || (unsigned)p[0] > p.size() - 1)
    {
        error = "Broken list.";
        return "";
    }
    else if ((unsigned)p[0] != p.size() - 1)
    {
        error = "Incorrect number of parameters.";
        return "";
    }
    
    unsigned size = p[0];
    vector<int> contents;
    
    for (unsigned i = 1; i <= size; i++)
    {
        contents.push_back(p[i]);
    }
    
    if (size > 0)
    {
        contents.pop_back();
    }
    else
    {
        error = "Cannot pop from an empty list.";
        return "";
    }
    
    string s;
    
    for (unsigned i = 0; i <= contents.size(); i++)
    {
        if (i == 0)
        {
            s += num_to_string(contents.size()) + ", ";
        }
        else
        {
            s += num_to_string(contents[i - 1]);
            
            if (i != contents.size())
                s += ',';
        }
    }
    
    if (s == "0, ")
    {
        s = "0";
    }

    return s;
}


List_top::List_top()
{
    Macro::hasBraces = false;
    Macro::name = "list_top";
    Macro::implicitParams = "";
}

string List_top::macro(map<string, string> &v, const vector<int>& p, string data)
{
    data = "";
    
    if (p.size() < 1)
    {
        error = "Incorrect number of parameters.";
        return "";
    }
    else if (p[0] < 0 || (unsigned)p[0] > p.size() - 1)
    {
        error = "Broken list.";
        return "";
    }
    else if ((unsigned)p[0] != p.size() - 1)
    {
        error = "Incorrect number of parameters.";
        return "";
    }
    
    unsigned size = p[0];
    vector<int> contents;
    
    for (unsigned i = 1; i <= size; i++)
    {
        contents.push_back(p[i]);
    }
    
    string s;
    if (size > 0)
    {
        s += num_to_string(contents[size - 1]);
    }
    else
    {
        error = "Cannot access element of empty list.";
        return "";
    }
    
    return s;
}


List_add::List_add()
{
    Macro::hasBraces = false;
    Macro::name = "list_add";
    Macro::implicitParams = "";
}

string List_add::macro(map<string, string>& v, const vector<int>& p, string data)
{
    data = "";
    
    if (p.size() < 1)
    {
        error = "Incorrect number of parameters.";
        return "";
    }
    else if (p[0] < 0 || (unsigned)p[0] > p.size() - 1)
    {
        error = "Broken list.";
        return "";
    }
    else if ((unsigned)p[0] > p.size() - 2)
    {
        error = "Not enough parameters.";
        return "";
    }
    
    unsigned size = p[0];
    int num = 0;
    for (unsigned i = size + 1; i < p.size(); i++)
    {
        num += p[i];
    }
    
    vector<int> contents;
    
    for (unsigned i = 1; i <= size; i++)
    {
        contents.push_back(p[i] + num);
    }
    
    string s;
    
    for (unsigned i = 0; i <= contents.size(); i++)
    {
        if (i == 0)
        {
            s += num_to_string(contents.size()) + ", ";
        }
        else
        {
            s += num_to_string(contents[i - 1]);
            
            if (i != contents.size())
                s += ',';
        }
    }

    return s;
}

Calc::Calc()
{
    Macro::hasBraces = false;
    Macro::name = "calc";
    Macro::implicitParams = "";
}

string Calc::macro(map<string, string>& v, const vector<int>& p, string data)
{
    data = "";
    
    if (p.size() != 1)
    {
        error = "Incorrect number of parameters.";
        return "";
    }
    
    string s;
    s += num_to_string(p[0]);
    return s;
}

string Macro::convert_to_ticks(map<string, string> &v, string& dt)
{
    Double doubleFix;
    Half halfFix;

    vector<int> noParams;
    string newData;

    // Run the data through the doubling macro and then the halving macro. This converts note lengths to ticks.
    newData = doubleFix.macro(v, noParams, dt);
    newData = halfFix.macro(v, noParams, newData);
    //
    
    return newData;
}
