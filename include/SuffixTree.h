#ifndef SUFFIXTREE_H
#define SUFFIXTREE_H

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


class SuffixTree {
    class Node {
    public:
        size_t edge_hash;
        vector<int> ch;    // vector of child nodes
        int left_index;
        int right_index;
        string keyword;

        Node() {
            edge_hash = 0;
            keyword = "";
            left_index = -1;
            right_index = -1;
        }

        Node(int left_index, int right_index, std::initializer_list<int> children, int pos, const string& suf, const string& keyword) : keyword(keyword) {
            ch.insert(ch.end(), children);
            this->left_index = left_index;
            this->right_index = right_index;
            string hash_string = suf.substr(0, left_index);
            edge_hash = hash<string>{}(hash_string);
        }
    };

public:
    vector<Node> nodes;
    

    SuffixTree(string & str, vector<string> &keywords_list) {
        int keyword_index = keywords_list.size();
        str.append("$");
        nodes.push_back(Node{});
        // subs.push_back("");
        for (int pos = str.length() - 2; pos >= 0; pos--) {
            if (str[pos] == '#') {
                keyword_index--;
                continue;
            }
            addSuffix(str, pos, keywords_list[keyword_index]);
        }
    }

    void addSuffix(const std::string & suf, int pos, const string & keyword) {
        int n = 0, x2, n2;

        for (int i = pos; i < suf.length(); ) {
            x2 = 0;
            while (true) {
                vector<int> children = nodes[n].ch;
                if (x2 == children.size()) {
                    // no matching child, remainder of suf becomes new node
                    n2 = nodes.size();
                    nodes.push_back(Node(i, suf.size() - 1, {}, pos, suf, keyword));
                    nodes[n].ch.push_back(n2);
                    return;
                }
                n2 = children[x2];
                if (suf[nodes[n2].left_index] == suf[i]) {
                    break;
                }
                x2++;
            }
            // find prefix of remaining suffix in common with child
            int sub2_left_index = nodes[n2].left_index;
            int sub2_right_index = nodes[n2].right_index;
            size_t j = 0;
            while (j < sub2_right_index - sub2_left_index + 1) {
                if (suf[i + j] != suf[sub2_left_index + j]) {
                    // split n2
                    int n3 = n2;
                    // new node for the part in common
                    n2 = nodes.size();
                    nodes.push_back(Node(sub2_left_index, sub2_left_index + j - 1, {n3}, -1, suf, ""));
                    nodes[n3].left_index = j;
                    nodes[n3].right_index = suf.size() - 1;
                    nodes[n].ch[x2] = n2;
                    break; // continue down the tree
                }
                j++;
            }
            i += j; // advance past part in common
            n = n2; // continue down the tree
        }
    }
    /*
    void recursion_print(int n, const std::string & pre){
        auto children = nodes[n].ch;
        if (children.size() == 0) {
            std::cout << "- " << nodes[n].sub << '\n';
            // std::cout << "- " << subs[n] << '\n';
            return;
        }

        std::cout << "+ " << nodes[n].sub << '\n';
        // std::cout << "+ " << subs[n] << '\n';

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
*/

    void appendSubtree(int cur_node_index, vector<string> &ret) {
        static int count = 0;
        if (nodes[cur_node_index].ch.size() == 0) {
            ret.push_back(nodes[cur_node_index].keyword);
            // cout << ++count << ": " << nodes[cur_node_index].keyword << endl;
            return;
        }
        
        for (auto p = nodes[cur_node_index].ch.begin(); p != nodes[cur_node_index].ch.end(); p++){
            appendSubtree(*p, ret);
        }
    }    

    vector<string> search(const char S[], const string& suf) {
        vector<string> ret;
        int m = strlen(S);
        int cur_node_index = 0, S_index = 0;


        // find the node, whose edges matching the queried string S
        while (S_index < m) {
            auto chs = nodes[cur_node_index].ch;
            int j;
            for (j = 0; j < chs.size(); j++) {
                if (suf[nodes[chs[j]].left_index] == S[S_index]) {
                    int sub_len = min(nodes[chs[j]].right_index - nodes[chs[j]].left_index + 1, m - S_index);
                    if (suf.compare(nodes[chs[j]].left_index, sub_len, &S[S_index], sub_len) == 0){
                        S_index += sub_len;
                        cur_node_index = chs[j];
                        break;
                    }
                    else
                        return ret;
                }
            }
            if (j == chs.size()){
                return ret;
            }
        }

        appendSubtree(cur_node_index, ret);
        return ret;
    }


};
#endif
