#include <iostream>
#include <fstream>
#include <iterator>
#include <sstream>
#include <vector>
#include <string>
#include <unordered_map>

using namespace std;


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
    unordered_map<string, vector<string>> sentences_map;
    for(int i = 0; i < sentences.size(); i++) {
          for(int j = 0; j < sentences[i].size(); j++) {
                sentences_map[sentences[i][j]].push_back(pos[i][j]);
          }
    }

    cout << "Sentence map of : " << sentences_map.size() << " unique words." << endl;
    cout << "Fish --> verb : " << get_emission_probability(sentences_map["fish"],"verb") << ", noun :"<< get_emission_probability(sentences_map["fish"],"noun") <<  endl;

    return 0;
}
