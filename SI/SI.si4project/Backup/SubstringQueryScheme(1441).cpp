#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string.h>
#include <math.h>
#include <map>            // std::map
#include <vector>         // std::vector 
#include <sys/time.h>     // gettimeofday
#include <stack>          // std::stack
#include <pbc.h>
#include <pbc_test.h>

#include "AES.h"
#include <openssl/hmac.h>
#include <openssl/aes.h>
#include <openssl/rand.h>
#include <stdexcept>

#include "PositionHeap.h"

#include "bloom_filter.hpp"


using namespace std;

void ReadKeywords(vector<string> &keywords_list, char* file_name){
    string keywords;
    ifstream readfile(file_name);
    if(readfile.is_open()){
        while(getline(readfile, keywords, '\n')){
            if(readfile.eof())
                keywords.append(1, 0);
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
    int total_len = 0;
    for(int i = 0; i < keywords_list.size(); i++){
        total_len += (keywords_list[i].size() - 1);
    }
    keywords_string.resize(total_len + keywords_list.size());

    int current_position = 0;
    for(int i = 0; i < keywords_list.size(); i++){
        keywords_string.replace(current_position, keywords_list[i].size() - 1, keywords_list[i]);
        current_position += keywords_list[i].size() - 1;
        keywords_string.replace(current_position, 1, "#");
        current_position ++;
    }
    return;
}

struct time_count{
    float total_time;
    int test_num;
};

#define CAL_REPEAT  5

// Test for outsourcing files
map<int,struct time_count> TestNewSolution(char *file_name)
{
    struct timeval time1,time2;
    
//    cout << "file_name:";
//    cin >> file_name;
    
    // initial AES key
    unsigned char aes_key[32];

    RAND_bytes(aes_key, 32);

    // read keywords from the file
    vector<string> keywords_list; 
    
    ReadKeywords(keywords_list, file_name); 

//    gettimeofday(&time1,NULL);
    
    // transfer keywords_list to a string
    string keywords_string;
    
    Keywords_to_str(keywords_list, keywords_string);

    // Building and encrypt position heap
    PositionHeap *heap = new PositionHeap(keywords_string.c_str(), aes_key, keywords_list);
    
//    gettimeofday(&time2,NULL);

    //sec
//    float evaluate_time = ((time2.tv_sec-time1.tv_sec) + ((double)(time2.tv_usec-time1.tv_usec))/1000000);
    
//    cout << "Outsourcing time: " << evaluate_time << endl;

    float evaluate_time = 0;

    string insert_keyword;
    
    int Repeat_time = 100;
//    int Keyword_len = 7;
    int Keyword_len;
    cout << "keyword_len:";
    cin >> Keyword_len;

    int count = 0;

    // Insert 
    for (int i = 0; i < Repeat_time; i++){
        if (keywords_list[i].size() > Keyword_len){
            insert_keyword.assign(keywords_list[i], 0, Keyword_len);
            gettimeofday(&time1,NULL);
            heap->Insert(insert_keyword, aes_key);
            gettimeofday(&time2,NULL);
            count++;
            evaluate_time += ((time2.tv_sec-time1.tv_sec) + ((double)(time2.tv_usec-time1.tv_usec))/1000000);
        }
    }
    
    

    cout << "keyword_len:" << Keyword_len <<", count: " << count << ", insert time: " << evaluate_time/count << endl;
    
 
    
    // Search
    map<int,struct time_count> test_count;
//
//    char S[20] = {0};
//    cout << "search keyword:";
//    cin >> S;
    
//    for(auto it = keywords_list.begin(); it != keywords_list.end(); it++){
//        gettimeofday(&time1,NULL);
        
//        vector<string> matching_keywords = heap->search((*it).c_str(), aes_key);
//        vector<string> matching_keywords = heap->search(S, aes_key);

//        for(int i = 0; i < matching_keywords.size(); i++)
//            cout << matching_keywords[i] << endl;
    
//        gettimeofday(&time2,NULL);

        //msec
//        float evaluate_time = 1000*((time2.tv_sec-time1.tv_sec) + ((double)(time2.tv_usec-time1.tv_usec))/1000000);

        
//        if (test_count.count((matching_keywords).size())==0){
//            time_count map_value = {evaluate_time, 1};
//            test_count[(matching_keywords).size()] = map_value;
//        }
//        else{
//            test_count[(matching_keywords).size()].total_time += evaluate_time;
//            test_count[(matching_keywords).size()].test_num += 1;
//        }
//    }
    
//    for(auto itr = test_count.begin(); itr != test_count.end(); itr++)
//        cout << "matching_keywords size: " << (itr)->first << ", " <<  "time: " << 
//        (itr)->second.total_time/(itr)->second.test_num << endl;
//        cout << "matching_keywords size: " << (itr)->first << ", " << "count: " << (itr)->second.test_num << ", "<<
//        "time: " << (itr)->second.total_time/(itr)->second.test_num << endl;
    
    delete heap;
    return test_count;   
}



map<int,struct time_count> TestNaiveSolution(char *file_name)
{
    struct timeval time1,time2;
    
    // read keywords from the file
    vector<string> keywords_list; 
    
    ReadKeywords(keywords_list, file_name); 

    // initial AES key
    unsigned char aes_key[32];
    
    RAND_bytes(aes_key, 32);

   
    BFIndex *index[keywords_list.size()];
    
    for (int i = 0; i < keywords_list.size(); i++){
        index[i] = new BFIndex(keywords_list[i], aes_key);
    }

    // Insert
    string insert_keyword;
    int Repeat_time = 100;
    int Keyword_len;
    cout << "keyword_len:";
    cin >> Keyword_len;
    int count = 0;
    float evaluate_time = 0;
    int BF_size;
    
    for (int i = 0; i < Repeat_time; i++){
        if (keywords_list[i].size() > Keyword_len){
            insert_keyword.assign(keywords_list[i], 0, Keyword_len);
            gettimeofday(&time1,NULL);
            BFIndex *insert_index = new BFIndex(insert_keyword, aes_key);
            gettimeofday(&time2,NULL);
            count++;
            BF_size = sizeof
            delete insert_index;
            evaluate_time += ((time2.tv_sec-time1.tv_sec) + ((double)(time2.tv_usec-time1.tv_usec))/1000000);
        }
    }

    cout << "keyword_len:" << Keyword_len <<", count: " << count << ", insert time: " << evaluate_time/count << endl;
    
    
    // Search
    map<int,struct time_count> test_count;

    //    char S[20] = {0};
    //    
    //    cout << "search keyword:";
    //    
    //    cin >> S;
        
    //    string search_keyword(S);
    
//    for(auto it = keywords_list.begin(); it != keywords_list.end(); it++){
//        vector<string> matching_keywords;
//        string return_keyword;
//    
//        gettimeofday(&time1,NULL);
//    
//        for (int i = 0; i < keywords_list.size(); i++){
//            return_keyword = index[i] -> search((*it), aes_key);
//            
//            if (return_keyword.size() > 0){
//                matching_keywords.push_back(return_keyword);
//            }
//        }
//    
//        gettimeofday(&time2,NULL);
//
//        // msec
//        float evaluate_time = 1000*((time2.tv_sec-time1.tv_sec) + ((double)(time2.tv_usec-time1.tv_usec))/1000000);
//        
//        if (test_count.count((matching_keywords).size())==0){
//            time_count map_value = {evaluate_time, 1};
//            test_count[(matching_keywords).size()] = map_value;
//        }
//        else{
//            test_count[(matching_keywords).size()].total_time += evaluate_time;
//            test_count[(matching_keywords).size()].test_num += 1;
//        }
//    }
    
//    for(auto itr = test_count.begin(); itr != test_count.end(); itr++)
//        cout << "matching_keywords size: " << (itr)->first << ", " << "count: " << (itr)->second.test_num << ", "<<  "time: " << 
//        (itr)->second.total_time/(itr)->second.test_num << endl;
      
    return test_count;
}

int main(int argc, char * argv[])
{
    
    char file_name[20] = "distinct_word_5000";
    
//    TestNewSolution(file_name);
    TestNaiveSolution(file_name);

    
    return 0;
}


