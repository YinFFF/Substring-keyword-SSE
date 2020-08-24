#ifndef POSITION_H
#define POSITION_H

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <map> 
#include <functional>
#include "AES.h"

using namespace std;


class PositionHeap {
    char *T;
    int n;
    
public:
    struct Node {
        map<string, int> childs;
        string keyword;
        int ciphertext_len;
    } *nodes;

    void appendSubtree(int pos, vector<string> &ret, unsigned char *aes_key) {
        for (auto itr = nodes[pos].childs.begin(); itr != nodes[pos].childs.end(); itr++){
            unsigned char output[nodes[itr->second].keyword.size()];
            int plaintext_len;
            plaintext_len = AES_decrypt((unsigned char*)&nodes[itr->second].keyword[AES_BLOCK_SIZE], 
                                         nodes[itr->second].ciphertext_len, 
                                         aes_key, 
                                         (unsigned char*)&nodes[itr->second].keyword[0], 
                                         output);
        
            string plaintext((const char*)output, plaintext_len);
            ret.push_back(plaintext);
            appendSubtree(itr->second, ret, aes_key);
        }
    }

    #define InsertKeywordLen 10
    PositionHeap(const char T[], unsigned char *aes_key, vector<string> &keywords_list) {
        unsigned char iv[AES_BLOCK_SIZE];
        int keyword_index = keywords_list.size();
        n = (int)strlen(T) + InsertKeywordLen;
        this->T = strdup(T);
        // each Node represent a position in the T, and the (n+1)th Node is the root node.
        nodes = new Node[n+1];
        for (int i = n; --i >= InsertKeywordLen; ) {
            const char *q = &T[i - InsertKeywordLen];
            if ((*q) == '#'){
                keyword_index--;
                continue;
            }
            int v = n;
            string childs_key(1, *q);

            while (nodes[v].childs.find(childs_key) != nodes[v].childs.end()){
                v = nodes[v].childs[childs_key];
                childs_key.append((++q), 1);
            }
            nodes[v].childs[childs_key] = i;
            nodes[i].keyword.resize(keywords_list[keyword_index].size() + 2 * AES_BLOCK_SIZE);
            RAND_bytes((unsigned char*)iv, AES_BLOCK_SIZE);
            nodes[i].keyword.replace(0, AES_BLOCK_SIZE, (const char*)iv, AES_BLOCK_SIZE);
            nodes[i].ciphertext_len = AES_encrypt((unsigned char *)&keywords_list[keyword_index][0], 
                                                    keywords_list[keyword_index].size(), 
                                                    aes_key, 
                                                    iv, 
                                                    (unsigned char *)&nodes[i].keyword[AES_BLOCK_SIZE]);
        }
    }
    ~PositionHeap() {
        free(T);
        delete[] nodes;
    }
    
    vector<string> search(const char S[], unsigned char *aes_key) {
       
        vector<string> ret;
        int m = (int)strlen(S), depth = 0, v = n;
        string childs_key;
        
        while (*S){
            childs_key.append(S++, 1);
            
            if (nodes[v].childs.find(childs_key) == nodes[v].childs.end()){
                v = -1;
                break;
            }
            
            v = nodes[v].childs[childs_key];
            
            depth++;
            
            unsigned char output[nodes[v].keyword.size()];

            int plaintext_len = AES_decrypt((unsigned char*)&nodes[v].keyword[AES_BLOCK_SIZE], nodes[v].ciphertext_len, 
                        aes_key, (unsigned char*)&nodes[v].keyword[0], output);
            
            string plaintext((const char*)output, plaintext_len);
            
            ret.push_back(plaintext);
        }
        if (v != -1)
            appendSubtree(v, ret, aes_key);
        return ret;
    }

    void Insert(string &U, unsigned char *aes_key) {
        string insert_keyword = U + "#END";
        unsigned char iv[AES_BLOCK_SIZE];
        for (int i = insert_keyword.size() - 4; --i >= 0; ) {
            const char *q = &insert_keyword[i];
            int v = n;
            string childs_key(1, *q);
            while (nodes[v].childs.find(childs_key) != nodes[v].childs.end()){
                v = nodes[v].childs[childs_key];
                childs_key.append((++q), 1);
            }
            nodes[v].childs[childs_key] = i;
            nodes[i].keyword.resize(U.size() + AES_BLOCK_SIZE);
            RAND_bytes((unsigned char*)iv, AES_BLOCK_SIZE);
            nodes[i].keyword.replace(0, AES_BLOCK_SIZE, (const char*)iv, AES_BLOCK_SIZE);
            nodes[i].ciphertext_len = AES_encrypt((unsigned char *)&U[0], 
                                                    U.size(), 
                                                    aes_key, 
                                                    iv, 
                                                    (unsigned char *)&nodes[i].keyword[AES_BLOCK_SIZE]);
        }
    }
};

#endif
