#ifndef BLOOMFILTER_H
#define BLOOMFILTER_H

#include <stdio.h>
#include <iostream>
#include "bloom_filter.hpp"

using namespace std;


class BFIndex {
        
public:
    struct Node {
        map<string, int> childs;
        string keyword;
        int ciphertext_len;
    } *nodes;

    void appendSubtree(int pos, vector<string> &ret, unsigned char *aes_key) {
        for (auto itr = nodes[pos].childs.begin(); itr != nodes[pos].childs.end(); itr++){
            unsigned char output[nodes[itr->second].keyword.size()];
//            unsigned char output[100];
            int plaintext_len;
            plaintext_len = AES_decrypt((unsigned char*)&nodes[itr->second].keyword[AES_BLOCK_SIZE], 
                                         nodes[itr->second].ciphertext_len, 
                                         aes_key, 
                                         (unsigned char*)&nodes[itr->second].keyword[0], 
                                         output);
        
//            output[plaintext_len] = 0;
//            printf("%s\n", output);
            string plaintext((const char*)output, plaintext_len - 1);
            ret.push_back(plaintext);
//            usleep(1000);
            appendSubtree(itr->second, ret, aes_key);
        }
    }
    PositionHeap(const char T[], unsigned char *aes_key, vector<string> &keywords_list) {
//        cout << "=================build positionheap\n";
        unsigned char iv[AES_BLOCK_SIZE];
        int keyword_index = keywords_list.size();
        n = (int)strlen(T);
        this->T = strdup(T);
        nodes = new Node[n+1];
        for (int i = n; --i >= 0; ) {
            const char *q = &T[i];
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
            nodes[i].keyword.resize(keywords_list[keyword_index].size() + AES_BLOCK_SIZE);
            RAND_bytes((unsigned char*)iv, AES_BLOCK_SIZE);
            nodes[i].keyword.replace(0, AES_BLOCK_SIZE, (const char*)iv, AES_BLOCK_SIZE);;
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
       
//        printf("========================before search\n");
        vector<string> ret;
        int m = (int)strlen(S), depth = 0, v = n;
        string childs_key;
        
        while (*S){
            childs_key.append(S++, 1);
            if (nodes[v].childs.find(childs_key) == nodes[v].childs.end()){
                v = -1;
                break;
            }
//            if (strncmp(T+v+depth, S, m-depth) == 0)
            v = nodes[v].childs[childs_key];
            depth++;
//            ret.push_back(nodes[v].keyword);
            unsigned char output[nodes[v].keyword.size()];
            int plaintext_len;
            plaintext_len = AES_decrypt((unsigned char*)&nodes[v].keyword[AES_BLOCK_SIZE], nodes[v].ciphertext_len, 
                        aes_key, (unsigned char*)&nodes[v].keyword[0], output);
//            output[plaintext_len] = 0;
//            printf("%s\n", output);
            string plaintext((const char*)output, plaintext_len -1);
            ret.push_back(plaintext);
//            usleep(1000);
        }
//        printf("======================before search appendsubtree\n");
        if (v != -1)
            appendSubtree(v, ret, aes_key);
        return ret;
    }
};

#endif
