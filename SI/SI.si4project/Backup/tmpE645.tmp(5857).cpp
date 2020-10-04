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

    gettimeofday(&time1,NULL);
    
    // transfer keywords_list to a string
    string keywords_string;
    
    Keywords_to_str(keywords_list, keywords_string);
    
    // Building and encrypt position heap
    PositionHeap *heap = new PositionHeap(keywords_string.c_str(), aes_key, keywords_list);
    
    gettimeofday(&time2,NULL);

    //sec
    float evaluate_time = ((time2.tv_sec-time1.tv_sec) + ((double)(time2.tv_usec-time1.tv_usec))/1000000);
    
    cout << "Outsourcing time: " << evaluate_time << endl;
    
//    char S[20] = {0};
//    cout << "search keyword:";
//    cin >> S;
    
    // Search
    map<int,struct time_count> test_count;
    
    for(auto it = keywords_list.begin(); it != keywords_list.end(); it++){
        gettimeofday(&time1,NULL);
        
        vector<string> matching_keywords = heap->search((*it).c_str(), aes_key);

//        for(int i = 0; i < matching_keywords.size(); i++)
//            cout << matching_keywords[i] << endl;
    
        gettimeofday(&time2,NULL);

        //msec
        evaluate_time = 1000*((time2.tv_sec-time1.tv_sec) + ((double)(time2.tv_usec-time1.tv_usec))/1000000);
        
        if (test_count.count((matching_keywords).size())==0){
            time_count map_value = {evaluate_time, 1};
            test_count[(matching_keywords).size()] = map_value;
        }
        else{
            test_count[(matching_keywords).size()].total_time += evaluate_time;
            test_count[(matching_keywords).size()].test_num += 1;
        }
    }
    
//    for(auto itr = test_count.begin(); itr != test_count.end(); itr++)
//        cout << "matching_keywords size: " << (itr)->first << ", " <<  "time: " << 
//        (itr)->second.total_time/(itr)->second.test_num << endl;
//        cout << "matching_keywords size: " << (itr)->first << ", " << "count: " << (itr)->second.test_num << ", "<<
//        "time: " << (itr)->second.total_time/(itr)->second.test_num << endl;
    
    delete heap;
    return test_count;   
}

class BFIndex { 
    bloom_parameters parameters;
    bloom_filter *filter;
    string ciphertext_keyword;
    int ciphertext_len;
    
public:
    BFIndex(string &plaintext_keyword, unsigned char *aes_key){
        
        struct timeval time1,time2;

        // How many elements roughly do we expect to insert?
        parameters.projected_element_count = 1000;
 
        // Maximum tolerable false positive probability? (0,1)
        parameters.false_positive_probability = 0.0001; // 1 in 10000
 
        // Simple randomizer (optional)
        parameters.random_seed = 0xA5A5A5A5;
 
        if (!parameters){
            std::cout << "Error - Invalid set of bloom filter parameters!" << std::endl;
        }

        parameters.compute_optimal_parameters();
        
        filter = new bloom_filter(parameters); 
        
        for (int j = 0; j < plaintext_keyword.size(); j++)
            for (int z = j; z < plaintext_keyword.size(); z++)
                filter->insert(plaintext_keyword.substr(j, z - j + 1));

        ciphertext_keyword = plaintext_keyword;
        
        ciphertext_keyword.resize(plaintext_keyword.size() + AES_BLOCK_SIZE);
        
        unsigned char iv[AES_BLOCK_SIZE];
        
        RAND_bytes((unsigned char*)iv, AES_BLOCK_SIZE);
        
        ciphertext_keyword.replace(0, AES_BLOCK_SIZE, (const char*)iv, AES_BLOCK_SIZE);

        ciphertext_len = AES_encrypt((unsigned char *)&plaintext_keyword[0], 
                                                    plaintext_keyword.size(), 
                                                    aes_key, 
                                                    iv, 
                                                    (unsigned char *)&ciphertext_keyword[AES_BLOCK_SIZE]);
    }

    // 
    string search(string &search_keyword, unsigned char *aes_key) {
        string result;
        
        if (filter -> contains(search_keyword)){
            unsigned char output[ciphertext_keyword.size()];
            
            int plaintext_len = AES_decrypt((unsigned char*)&ciphertext_keyword[AES_BLOCK_SIZE], ciphertext_len, 
                        aes_key, (unsigned char*)&ciphertext_keyword[0], output);
            
            result.assign((const char*)output, plaintext_len -1);   
        }
        
        return result;
    }
    
    ~BFIndex(){
        delete filter;
    }
    
};
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

//    char S[20] = {0};
//    
//    cout << "search keyword:";
//    
//    cin >> S;
    
//    string search_keyword(S);

    // Search
    map<int,struct time_count> test_count;
    
    for(auto it = keywords_list.begin(); it != keywords_list.end(); it++){
        vector<string> matching_keywords;
        string return_keyword;
    
        gettimeofday(&time1,NULL);
    
        for (int i = 0; i < keywords_list.size(); i++){
            return_keyword = index[i] -> search((*it), aes_key);
            
            if (return_keyword.size() > 0){
                matching_keywords.push_back(return_keyword);
            }
        }
    
        gettimeofday(&time2,NULL);

        // msec
        float evaluate_time = 1000*((time2.tv_sec-time1.tv_sec) + ((double)(time2.tv_usec-time1.tv_usec))/1000000);
        
        if (test_count.count((matching_keywords).size())==0){
            time_count map_value = {evaluate_time, 1};
            test_count[(matching_keywords).size()] = map_value;
        }
        else{
            test_count[(matching_keywords).size()].total_time += evaluate_time;
            test_count[(matching_keywords).size()].test_num += 1;
        }
    }
    
//    for(auto itr = test_count.begin(); itr != test_count.end(); itr++)
//        cout << "matching_keywords size: " << (itr)->first << ", " << "count: " << (itr)->second.test_num << ", "<<  "time: " << 
//        (itr)->second.total_time/(itr)->second.test_num << endl;
    
    
    return test_count;
}



int main(int argc, char * argv[])
{
    
    char file_name[20] = "distinct_word_35000";
    
    map<int,struct time_count> result_new;
    map<int,struct time_count> result_naive;
    
    result_new = TestNewSolution(file_name);
    result_naive = TestNaiveSolution(file_name);

    for(auto itr = result_new.begin(); itr != result_new.end(); itr++)
        if (result_naive.count((itr)->first) > 0){
            cout << "matching_keywords size: " << (itr)->first << ", " 
                <<  "new time: " << (itr)->second.total_time/(itr)->second.test_num << ", "
                <<  "naive time: " << (result_naive[(itr)->first]).total_time/(result_naive[(itr)->first]).test_num << endl;
                
        }

    return 0;
}


