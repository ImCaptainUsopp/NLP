#include <cmath>
#include <iostream>
#include <fstream>
#include <iterator>
#include <utility>
#include <sstream>
#include <vector>
#include <string>
#include <unordered_map>
#include <algorithm>

using namespace std;

struct hash_pair {
    template <class T1, class T2>
    size_t operator()(const pair<T1, T2> &p) const
    {
        size_t hash1 = hash<T1>{}(p.first);
        size_t hash2 = hash<T2>{}(p.second);
        // Combine the two hash values
        return hash1 ^ (hash2 + 0x9e3779b9 + (hash1 << 6) + (hash1 >> 2));
    }
};

double get_emission_probability(pair<int, unordered_map<string, int>> word_emission, string pos_tag)
{
    return 1.0 * word_emission.second[pos_tag] / word_emission.first;
}

vector<string> viterbi(unordered_map<string, pair<int, unordered_map<string, int>>> emission_matrix,
                       unordered_map<pair<string, string>, double, hash_pair> linear_trans_prob_matrix,
                       vector<string> sentence)
{
    if (sentence.empty()) return {};
    vector<string> posSentence = {"start"};

    for (const auto &word : sentence) 
    {
        unordered_map<string, double> temp;
        for (const auto &word_tag_to_count : emission_matrix[word].second)
        {
            string state = word_tag_to_count.first;
            double max_prob = -INFINITY;

            for (const auto &prev_state : posSentence)
            {
                // Next type
                double trans_prob = linear_trans_prob_matrix[{prev_state, state}];
                double emission_prob = get_emission_probability(emission_matrix[word], state);
                double score = log(trans_prob) + log(emission_prob);

                max_prob = max(max_prob, score);
            }

            temp[state] = max_prob;
        }

        string best_pos_tag_guess = max_element(temp.begin(), temp.end(),
                                          [](const pair<string, double> &a, const pair<string, double> &b)
                                          {
                                              return a.second < b.second;
                                          })
                                  ->first;
        posSentence.push_back(best_pos_tag_guess);
    }

    posSentence.push_back("end");
    return posSentence;
}

int main()
{
    ifstream data("../data.txt");
    ifstream posdata("../pos.txt");
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
    unordered_map<string, pair<int, unordered_map<string, int>>> emission_matrix;

    #pragma omp parallel for schedule(dynamic, 10)
    for (int i = 0; i < sentences.size(); i++) {

        if (sentences[i].size() != pos[i].size()) {
            cerr << "Words and POS mismatch on line : " << i << endl;
            continue;
        }

        for (int j = 0; j < sentences[i].size(); j++)
        {
            string word = sentences[i][j];
            string pos_tag = pos[i][j];

            emission_matrix[word].second[pos_tag] += 1;
            emission_matrix[word].first += 1;
        }
    }

    cout << "Sentence map of : " << emission_matrix.size() << " unique words." << endl;
    cout << "fish --> verb : " << get_emission_probability(emission_matrix["fish"], "verb") << ", noun :" << get_emission_probability(emission_matrix["fish"], "noun") << endl;

    /*
     *  Building of the transition matrix.
     */
    unordered_map<pair<string, string>, int, hash_pair> translation_matrix;
    unordered_map<string, int> sum_of_first_word_pos;

    for (int i = 0; i < pos.size(); i++) {
        for (int j = 0; j < pos[i].size(); j++) {
            string first_word_pos;
            string second_word_pos;

            first_word_pos = j == 0 ? "start" : pos[i][j - 1];
            second_word_pos = j == pos[i].size() ? "end" : pos[i][j];

            pair<string, string> key = make_pair(first_word_pos, second_word_pos);
            translation_matrix[key] += 1;
            sum_of_first_word_pos[first_word_pos] += 1;
        }
    }

    unordered_map<pair<string, string>, double, hash_pair> linear_trans_prob_matrix;

    for (auto key_value : translation_matrix) {
        pair<string, string> key = key_value.first;
        int value = key_value.second;

        linear_trans_prob_matrix[key] = 1.0 * value / sum_of_first_word_pos[key.first];

        printf("[%8s, %8s] => %.3f \n", key.first.c_str(), key.second.c_str(), linear_trans_prob_matrix[key]);
    }
    
    cout << "Size of translation matrix : " << translation_matrix.size() << endl;
    
    
    
    /*
     *  Execution of Viterbi
     */
    vector<string> test = {"tom", "loves", "fish"};
    test = viterbi(emission_matrix, linear_trans_prob_matrix, test);
    cout << "result : " << endl;
    for (const auto &word : test)
    {
        cout << word << " ";
    }
    cout << endl;
    return 0;
}
