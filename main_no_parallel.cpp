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
#include <chrono>

// const int DATA_MULTIPLIER = 1000000; // Out of memory :)
const int DATA_MULTIPLIER = 100000;

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

struct Word_emission {
    long total_count = 0;
    unordered_map<string, int> post_tag_to_count;

    double probability(string pos_tag) {
        return 1.0 * post_tag_to_count[pos_tag] / total_count;
    }
};

void read_data_files(vector<vector<string>> &sentences, vector<vector<string>> &pos);
unordered_map<string, Word_emission> build_emission_matrix(vector<vector<string>> sentences, vector<vector<string>> pos_tags);
unordered_map<pair<string, string>, double, hash_pair> build_transition_probability_matrix(vector<vector<string>> pos_tags);

vector<string> viterbi(unordered_map<string, Word_emission> emission_matrix,
                       unordered_map<pair<string, string>, double, hash_pair> linear_trans_prob_matrix,
                       vector<string> sentence);

int main()
{
    vector<vector<string>> sentences, pos_tags;

    read_data_files(sentences, pos_tags);

    auto start = chrono::high_resolution_clock::now();
    auto emission_matrix = build_emission_matrix(sentences, pos_tags);
    auto end = chrono::high_resolution_clock::now();
    cout << "Time taken for Emission matrix = " << chrono::duration_cast<chrono::milliseconds>(end - start).count() << " ms" << std::endl;


    start = chrono::high_resolution_clock::now();
    auto linear_trans_prob_matrix = build_transition_probability_matrix(pos_tags);
    end = chrono::high_resolution_clock::now();
    cout << "Time taken for Transition matrix = " << chrono::duration_cast<chrono::milliseconds>(end - start).count() << " ms" << std::endl;
    
    /*
     *  Execution of Viterbi
     */
    vector<string> test = {"tom", "loves", "fish"};
    vector<string> result = viterbi(emission_matrix, linear_trans_prob_matrix, test);
    cout << "result : ";
    for (const auto &word : result)
    {
        cout << word << " ";
    }
    cout << endl;

    return 0;
}



void read_data_files(vector<vector<string>> &sentences, vector<vector<string>> &pos) {
    ifstream data("./data.txt");
    ifstream posdata("./pos.txt");

    if (!data.is_open() || !posdata.is_open()) {
        cerr << "Error opening file" << endl;
        return;
    }

    string line;

    while (getline(data, line)) {
        if (!line.empty()) {
            istringstream iss(line);
            vector<string> words((istream_iterator<string>(iss)), istream_iterator<string>());
            for (int i = 0; i < DATA_MULTIPLIER; i++) 
                sentences.push_back(words);
        }
    }

    data.close();

    while (getline(posdata, line)) {
        if (!line.empty()) {
            vector<string> words_pos;
            stringstream ss(line);
            string word_pos;
            while (getline(ss, word_pos, ',')) {
                words_pos.push_back(word_pos);
            }
            for (int i = 0; i < DATA_MULTIPLIER; i++) 
                pos.push_back(words_pos);
        }
    }
    cout << "Number of sentences in input data: " << sentences.size() << endl;
    cout << "Number of pos tags  in input data: " << pos.size() << endl;

    posdata.close();
}

unordered_map<string, Word_emission> build_emission_matrix(vector<vector<string>> sentences, vector<vector<string>> pos_tags) {
    unordered_map<string, Word_emission> emission_matrix;

    for (int i = 0; i < sentences.size(); i++) {

        if (sentences[i].size() != pos_tags[i].size()) {
            cerr << "Words and POS mismatch on line : " << i << endl;
            continue;
        }

        for (int j = 0; j < sentences[i].size(); j++)
        {
            string word = sentences[i][j];
            string pos_tag = pos_tags[i][j];

            emission_matrix[word].total_count += 1;
            emission_matrix[word].post_tag_to_count[pos_tag] += 1;
        }
    }

    cout << "Input data has " << emission_matrix.size() << " unique words." << endl;
    // cout << "fish --> verb : " << emission_matrix["fish"].probability("verb") << ", noun :" << emission_matrix["fish"].probability("noun") << endl;

    return emission_matrix;
}

unordered_map<pair<string, string>, double, hash_pair> build_transition_probability_matrix(vector<vector<string>> pos_tags) {

    unordered_map<pair<string, string>, double, hash_pair> linear_trans_prob_matrix;

    unordered_map<pair<string, string>, int, hash_pair> transition_to_count;
    unordered_map<string, int> sum_of_first_word_pos_tag;

    for (int i = 0; i < pos_tags.size(); i++) {
        for (int j = 0; j < pos_tags[i].size(); j++) {

            string first_word_pos = j == 0 ? "start" : pos_tags[i][j - 1];
            string second_word_pos = j == pos_tags[i].size() ? "end" : pos_tags[i][j];

            pair<string, string> key = make_pair(first_word_pos, second_word_pos);
            transition_to_count[key] += 1;
            sum_of_first_word_pos_tag[first_word_pos] += 1;
        }
    }

    for (auto key_value : transition_to_count) {
        pair<string, string> key = key_value.first;
        int value = key_value.second;

        linear_trans_prob_matrix[key] = 1.0 * value / sum_of_first_word_pos_tag[key.first];

        printf("[%8s, %8s] => %.3f \n", key.first.c_str(), key.second.c_str(), linear_trans_prob_matrix[key]);
    }
    
    cout << "Size of translation matrix : " << transition_to_count.size() << endl;

    return linear_trans_prob_matrix;
}

vector<string> viterbi(unordered_map<string, Word_emission> emission_matrix,
                       unordered_map<pair<string, string>, double, hash_pair> linear_trans_prob_matrix,
                       vector<string> sentence)
{
    if (sentence.empty()) return {};
    vector<string> posSentence = {"start"};

    for (const auto &word : sentence) 
    {
        unordered_map<string, double> temp;
        for (const auto &word_tag_to_count : emission_matrix[word].post_tag_to_count)
        {
            string state = word_tag_to_count.first;
            double max_prob = -INFINITY;

            for (const auto &prev_state : posSentence)
            {
                // Next type
                double trans_prob = linear_trans_prob_matrix[{prev_state, state}];
                double emission_prob = emission_matrix[word].probability(state);
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
