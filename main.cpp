#include <iostream>
#include <fstream>
#include <iterator>
#include <sstream>
#include <vector>
#include <string>
using namespace std;

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



    return 0;
}
