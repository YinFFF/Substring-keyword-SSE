#ifndef POSITION_H
#define POSITION_H

#include <algorithm>
#include <exception>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <deque>
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
    } *nodes;


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

            nodes[i].keyword = keywords_list[keyword_index];

            /*
            nodes[i].keyword.resize(keywords_list[keyword_index].size() + 2 * AES_BLOCK_SIZE);
            RAND_bytes((unsigned char*)iv, AES_BLOCK_SIZE);
            nodes[i].keyword.replace(0, AES_BLOCK_SIZE, (const char*)iv, AES_BLOCK_SIZE);
            nodes[i].ciphertext_len = AES_encrypt((unsigned char *)&keywords_list[keyword_index][0], 
                                                    keywords_list[keyword_index].size(), 
                                                    aes_key, 
                                                    iv, 
                                                    (unsigned char *)&nodes[i].keyword[AES_BLOCK_SIZE]);
            */
        }
    }

    ~PositionHeap() {
        free(T);
        delete[] nodes;
    }
    
    void appendSubtree(int pos, vector<string> &ret, unsigned char *aes_key) {
        for (auto itr = nodes[pos].childs.begin(); itr != nodes[pos].childs.end(); itr++){
            int plaintext_len;
            /*
            unsigned char output[nodes[itr->second].keyword.size()];
            plaintext_len = AES_decrypt((unsigned char*)&nodes[itr->second].keyword[AES_BLOCK_SIZE], 
                                         nodes[itr->second].ciphertext_len, 
                                         aes_key, 
                                         (unsigned char*)&nodes[itr->second].keyword[0], 
                                         output);
        
            string plaintext((const char*)output, plaintext_len);
            */
            ret.push_back(nodes[itr->second].keyword);
            appendSubtree(itr->second, ret, aes_key);
        }
    }

    vector<string> search(const char S[], unsigned char *aes_key) {
       
        vector<string> ret;
        vector<int> ret2;
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
    
            /*
            unsigned char output[nodes[v].keyword.size()];

            int plaintext_len = AES_decrypt((unsigned char*)&nodes[v].keyword[AES_BLOCK_SIZE], nodes[v].ciphertext_len, 
                        aes_key, (unsigned char*)&nodes[v].keyword[0], output);
            
            string plaintext((const char*)output, plaintext_len);
            */
            
            ret.push_back(nodes[v].keyword);
        }
        if (v != -1)
            appendSubtree(v, ret, aes_key);
        return ret;
    }

    void Insert(string &U, unsigned char *aes_key) {
        // In reality, the user should choose a random number instead of "END"
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
            nodes[i].keyword = U;

            /*
            nodes[i].keyword.resize(U.size() + AES_BLOCK_SIZE);
            RAND_bytes((unsigned char*)iv, AES_BLOCK_SIZE);
            nodes[i].keyword.replace(0, AES_BLOCK_SIZE, (const char*)iv, AES_BLOCK_SIZE);
            nodes[i].ciphertext_len = AES_encrypt((unsigned char *)&U[0], 
                                                    U.size(), 
                                                    aes_key, 
                                                    iv, 
                                                    (unsigned char *)&nodes[i].keyword[AES_BLOCK_SIZE]);
            */
        }
    }
};



struct Node {
    char F;
    Node *L;
    int SA;
    string keyword;
};

bool Sort (Node *a, Node *b) {return a->F > b->F;}

class BWT {
    int len;
    map<char, int> LLSet;
    vector<Node *> nodes;

    public:

    BWT(const char T[], unsigned char *aes_key, vector<string> &keywords_list) {

        len = strlen(T);

        nodes.resize(len, NULL);

        int keyword_index = keywords_list.size();

        // each element in nodes is a pointer to a class Node
        for (int i = 0; i < len; i++){
            nodes[i] = new Node;
        }

        for (int i = len - 1; i > 0; i--){
            nodes[i]->F = T[i];
            nodes[i]->L = nodes[i - 1];
            nodes[i]->SA = i;

            if(T[i] == '#'){
                keyword_index--;
            }else{
                nodes[i]->keyword = keywords_list[keyword_index];
            }
        }

        nodes[0]->F = T[0];
        nodes[0]->L = nodes[len - 1];
        nodes[0]->SA = 0;

        sort(nodes.begin(), nodes.end(), Sort);

        char cur = nodes[0]->F;
        LLSet[cur] = 0;
        for (int i = 1; i < len; i++){
            if (nodes[i]->F != cur){
                cur = nodes[i]->F;
                LLSet[cur] = i;
            }
        }


    }
    
    ~BWT() {
        for (int i = 0; i < len; i++)
            delete nodes[i];
    }

    deque<Node *> search(const char S[], unsigned char *aes_key) {
        int n = strlen(S);
        deque<Node *> node_queue;
        //int ret = 0;
        //vector<string> ret;

        if (n == 0 || LLSet.find(S[n - 1]) == LLSet.end()){
            return node_queue;
        }else{
            int cur_index = LLSet[S[n - 1]];
            node_queue.push_back(nodes[cur_index]);
            while (cur_index + 1 < nodes.size() && nodes[cur_index + 1]->F == S[n - 1]){
                node_queue.push_back(nodes[cur_index + 1]);
                cur_index++;
            }
        }

        for (int i = n - 2; i >= 0 && node_queue.size(); i--) {
            int left_len = node_queue.size();
            while (left_len--) {
                Node *cur_node = node_queue.front();
                node_queue.pop_front();
                if (cur_node->L->F == S[i])
                    node_queue.push_back(cur_node->L);
            }
        }
        
        return node_queue;
        /*
        for (int i = 0; i < node_queue.size(); i++){
            ret.push_back(node_queue[i]->keyword);
        }
        return ret;
        */
    }
};


class SuffixTree {
    char *T;
    int n;
    
public:
    struct Node {
        map<string, int> childs;
        string keyword;
    } *nodes;

    #define InsertKeywordLen 10
    SuffixTree(const char T[], unsigned char *aes_key, vector<string> &keywords_list) {
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

            nodes[i].keyword = keywords_list[keyword_index];

            /*
            nodes[i].keyword.resize(keywords_list[keyword_index].size() + 2 * AES_BLOCK_SIZE);
            RAND_bytes((unsigned char*)iv, AES_BLOCK_SIZE);
            nodes[i].keyword.replace(0, AES_BLOCK_SIZE, (const char*)iv, AES_BLOCK_SIZE);
            nodes[i].ciphertext_len = AES_encrypt((unsigned char *)&keywords_list[keyword_index][0], 
                                                    keywords_list[keyword_index].size(), 
                                                    aes_key, 
                                                    iv, 
                                                    (unsigned char *)&nodes[i].keyword[AES_BLOCK_SIZE]);
            */
        }
    }

    ~SuffixTree() {
        free(T);
        delete[] nodes;
    }
    
    void appendSubtree(int pos, vector<string> &ret, unsigned char *aes_key) {
        for (auto itr = nodes[pos].childs.begin(); itr != nodes[pos].childs.end(); itr++){
            int plaintext_len;
            /*
            unsigned char output[nodes[itr->second].keyword.size()];
            plaintext_len = AES_decrypt((unsigned char*)&nodes[itr->second].keyword[AES_BLOCK_SIZE], 
                                         nodes[itr->second].ciphertext_len, 
                                         aes_key, 
                                         (unsigned char*)&nodes[itr->second].keyword[0], 
                                         output);
        
            string plaintext((const char*)output, plaintext_len);
            */
            ret.push_back(nodes[itr->second].keyword);
            appendSubtree(itr->second, ret, aes_key);
        }
    }

    vector<string> search(const char S[], unsigned char *aes_key) {
       
        vector<string> ret;
        vector<int> ret2;
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
    
            /*
            unsigned char output[nodes[v].keyword.size()];

            int plaintext_len = AES_decrypt((unsigned char*)&nodes[v].keyword[AES_BLOCK_SIZE], nodes[v].ciphertext_len, 
                        aes_key, (unsigned char*)&nodes[v].keyword[0], output);
            
            string plaintext((const char*)output, plaintext_len);
            */
            
            ret.push_back(nodes[v].keyword);
        }
        if (v != -1)
            appendSubtree(v, ret, aes_key);
        return ret;
    }

    void Insert(string &U, unsigned char *aes_key) {
        // In reality, the user should choose a random number instead of "END"
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
            nodes[i].keyword = U;

            /*
            nodes[i].keyword.resize(U.size() + AES_BLOCK_SIZE);
            RAND_bytes((unsigned char*)iv, AES_BLOCK_SIZE);
            nodes[i].keyword.replace(0, AES_BLOCK_SIZE, (const char*)iv, AES_BLOCK_SIZE);
            nodes[i].ciphertext_len = AES_encrypt((unsigned char *)&U[0], 
                                                    U.size(), 
                                                    aes_key, 
                                                    iv, 
                                                    (unsigned char *)&nodes[i].keyword[AES_BLOCK_SIZE]);
            */
        }
    }
};
#endif
