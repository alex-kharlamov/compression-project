#include "codec.h"

//
//  main.cpp
//  hzip
//
//  Created by Alexey Kharlamov on 01.02.16.
//  Copyright © 2016 Alexey Kharlamov. All rights reserved.
//

#include "stdafx.h"
#include <algorithm>
#include <iostream>
#include <string>
#include <map>
#include <queue>
#include <fstream>
#include <math.h>
#include <cstdlib>
#include <stdio.h>
#include <ctime>



//std::vector<std::string> dictionary(258, "");

struct ver {
    char letter;
    const ver *leftson, *rightson;
    int counter;
    bool operator() (ver const &a, ver const &b) {
        return a.counter > b.counter;
    }
    ver* myself;
};

void build_dict(ver const &v, std::string str, std::vector<std::string> &dictionary) {
    if (v.letter != '\0') {
        dictionary[int(v.letter) + 128] = str;
    }
    if (v.leftson != NULL) {
        build_dict(*v.leftson, str + "0", dictionary);
    }
    if (v.rightson != NULL) {
        build_dict(*v.rightson, str + "1", dictionary);
    }

}

std::string dec_to_bin(int dec) {
    int i;
    int mod;
    long double_ = 0;

    for (i = 0; dec > 0; i++) {

        mod = dec % 2;
        dec = (dec - mod) / 2;
        double_ += mod * pow((double)10, i);
    }
    return std::to_string(double_);

}


unsigned char pack_byte(bool bits[8])
{
    unsigned char result(0);
    for (unsigned i(8); i--;)
    {
        result <<= 1;
        result |= unsigned char(bits[i]);
    }
    return result;
}

void encode() {
    std::vector<std::string> dictionary(258, "");
    unsigned long start_time = clock();


    std::ifstream file("lorem.txt");

    //std::string   result;

    //std::copy(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>(), std::back_inserter(result));
    std::vector<int> dict(256, 0);
    std::cout << "ok";
    int w = 0;
    std::string buffer;
    
    
    while (std::getline(file, buffer))
    {
        for (size_t i = 0; i < buffer.length(); i++)
        {
            dict[int(buffer[i]) + 128] += 1;
        }
        if (w % 100000 == 0)
        {
            std::cout << w << std::endl;
        }
        ++w;
       
    }

    

    file.close();


    std::cout << "reading " << (clock() - start_time) / (double)CLOCKS_PER_SEC << std::endl;



    std::priority_queue<ver, std::vector<ver>, ver> qu;

    std::vector<std::pair<int, char>> chardict;
    for (int i = 0; i < 256; ++i) {
        if (dict[i] > 0) {
            chardict.push_back(std::make_pair(dict[i], char(i - 128)));
        }
    }

    std::sort(chardict.begin(), chardict.end());

    for (int i = 0; i < chardict.size(); ++i) { //инициализация вершин
        ver* temp = new ver;
        temp->myself = temp;
        temp->counter = chardict[i].first;
        temp->letter = chardict[i].second;
        temp->leftson = NULL;
        temp->rightson = NULL;
        qu.push(*temp);

    }

    unsigned long init = clock() - start_time;
    std::cout << "init " << init / (double)CLOCKS_PER_SEC << std::endl;

    while (qu.size() > 1) { //постройка дерева
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
        newver->letter = '\0';
        qu.push(*newver);
    }

    unsigned long tree = clock();
    std::cout << "build tree " << (tree - init) / (double)CLOCKS_PER_SEC << std::endl;

    build_dict(qu.top(), "", dictionary); //создание словаря

    unsigned long dic = clock();
    std::cout << "build dict " << (dic - tree) / (double)CLOCKS_PER_SEC << std::endl;

    std::ofstream fout("dict.txt");
    int counter = 0;

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
    std::cout << "building dictionary " << builddict / (double)CLOCKS_PER_SEC - dic / (double)CLOCKS_PER_SEC << std::endl;

    std::ofstream codeout("coded");
    std::ifstream filenew("lorem.txt");

    int j = -1;
    std::string buff = "";
    bool bits[8];


    while (!filenew.eof())
    {
        std::string buffer;
        std::getline(filenew, buffer);


        for (unsigned long long i = 0; i < buffer.length(); ++i) {
            //codeout << dictionary[int(buffer[i]) + 128];
            // попробуем без упаковки
            for (auto elem : dictionary[int(buffer[i]) + 128]) {
                j += 1;
                bits[j] = bool(elem - '0');


                if (j == 7) {
                    buff += pack_byte(bits);
                    j = -1;

                }

                if (buff.length() > 10) {
                    codeout << buff;
                    buff = "";
                }


            }
    }


    }
    


    codeout << buff;

    codeout.close();
    std::cout << "writng " << (clock() - builddict) / (double)CLOCKS_PER_SEC << std::endl;
    std::cout << "all " << (clock() - start_time) / (double)CLOCKS_PER_SEC << std::endl;



}



void decode() {
    std::ifstream dicin("dict.txt");
    int n;
    dicin >> n;
    std::cout << n << std::endl;
    std::map<std::string, char> dict;

    for (int i = 0; i < n; ++i) {
        std::string str;
        char let;
        dicin >> let >> str;
        dict[str] = let;
    }

    dicin.close();


    std::ifstream in("coded");

    std::ofstream out("output.txt");
    std::string mem = "";

    while (!in.eof()) {
        std::string tempstr;
        char temp = in.get();
        //std::cout << temp;
        mem += temp;
        //std::getline(in, tempstr);
        //for (int i = 0; i < tempstr.length(); ++i) {
        //std::string str = dec_to_bin(int(tempstr[i]));
        //mem += tempstr[i];
        //}

    }
    //std::cout << mem << std::endl;
    in.close();

    std::string tempstr = "";
    for (int i = 0; i < mem.length(); ++i) {
        tempstr += mem[i];
        try {
            out << dict.at(tempstr);

            tempstr = "";
        }
        catch (const std::out_of_range& oor) {
            continue;
        }

    }

    out.close();


}

int main() {
    encode();

    std::cout << "coding done" << std::endl;
    decode();
    int n;
    std::cin >> n;

}


