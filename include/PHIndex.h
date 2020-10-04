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




class SuffixTree {
    class Node {
        public:
        string sub;   // a substring of the input string
        vector<int> ch;    // vector of child nodes
        //string keyword;
        int pos;
 
        Node() {
            sub = "";
            pos = -1;
        }
       
        Node(const std::string& sub, std::initializer_list<int> children, int pos) : sub(sub) {
            ch.insert(ch.end(), children);
            this->pos = pos;
        }
    };

    std::vector<Node> nodes;
    
public:

    //SuffixTree(const char T[], unsigned char *aes_key, vector<string> &keywords_list) {
    SuffixTree(const std::string& str) {
        nodes.push_back(Node{});
        for (int i = str.length() - 1; i >= 0; i--) {
            cout << "i : " << i << endl;
            addSuffix(str.substr(i), i);
        }
    }

    void recursion_print(int n, const std::string & pre){
        auto children = nodes[n].ch;
        if (children.size() == 0) {
            std::cout << "- " << nodes[n].sub << ": " << nodes[n].pos <<'\n';
            return;
        }
        std::cout << "+ " << nodes[n].sub << '\n';

        auto it = children.begin();
        if (it != children.end()) do {
            if ( (it + 1) == children.end()) break;
            std::cout << pre << "+-";
            recursion_print(*it, pre + "| ");
            it++;
        } while (true);

        std::cout << pre << "+-";
        recursion_print(children[children.size() - 1], pre + "  ");
    }

    void visualize() {
        if (nodes.size() == 0) {
            std::cout << "<empty>\n";
            return;
        }
        recursion_print(0, "");
    }

    void addSuffix(const std::string & suf, int pos) {
        cout << "================= addSuffix: "<< suf << endl;
        int n = 0, i = 0, x2, n2;
        char b;

        while (i < suf.length()) {
            b = suf[i];
            x2 = 0;
            // find a node, i.e., nodes[n2], whose sub is "b..."
            while (true) {
                auto children = nodes[n].ch;
                if (x2 == children.size()) {
                    // no matching child, remainder of suf becomes new node
                    n2 = nodes.size();
                    nodes.push_back(Node(suf.substr(i), {}, pos));
                    nodes[n].ch.push_back(n2);
                    return;
                }
                n2 = children[x2];
                if (nodes[n2].sub[0] == b) {
                    break;
                }
                x2++;
            }
            // find prefix of remaining suffix in common with child
            auto sub2 = nodes[n2].sub;
            size_t j = 0;
            while (j < sub2.size()) {
                if (suf[i + j] != sub2[j]) {
                    // split n2
                    auto n3 = n2;
                    // new node for the part in common
                    n2 = nodes.size();
                    nodes.push_back(Node(sub2.substr(0, j), { n3 }, -1));
                    nodes[n3].sub = sub2.substr(j); // old node loses the part in common
                    nodes[n].ch[x2] = n2;
                    break; // continue down the tree
                }
                j++;
            }
            i += j; // advance past part in common
            n = n2; // continue down the tree
        }
    }

    void appendSubtree(int cur_node_index, vector<string> &ret) {
        if (nodes[cur_node_index].ch.size() == 0) {
            cout << nodes[cur_node_index].sub << ": " << nodes[cur_node_index].pos << endl;
            return;
        }
        
        for (auto p = nodes[cur_node_index].ch.begin(); p != nodes[cur_node_index].ch.end(); p++){
            appendSubtree(*p, ret);
        }

    }    
    void search(const char S[]) {
        size_t m = strlen(S);
        size_t cur_node_index = 0, S_index = 0, x2, n2;
        char b;

        // find the node, whose edges matching the queried string S
        while (S_index < m) {
            auto chs = nodes[cur_node_index].ch;
            int j;
            for (j = 0; j < chs.size(); j++) {
                if (nodes[chs[j]].sub[0] == S[S_index]) {
                    int sub_len = min(nodes[chs[j]].sub.size(), m - S_index);
                    if (nodes[chs[j]].sub.compare(0, sub_len, &S[S_index], sub_len) == 0){
                        S_index += sub_len;
                        cur_node_index = chs[j];
                        break;
                    }
                    else
                        return;
                }
            }
            if (j == chs.size()){
                return;
            }
        }

        vector<string> ret;
        appendSubtree(cur_node_index, ret);
    }

/*
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
    
            //unsigned char output[nodes[v].keyword.size()];

            //int plaintext_len = AES_decrypt((unsigned char*)&nodes[v].keyword[AES_BLOCK_SIZE], nodes[v].ciphertext_len, 
             //           aes_key, (unsigned char*)&nodes[v].keyword[0], output);
            
            //string plaintext((const char*)output, plaintext_len);
            
            //ret.push_back(nodes[v].keyword);
        }
        if (v != -1)
            appendSubtree(v, ret, aes_key);
        return ret;
    }
    */

};
#endif
