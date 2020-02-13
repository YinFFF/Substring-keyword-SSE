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

#include "PHIndex.h"
#include "BFIndex.h"


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


// Test the proposed scheme
int TestNewSolution(char *file_name)
{
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
    PositionHeap *heap = new PositionHeap(keywords_string.c_str(), aes_key, keywords_list);
    
    // Search
    char S[20] = {0};
    cout << "search keyword:";
    cin >> S;
    vector<string> matching_keywords = heap->search(S, aes_key);
    for(int i = 0; i < matching_keywords.size(); i++)
        cout << matching_keywords[i] << endl;
    cout << matching_keywords.size() << endl;
    
    delete heap;
    return 0;   
}


// test naive solution
int TestNaiveSolution(char *file_name)
{   
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
    
    // Search
    char S[20] = {0};    
    cout << "search keyword:";
    cin >> S;
    string search_keyword(S);
    vector<string> matching_keywords;
    string return_keyword;

    for (int i = 0; i < keywords_list.size(); i++){
        return_keyword = index[i] -> search(search_keyword, aes_key);
        
        if (return_keyword.size() > 0){
            matching_keywords.push_back(return_keyword);
        }
    }

    for(auto itr = matching_keywords.begin(); itr != matching_keywords.end(); itr++)
        cout << *itr << endl;
    cout << matching_keywords.size() << endl;
      
    return 0;
}

int main(int argc, char * argv[])
{
    
    char file_name[30] = "./Testfile/distinct_word_5000";
    
    TestNewSolution(file_name);
//    TestNaiveSolution(file_name);

    return 0;
}


