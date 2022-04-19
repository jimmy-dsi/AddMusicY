#ifndef MACROS_H
#define MACROS_H

#include "global.h"
#include "byte.h"
#include "macros.h"

#include <iostream>
#include <iomanip>
#include <vector>
#include <map>
#include <sstream>
#include <cmath>
#include <stdlib.h>

using namespace std;

class Macro
{
    protected:
        string name;
        string implicitParams;
        string error;
        bool hasBraces;

    public:
        virtual ~Macro() {}
        virtual string macro(map<string, string>& v, const vector<int>& p, string data = "") = 0;

        bool is_error();
        bool has_braces();
        void clear_error();
        string get_implicit_params();
        string get_error();
        int string_to_num(const string& s);
        string num_to_string(int n);
        string dec_to_hex(int n);
        int bin_to_dec(const string& bin);
        unsigned hex_to_dec(const string& hex);
        string char_to_string(char c);
        string convert_to_ticks(map<string, string> &v, string& dt);
};

class Scale: public Macro
{
    enum Type
    {
        MAJOR,
        HARMONIC_MINOR,
        MAJOR_PENTATONIC,
        MINOR_PENTATONIC,
        CHROMATIC,
        WHOLE_TONE,
        MELODIC_MINOR
    };

    private:
        int key;
        int offset;
        int direction;
        Type type;
        int noteLength;
        int numNotes;

    public:
        Scale();
        virtual string macro(map<string, string>& v, const vector<int>& p, string data);
};

class Arp: public Macro
{
    private:
        int root;
        int noteLength;
        vector<int> offsets;

    public:
        Arp();
        virtual string macro(map<string, string>& v, const vector<int>& p, string data);
};

class LoopM: public Macro
{
    private:
        int initOctave;
        int initTranspose;
        int iterations;

    public:
        LoopM();
        virtual string macro(map<string, string>& v, const vector<int>& p, string data);
};

class Double: public Macro
{
    public:
        Double();
        virtual string macro(map<string, string>& v, const vector<int>& p, string data);
};

class Half: public Macro
{
    public:
        Half();
        virtual string macro(map<string, string>& v, const vector<int>& p, string data);
};

class Bpm: public Macro
{
    private:
        float bpm;

    public:
        Bpm();
        virtual string macro(map<string, string>& v, const vector<int>& p, string data);
};

class Hex: public Macro
{
    private:
        int dec;

    public:
        Hex();
        virtual string macro(map<string, string>& v, const vector<int>& p, string data);
};

class Hexc: public Macro
{
    private:
        int dec;

    public:
        Hexc();
        virtual string macro(map<string, string>& v, const vector<int>& p, string data);
};

class Binary: public Macro
{
    private:
        char dec;

    public:
        Binary();
        virtual string macro(map<string, string>& v, const vector<int>& p, string data);
};

class Add: public Macro
{
    public:
        Add();
        virtual string macro(map<string, string>& v, const vector<int>& p, string data);
};

class Sub: public Macro
{
    public:
        Sub();
        virtual string macro(map<string, string>& v, const vector<int>& p, string data);
};

class Mul: public Macro
{
    public:
        Mul();
        virtual string macro(map<string, string>& v, const vector<int>& p, string data);
};

class Div: public Macro
{
    public:
        Div();
        virtual string macro(map<string, string>& v, const vector<int>& p, string data);
};

class Mod: public Macro
{
    public:
        Mod();
        virtual string macro(map<string, string>& v, const vector<int>& p, string data);
};

class And: public Macro
{
    public:
        And();
        virtual string macro(map<string, string>& v, const vector<int>& p, string data);
};

class Or: public Macro
{
    public:
        Or();
        virtual string macro(map<string, string>& v, const vector<int>& p, string data);
};

class Not: public Macro
{
    public:
        Not();
        virtual string macro(map<string, string>& v, const vector<int>& p, string data);
};

class Vrand: public Macro
{
    private:
        int seed;
        int min;
        int max;
        int initVolume;

    public:
        Vrand();
        virtual string macro(map<string, string>& v, const vector<int>& p, string data);
};

class Qrand: public Macro
{
    private:
        int seed;
        int min;
        int max;
        int initLegato;
        int initVelocity;

    public:
        Qrand();
        virtual string macro(map<string, string>& v, const vector<int>& p, string data);
};

class Yrand: public Macro
{
    private:
        int seed;
        int min;
        int max;
        int initPan;

    public:
        Yrand();
        virtual string macro(map<string, string>& v, const vector<int>& p, string data);
};

class Portamento: public Macro
{
    private:
        int octave;
        int transposition;
        int length;
        bool isLegato;

    public:
        Portamento();
        virtual string macro(map<string, string>& v, const vector<int>& p, string data);
};

class Arpeggiator: public Macro
{
    enum Type2
    {
        MAJOR,
        MINOR,
        DIMINISHED,
        HALF_DIMINISHED,
        DOMINANT
    };

    private:
        int seed;
        int key;
        Type2 type;
        int depth;
        int noteLength;
        int numNotes;

    public:
        Arpeggiator();
        virtual string macro(map<string, string>& v, const vector<int>& p, string data);
};

class Echo_Enable: public Macro
{
    private:
        int echoFlags;
        int echoRightVol;
        int echoLeftVol;
        int echoDelay;
        int echoFeedback;
        int echoFIR;
        int echoEnableBits;

    public:
        Echo_Enable();
        virtual string macro(map<string, string>& v, const vector<int>& p, string data);
};

class Echo_Disable: public Macro
{
    private:
        int echoFlags;
        int echoRightVol;
        int echoLeftVol;
        int echoDelay;
        int echoFeedback;
        int echoFIR;
        int echoEnableBits;

    public:
        Echo_Disable();
        virtual string macro(map<string, string>& v, const vector<int>& p, string data);
};

class Adsr: public Macro
{
    private:
        int attack;
        int decay;
        int sustain;
        int release;

    public:
        Adsr();
        virtual string macro(map<string, string>& v, const vector<int>& p, string data);
};

class Gain: public Macro
{
    private:
        int mode;
        int value;

    public:
        Gain();
        virtual string macro(map<string, string>& v, const vector<int>& p, string data);
};

class Pitch: public Macro
{
    public:
        Pitch();
        virtual string macro(map<string, string>& v, const vector<int>& p, string data);
};

class Smw: public Macro
{
    public:
        Smw();
        virtual string macro(map<string, string>& v, const vector<int>& p, string data);
};

class Qmix: public Macro
{
    public:
        Qmix();
        virtual string macro(map<string, string>& v, const vector<int>& p, string data);
};

class Table: public Macro
{
    private:
        int instrument;
        int fineTune;
        int articulation;
        int volume;
        int pan;
        char command;
        int ticks;
        int loopPoint;
        vector<int> table;

    public:
        Table();
        virtual string macro(map<string, string>& v, const vector<int>& p, string data);
};

class List_size: public Macro
{
    public:
        List_size();
        virtual string macro(map<string, string>& v, const vector<int>& p, string data);
};

class List_contents: public Macro
{
    public:
        List_contents();
        virtual string macro(map<string, string>& v, const vector<int>& p, string data);
};

class List_get: public Macro
{
    public:
        List_get();
        virtual string macro(map<string, string>& v, const vector<int>& p, string data);
};

class List_set: public Macro
{
    public:
        List_set();
        virtual string macro(map<string, string>& v, const vector<int>& p, string data);
};

class List_insert: public Macro
{
    public:
        List_insert();
        virtual string macro(map<string, string>& v, const vector<int>& p, string data);
};

class List_delete: public Macro
{
    public:
        List_delete();
        virtual string macro(map<string, string>& v, const vector<int>& p, string data);
};

class List_push: public Macro
{
    public:
        List_push();
        virtual string macro(map<string, string>& v, const vector<int>& p, string data);
};

class List_pop: public Macro
{
    public:
        List_pop();
        virtual string macro(map<string, string>& v, const vector<int>& p, string data);
};

class List_top: public Macro
{
    public:
        List_top();
        virtual string macro(map<string, string>& v, const vector<int>& p, string data);
};

class List_add: public Macro
{
    public:
        List_add();
        virtual string macro(map<string, string>& v, const vector<int>& p, string data);
};

class Calc: public Macro
{
    public:
        Calc();
        virtual string macro(map<string, string>& v, const vector<int>& p, string data);
};

#endif // MACROS_H
