#include <iostream>
#include <fstream>
#include <iterator>
#include <utility>
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
    unordered_map<string,unordered_map<string,int>> translation_matrix;
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
            translation_matrix[pos[i][j]] =  // i don't know how to make a dictionnary with two keys 
        }
    }

    return 0;
}
