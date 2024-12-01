#include <iostream>
#include <fstream>
#include <iterator>
#include <utility>
#include <sstream>
#include <vector>
#include <string>
#include <unordered_map>

using namespace std;

struct hash_pair {
    template <class T1, class T2>
    size_t operator()(const pair<T1, T2>& p) const
    {
        size_t hash1 = hash<T1>{}(p.first);
        size_t hash2 = hash<T2>{}(p.second);
        // Combine the two hash values
        return hash1 ^ (hash2 + 0x9e3779b9 + (hash1 << 6) + (hash1 >> 2));
    }
};

double get_emission_probability(vector<string> maps,string type) {
    double probability = 0.0;
    int occ = 0;
    for (const auto& element : maps) {
        if (element == type) {
            occ++;
        }
    }
    probability = (double) occ / (double) maps.size();
    return probability;
}



int main() {
    ifstream data("./data.txt");
    ifstream posdata("./pos.txt");
    vector<vector<string>> sentences;
    vector<vector<string>> pos;
    string line;

    if (!data.is_open() || !posdata.is_open()) {
        cerr << "Error opening file" << endl;
        return 1;
    }

    cout << "Processing Data .." << endl;
    while (getline(data, line)) {
        if (!line.empty()) {
            istringstream iss(line);
            vector<string> words((istream_iterator<string>(iss)), istream_iterator<string>());
            sentences.push_back(words);
        }
    }
    while (getline(posdata, line)) {
        if (!line.empty()) {
            vector<string> words_pos;
            stringstream ss(line);
            string word_pos;
            while (getline(ss, word_pos, ',')) {
                words_pos.push_back(word_pos);
            }

            pos.push_back(words_pos);
        }
    }
    cout << "Number of sentences processed: " << sentences.size() << endl;
    cout << "Number of pos processed: " << pos.size() << endl;

   /*
    *   Building of the emission matrix.
    */
    unordered_map<string, vector<string>> emission_matrix;
    for(int i = 0; i < sentences.size(); i++) {
          for(int j = 0; j < sentences[i].size(); j++) {
                emission_matrix[sentences[i][j]].push_back(pos[i][j]);
          }
    }

    cout << "Sentence map of : " << emission_matrix.size() << " unique words." << endl;
    cout << "Fish --> verb : " << get_emission_probability(emission_matrix["fish"],"verb") << ", noun :"<< get_emission_probability(emission_matrix["fish"],"noun") <<  endl;

    /*
     *  Building of the transition matrix.
     */
    unordered_map<pair<string, string>, int, hash_pair> translation_matrix;
    unordered_map<string, int> sum_of_first_word_pos;

    for(int i = 0; i < pos.size(); i++) {
        for(int j = 0; j < pos[i].size(); j++) {
            string first_word_pos = "";
            string second_word_pos = "";

            if(j == 0)
                first_word_pos = "start";
            if(j == pos[i].size() - 1)
                break;
            if(j == pos[i].size() - 2)
                second_word_pos = "end";

            if(first_word_pos == "")
                first_word_pos = pos[i][j];
            if(second_word_pos == "")
                second_word_pos = pos[i][j+1];

            pair<string, string> key = make_pair(first_word_pos, second_word_pos);
            
            translation_matrix[key] += 1;
            sum_of_first_word_pos[key.first] += 1;
        }
    }

    unordered_map<pair<string, string>, double, hash_pair> linear_trans_prob_matrix;

    for (auto key_value : translation_matrix) {
        pair<string, string> key = key_value.first;
        int value = key_value.second;

        linear_trans_prob_matrix[key] = 1.0 * value / sum_of_first_word_pos[key.first];

        printf("[%8s, %8s] => %.3f \n", key.first.c_str(), key.second.c_str(), linear_trans_prob_matrix[key]);
    }

    return 0;
}
