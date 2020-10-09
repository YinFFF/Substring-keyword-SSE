#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string.h>
#include <math.h>
#include <map>            // std::map
#include <vector>         // std::vector 
#include <sys/time.h>     // gettimeofday
#include <stack>          // std::stack
#include <openssl/hmac.h>
#include <openssl/aes.h>
#include <openssl/rand.h>
#include <stdexcept>

#include "PHIndex.h"
#include "BFIndex.h"
#include "BWT.h"
#include "AES.h"


using namespace std;

void ReadKeywords(vector<string> &keywords_list, char* file_name){
    string keywords;
    ifstream readfile(file_name);
    if(readfile.is_open()){
        while(getline(readfile, keywords, '\n')){
            //if(readfile.eof())
                //keywords.append(1, 0);
            if (keywords.size() == 0) break;
            if(keywords.find('#') != -1) throw invalid_argument("find a # in file\n");
            keywords_list.push_back(keywords);
        }
        readfile.close();
    }
    else
        printf("file open error\n");
    return;
}

void Keywords_to_str(vector<string> &keywords_list, string &keywords_string){
    if (keywords_string[keywords_string.size() - 1] == '\r'){
        keywords_string = keywords_string.substr(0, keywords_string.size() - 1);
    }

    int total_len = 0;
    for(int i = 0; i < keywords_list.size(); i++){
        total_len += (keywords_list[i].size());
    }
    keywords_string.resize(total_len + keywords_list.size());

    int current_position = 0;
    for(int i = 0; i < keywords_list.size(); i++){
        keywords_string.replace(current_position, keywords_list[i].size(), keywords_list[i]);
        current_position += keywords_list[i].size();
        keywords_string.replace(current_position, 1, "#");
        current_position ++;
    }
    return;
}


struct time_count{
    float total_time;
    int test_num;
};

// Test the proposed scheme
int TestSuffixTreeSolution(char *file_name)
{
    cout << "======TestSuffixTreeSolution========\n";
    struct timeval time1,time2;
    long start, end;
    float evaluate_time;

    // initial AES key
    unsigned char aes_key[32];
    RAND_bytes(aes_key, 32);

    // read keywords to keywords_list from the file
    vector<string> keywords_list;
    ReadKeywords(keywords_list, file_name); 

    // transfer keywords_list to a string (i.e., dictioanry string t_W)
    string keywords_string;
    Keywords_to_str(keywords_list, keywords_string);
    
    // Building and encrypt position suffix tree
    gettimeofday(&time1, NULL);
    SuffixTree *suffix_tree = new SuffixTree(keywords_string, keywords_list);
    gettimeofday(&time2, NULL);

    int nodes_size = 0;
    for (int i = 0; i < suffix_tree->nodes.size(); i++) {
        nodes_size += suffix_tree->nodes[i].keyword.size();
        nodes_size += suffix_tree->nodes[i].ch.size() * sizeof(int);
        //nodes_size += suffix_tree->nodes[i].sub.size();
        nodes_size += (2 * sizeof(int) + sizeof(size_t));
    }

    cout << "index of storage: " << nodes_size << endl;

/*
    int subs_size = 0;
    for (int i = 0; i < suffix_tree->subs.size(); i++) {
        subs_size += suffix_tree->subs[i].size();
    }
    cout << "suffix_tree.subs.size(): " << subs_size << endl;
    cout << "after new\n";
    */
    //suffix_tree->clear_subs();
    //std::vector<string>.swap(suffix_tree->subs);
    /*
    for (int i = 0; i < suffix_tree->subs.size(); i++) {
        string(suffix_tree->subs[i]).swap(suffix_tree->subs[i]);
        //suffix_tree->subs[i].clear();
        //suffix_tree->subs[i].shrink_to_fit();

    }*/

    //msec
    evaluate_time = 1000*((time2.tv_sec-time1.tv_sec) + ((double)(time2.tv_usec-time1.tv_usec))/1000000);
    cout << "generate time: " << evaluate_time << "ms" << endl;

    getchar();
    // Search
    // char S[20] = {0};
    // cout << "search keyword:";
    // cin >> S;
    map<int,struct time_count> test_count;
    for(auto it = keywords_list.begin(); it != keywords_list.end(); it++){
        gettimeofday(&time1, NULL);
        vector<string> matching_keywords = suffix_tree->search((*it).c_str(), keywords_string);
        // suffix_tree->search(S, keywords_string);
        gettimeofday(&time2, NULL);
        evaluate_time = 1000*((time2.tv_sec-time1.tv_sec) + ((double)(time2.tv_usec-time1.tv_usec))/1000000);
        
        int key = matching_keywords.size();
        if (test_count.count(key)==0){
            time_count map_value = {evaluate_time, 1};
            test_count[key] = map_value;
        }
        else{
            test_count[key].total_time += evaluate_time;
            test_count[key].test_num += 1;
        }
    }
    for(auto itr = test_count.begin(); itr != test_count.end(); itr++)
        cout << "matching_keywords: " << (itr)->first << ", " << "count: " << (itr)->second.test_num << ", "<<
        "time: " << (itr)->second.total_time/(itr)->second.test_num << endl;

//    for(int i = 0; i < matching_keywords.size(); i++)
//        cout << matching_keywords[i] << endl;

//    cout << matching_keywords.size() << endl;
    
    delete suffix_tree;
    return 0;   
}

int main(int argc, char * argv[])
{
    if (argc != 2){
        cout << "error parameter\n";
        return 0;
    }
    //char file_name[50] = "./Testfile/distinct_word_5000";
    char *file_name = argv[1];
    
    TestSuffixTreeSolution(file_name);
    return 0;
}


