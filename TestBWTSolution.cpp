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

int TestBWTSolution(char *file_name)
{
    cout << "======TestBWTSolution========\n";
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

    // Building and encrypt position heap
    gettimeofday(&time1, NULL);
    BWT *bwt = new BWT(keywords_string.c_str(), aes_key, keywords_list);
    gettimeofday(&time2, NULL);
    
    int nodes_size = 0;
    nodes_size += bwt->LLSet.size() * (sizeof(int) + sizeof(char));

    for (int i = 0; i < bwt->nodes.size(); i++) {
        nodes_size += bwt->nodes[i]->keyword.size();
        nodes_size += (sizeof(char) + sizeof(bwt_Node *));
    }

    cout << "The storage of Index: " << nodes_size << endl;
    //msec
    evaluate_time = 1000*((time2.tv_sec-time1.tv_sec) + ((double)(time2.tv_usec-time1.tv_usec))/1000000);
    cout << "generate time: " << evaluate_time << "ms" << endl;

    // Search
    /*
    char S[20] = {0};
    cout << "search keyword:";
    cin >> S;
    */
    map<int,struct time_count> test_count;

    for(auto it = keywords_list.begin(); it != keywords_list.end(); it++){
        gettimeofday(&time1, NULL);
        //deque<Node *> matching_keywords = bwt->search(input.c_str(), aes_key);
        deque<bwt_Node *> matching_keywords = bwt->search((*it).c_str(), aes_key);
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

    //msec
    for(auto itr = test_count.begin(); itr != test_count.end(); itr++)
        cout << "matching_keywords.size(): " << (itr)->first << ", " << "matching_count: " << (itr)->second.test_num << ", "<<
        "average_time: " << (itr)->second.total_time/(itr)->second.test_num << endl;

    
    delete bwt;
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
    
    TestBWTSolution(file_name);
    return 0;
}


