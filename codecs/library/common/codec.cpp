//
//  main.cpp
//  hzip
//
//  Created by Alexey Kharlamov on 01.02.16.
//  Copyright © 2016 Alexey Kharlamov. All rights reserved.
//

#include <iostream>
#include <string>
#include <map>
#include <queue>
#include <fstream>
#include <math.h>
#include <cstdlib>
#include <stdio.h>
#include <ctime>
#include <algorithm>
#include <bitset>
#if 0
#include <experimental/string_view>
#endif


//std::vector<std::string> dictionary(258, "");

struct ver {
    char letter;
    const ver *leftson, *rightson;
    unsigned long long counter;
    bool operator() (ver const &a, ver const &b) {
        return a.counter > b.counter;
    }
    ver* myself;
    bool letterused = false;
};

void build_dict(ver const &v, std::string str, std::vector<std::string> &dictionary) {
    if (v.letterused) {
        dictionary[int(v.letter) + 128] = str;
    }
    if (v.leftson != nullptr) {
        build_dict(*v.leftson, str + "0", dictionary);
    }
    if (v.rightson != nullptr) {
        build_dict(*v.rightson, str + "1", dictionary);
    }
    
}

std::string dec_to_bin(int dec) {
    int i;
    int mod;
    long double_ = 0;
    
    for (i = 0; dec>0; i++) {
        
        mod = dec % 2;
        dec = (dec - mod) / 2;
        double_ += mod * pow((double)10, i);
    }
    
    std::string res = std::to_string(double_);
    
    if (res.length() < 7) {
        std::string temp = "";
        for (int i = res.length(); i < 7; ++i) {
            temp += "0";
        }
        res = temp + res;
    }
    return res;
    
}


char pack_byte(bool bits[7]) {
#if 0
    unsigned char result(0);
    for (unsigned i(8); i--;) {
        result <<= 1;
        result |= unsigned char(bits[i]);
    }
    return result;
#endif
    short pow[8] = { 128, 64, 32, 16, 8, 4, 2, 1 };
    auto res = 0;
    for (int i = 0; i < 8; ++i) {
        res += int(bits[i]) * pow[i];
    }
    return res;
    
}

void load_file(std::string& s, std::istream& is) {
    s.erase();
    if (is.bad()) return;
    //
    // attempt to grow string buffer to match file size,
    // this doesn't always work...
    s.reserve(is.rdbuf()->in_avail());
    char c;
    while (is.get(c)) {
        // use logarithmic growth stategy, in case
        // in_avail (above) returned zero:
        if (s.capacity() == s.size())
            s.reserve(s.capacity() * 3);
        s.append(1, c);
    }
}

void encode() {
    unsigned long long buff_out_size = 11000000;
    std::vector<std::string> dictionary(258, "");
    unsigned long start_time = clock();
    
    std::ifstream file("data2");
    
    
    if (file) {
        
        // get length of file:
        file.seekg(0, file.end);
        unsigned long long file_length = file.tellg();
        file.seekg(0, file.beg);
        /*
        char * buffer = new char[length];
        
        // read data as a block:
        file.read(buffer, length);
         
        */
        std::string buffer;
        buffer.reserve(file_length);
        
        std::string s;
        
        while (!file.eof())
        {
            std::getline(file, s);
            buffer += s + "\n";
        }
        buffer.erase(buffer.length() - 1, 1);
        unsigned long long length = buffer.length();
        
        std::cout << "reading " << (clock() - start_time) / (double)CLOCKS_PER_SEC << std::endl;
        
        std::vector<int> dict(256, 0);
        
        for (unsigned long long i = 0; i < length; ++i) {
            dict[int(static_cast<unsigned char> (buffer[i]))] += 1;
        }
        
        std::priority_queue<ver, std::vector<ver>, ver> qu;
        
        std::vector<std::pair<int, char>> chardict;
        for (int i = 0; i < 256; ++i) {
            if (dict[i] > 0) {
                chardict.push_back(std::make_pair(dict[i], char(i)));
            }
        }
        
        std::sort(chardict.begin(), chardict.end());
        
        for (int i = 0; i < chardict.size(); ++i) { //vertex init
            ver* temp = new ver;
            temp->myself = temp;
            temp->counter = chardict[i].first;
            temp->letter = chardict[i].second;
            temp->leftson = nullptr;
            temp->rightson = nullptr;
            temp->letterused = true;
            qu.push(*temp);
            
        }
        
        unsigned long init = clock() - start_time;
        std::cout << "init " << init / (double)CLOCKS_PER_SEC << std::endl;
        
        while (qu.size() > 1) { //build tree
            const ver* left;
            const ver* right;
            left = qu.top().myself;
            qu.pop();
            right = qu.top().myself;
            qu.pop();
            ver* newver = new ver;
            newver->myself = newver;
            newver->leftson = left;
            newver->rightson = right;
            newver->counter = left->counter + right->counter;
            qu.push(*newver);
        }
        
        unsigned long tree = clock();
        std::cout << "build tree " << (tree - init) / (double)CLOCKS_PER_SEC << std::endl;
        
        build_dict(qu.top(), "", dictionary); //making dict
        
        unsigned long dic = clock();
        std::cout << "build dict " << (dic - tree) / (double)CLOCKS_PER_SEC << std::endl;
        
        std::ofstream fout("dict.txt");
        unsigned long long counter = 0;
        
        for (int i = 0; i < dictionary.size(); ++i) {
            if (dictionary[i] != "") {
                counter++;
            }
        }
        
        
        fout << counter << std::endl; //в 1 строке каждого словаря хранится кол-во слов в нем КОСТЫЛИ!!! ИСПРАВИТЬ!!!
        
        for (int i = 0; i < dictionary.size(); ++i) {
            if (dictionary[i] != "") {
                fout << char(i - 128) << " " << dictionary[i] << std::endl;
            }
        }
        
        fout.close();
        
        unsigned long builddict = clock();
        std::cout << "writing dict " << builddict / (double)CLOCKS_PER_SEC - dic / (double)CLOCKS_PER_SEC << std::endl;
        
        std::ofstream codeout("coded");
        
        int j = -1;
        std::string buff = "";
        buff.reserve(length);
        bool bits[8];
        
        for (unsigned long long i = 0; i < length; ++i) {
            //codeout << dictionary[int(buffer[i]) + 128];
            // попробуем без упаковки
            //std::cout << dictionary[int(buffer[i]) + 128] << " ";
            for (auto elem : dictionary[int(buffer[i]) + 128]) {
                
                j += 1;
                bits[j] = bool(elem - '0');
                
                
                if (j == 7) {
                    buff += pack_byte(bits);
                    if (buff.length() > buff_out_size) {
                        codeout << buff;
                        buff = "";
                    }
                    j = -1;
                }
                
                
            }
            
        }
        //delete[] buffer;
        
        
        codeout << buff;
        for (int i = 0; i < j + 1; ++i) {
            codeout << bits[i];
        }
        codeout << j + 1;
        //std::cout << buff;
        
        codeout.close();
        std::cout << "writing " << (clock() - builddict) / (double)CLOCKS_PER_SEC << std::endl;
        std::cout << "coding " << (clock() - start_time) / (double)CLOCKS_PER_SEC << std::endl;
    }
    
    
}

struct decode_huf_ver {
    char letter;
    decode_huf_ver *leftson, *rightson;
    decode_huf_ver* myself;
    bool used = false;
};



void decode() {
    unsigned long long buff_out_size = 11000000;
    unsigned long start = clock();
    std::ifstream dicin("dict.txt");
    std::string n;
    std::getline(dicin, n);
    //std::cout << std::stoi(n) << std::endl;
    std::map<std::string, char> dict;
    decode_huf_ver root;
    root.leftson = &root;
    root.rightson = &root;
    root.myself = &root;
    for (int i = 0; i < std::stoi(n); ++i) {
        std::string str;
        char let;
        let = dicin.get();
        std::getline(dicin, str);
        //std::cout << let << " " << str << std::endl;
        //let = str[0];
        //str.erase(0, 2);
        dict[str] = let;
        decode_huf_ver *temp = new decode_huf_ver;
        temp = &root;
        for (unsigned long long j = 0; j < str.length(); ++j) {
            if (str[j] == '1') {
                
                if (temp->rightson != &root) {
                    temp = temp->rightson;
                    if (j == str.length() - 1) {
                        temp->letter = let;
                        temp->used = true;
                        break;
                    }
                } else {
                    decode_huf_ver *temp_temp = new decode_huf_ver;
                    temp_temp->leftson = &root;
                    temp_temp->rightson = &root;
                    temp->rightson = temp_temp;
                    temp = temp->rightson;
                    if (j == str.length() - 1) {
                        temp->letter = let;
                        temp->used = true;
                        break;
                    }
                    
                }
            }
            if (str[j] == '0') {
                
                
                if (temp->leftson != &root) {
                    temp = temp->leftson;
                    if (j == str.length() - 1) {
                        temp->letter = let;
                        temp->used = true;
                        break;
                    }
                } else {
                    decode_huf_ver *temp_temp = new decode_huf_ver;
                    temp_temp->leftson = &root;
                    temp_temp->rightson = &root;
                    temp->leftson = temp_temp;
                    temp = temp->leftson;
                    if (j == str.length() - 1) {
                        temp->letter = let;
                        temp->used = true;
                        break;
                    }
                    
                    
                }
                
            }
        }
        
        //std::cout << let << " " << str << " " <<  str.length() <<  std::endl;
    }
    
    
    dicin.close();
    
    const char *fileName("coded");
    std::ifstream file(fileName, std::ios::binary);
    //std::ifstream file("coded", std::ios::binary);
    std::string buffer;
    
    file.seekg(0, std::ios_base::end);
    std::ifstream::pos_type len = file.tellg();
    file.seekg(0);
    buffer.resize(len);
    file.read((char*)buffer.data(), len);
    unsigned long long length = len;
    
    unsigned long dict_coded = clock();
    std::cout << "reading dict with coded file " << (dict_coded - start) / (double)CLOCKS_PER_SEC << std::endl;
    
    std::ofstream out("output");
    std::string mem = "";
    mem.reserve(length);
    std::string buff = "";
    buff.reserve(length);
    //std::cout << buffer[length - 1] << std::endl;
    
    decode_huf_ver *temp = new decode_huf_ver;
    temp = &root;
    unsigned long long j = 0;
    
    std::string dec_to_str_vec [256];
    
    for (int i = 0; i < 256; ++i) {
        std::bitset<8> bits = i;
        
        dec_to_str_vec[i] = bits.to_string();
    }
    
    for (unsigned long long i = 0; i < length - buffer[length - 1] + '0' - 1; ++i) {
        //std::cout << buffer[i] << " " << int(buffer[i]) << std::endl;
        //mem += dec_to_bin(int(buffer[i]));
        //std::bitset<7> bits = int(buffer[i]);
        
        //mem += bits.to_string<char, std::char_traits<char>, std::allocator<char> >();
        mem += dec_to_str_vec[(256 + buffer[i]) % 256];
        
        if (mem.length() > buff_out_size * 10) {
            while (j < mem.length()) {
                if (mem[j] == '1') {
                    temp = temp->rightson;
                    
                    if (temp->used == true) {
                        buff += temp->letter;
                        if (buff.length() > buff_out_size ) {
                            out << buff;
                            buff = "";
                        }
                        temp = &root;
                        
                    }
                } else {
                    temp = temp->leftson;
                    
                    if (temp->used == true) {
                        buff += temp->letter;
                        if (buff.length() > buff_out_size ) {
                            out << buff;
                            buff = "";
                        }
                        temp = &root;
                        
                    }
                }
                
                j += 1;
            }
        
        j = 0;
        mem = "";
        
        }
    }
    
    for (unsigned long long i = length - buffer[length - 1] + '0' - 1; i < length; ++i) {
        mem += buffer[i];
        
        
    }
    unsigned long memorised = clock();
    std::cout << "outmemorised " << (memorised - dict_coded) / (double)CLOCKS_PER_SEC << std::endl;

    while (j < mem.length()) {
            if (mem[j] == '1') {
                    temp = temp->rightson;
                
                    if (temp->used == true) {
                        buff += temp->letter;
                        if (buff.length() > buff_out_size ) {
                            out << buff;
                            buff = "";
                        }
                        temp = &root;
                    
                }
            } else {
                    temp = temp->leftson;
                
                    if (temp->used == true) {
                        buff += temp->letter;
                        if (buff.length() > buff_out_size ) {
                            out << buff;
                            buff = "";
                        }
                        temp = &root;
                    
                }
            }
        
        j += 1;
    }

    
    
    std::cout << std::endl << std::endl;
    
    unsigned long outbuff = clock();
    std::cout << "outbufing " << (outbuff - memorised) / (double)CLOCKS_PER_SEC << std::endl;
    
    
    out << buff;
    
    out.close();
    std::cout << "decoding done in  " << (clock() - start) / (double)CLOCKS_PER_SEC << std::endl;
    
}

int main() {
    encode();
    
    std::cout << "coding done" << std::endl;
    decode();
    
}
