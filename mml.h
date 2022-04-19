#ifndef MML_H
#define MML_H

#include "global.h"

#include "brr.h"
#include "sample.h"
#include "music.h"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <set>

enum Parameter
{
    UNSIGNED = 0,
    SIGNED = 1,
    DECIMAL = 0,
    SIGNED_DECIMAL = 1,
    HEXADECIMAL = 2,
    BINARY = 4
};

const byte BASE_NOTE = 0x80;
const byte CAP_NOTE = 0xC7;
const byte TIE = 0xC8;
const byte REST = 0xC9;
const byte WHOLE_NOTE = 0xC0;

struct Macro
{
    std::string body;
    std::vector<std::string> parameters;
};

class Mml
{
    private:
        struct Loop
        {
            unsigned dataIndex; // Location in byte array of the ARAM pointer to loop data.
            unsigned destIndex; // The loop index, which will ultimately determine the ARAM pointer for the loop to jump to.

            Loop(unsigned data, unsigned dest)
            {
                dataIndex = data;
                destIndex = dest;
            }
        };

        std::string data;

        bool isError;
        std::string END_OF_FILE;
        std::string NO_VALUE;
        std::string INVALID_VALUE;
        
        std::string file;
        std::vector<std::string> fileStack;
        std::vector<unsigned> lineStack;
        
        std::set<std::string> defines;
        std::map<std::string, Macro*> macros;
        std::vector<std::string> paramStack;
        std::vector< std::vector<int> > lists;
        std::map<unsigned, unsigned> listIDs;
        std::map<std::string, std::string> variables;
        std::map<std::string, std::string> constants;
        std::map<std::string, std::string> userDefined;
        bool isAssignment;
        bool isQuote;
        bool isEvalMacro;
        std::string assignVar;
        std::string assignContents;
        unsigned preAssignIndex;
        unsigned assignLength;
        bool isInQuotes;

        unsigned lineNum;
        int currentChan;
        
        std::map< std::string, std::vector<std::string> > kits;

        std::vector<unsigned> patterns;
        std::vector< std::vector<int> > tracks;
        unsigned short patternStart;
        bool isPatternDefined;
        
        std::map<std::string, Sample> samples;
        byte sampleSlots;
        unsigned short sampleDataOffset;

        std::vector<Loop> loopOrigins;
        std::vector<unsigned short> loopDestinations;
        std::map<std::string, unsigned> loopLabels;

        std::string currentLabel;

        bool shouldLoop;
        bool hasIntro;

        char command;
        byte octave;
        byte transpose;
        byte defaultLength;
        byte prevTicks;
        byte dynamic;
        byte prevDynamic;
        bool isTriplet;
        bool isLoop;

    public:
        Mml();
        ~Mml();
        void load_file(std::string filename);
        bool preprocess();
        bool interpret(Music& music);

    private:
        void remove_comments();
        void replace();
        bool add_defines();
        void add_implicit_params();
        bool write_variables(std::string& dt, unsigned index, bool& isLabel);
        bool read_variables(std::string& dt, unsigned index);
        bool assign_variables(std::string& dt, unsigned& index);
        bool set_variable(std::string key, std::string value);
        void print_variables();
        bool expand_macros(std::string& dt);
        bool expand_loops(std::string& dt, unsigned index);
        bool expand_kits();
        std::string get_macro_data(std::string& dt, const std::vector<int>& p, std::string macroname);
        std::string list_push(const std::vector<int>& p);
        std::string list_pop(const std::vector<int>& p);
        std::string list_top(const std::vector<int>& p);
        std::string list_get(const std::vector<int>& p);
        std::string list_set(const std::vector<int>& p);
        std::string list_size(const std::vector<int>& p);
        std::string list(const std::vector<int>& p);
        std::string list_create(const std::vector<int>& p);
        std::string list_destroy(const std::vector<int>& p);
        std::string list_exists(const std::vector<int>& p);
        int find_list_id(unsigned n);
        void init();
        void pattern_init();
        int note_to_num(std::string note);
        int char_to_num(std::string chr);
        bool is_integer(const std::string& s);
        bool is_whitespace(char c);
        bool is_bit(char c);
        bool is_digit(char c);
        bool is_nybble(char c);
        int string_to_num(const std::string& s);
        std::string num_to_string(int n);
        unsigned hex_to_dec(const std::string& s);
        unsigned bin_to_dec(const std::string& s);
        bool iterate(char& chr, unsigned& i);
        void update_line(char& c);
        unsigned get_line_num(const std::string& dt, unsigned index);
        void print_error(const std::string& s);
        void print_warning(const std::string& s);
        int get_parameter(unsigned paramNum, Parameter type, char& chr, unsigned& i);
        std::string evaluate_macro(std::string& dt, unsigned &index);
        std::vector<int> get_macro_parameters(std::string args);
        std::string evaluate_condition(std::string macroname, const std::vector<int>& parameters, std::string& dt, unsigned& index);
        std::string remove_whitespace(std::string s);
        std::string strip_newlines(std::string s);
        std::string enter_file(const std::string& dt, unsigned& i);
        std::string include_binary(std::string filename);
};

#endif // MML_H
