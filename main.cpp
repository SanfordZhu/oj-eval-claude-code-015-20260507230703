#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <unordered_map>
#include <sstream>
#include <filesystem>

using namespace std;
namespace fs = filesystem;

class FileStorage {
private:
    string baseDir;

    string getFilename(const string& key) {
        return baseDir + "/" + key + ".dat";
    }

public:
    FileStorage() {
        baseDir = "storage";
        if (!fs::exists(baseDir)) {
            fs::create_directory(baseDir);
        }
    }

    void insert(const string& key, int value) {
        string filename = getFilename(key);
        vector<int> values;

        // Read existing values
        if (fs::exists(filename)) {
            ifstream infile(filename, ios::binary);
            int count;
            infile.read(reinterpret_cast<char*>(&count), sizeof(count));
            values.resize(count);
            infile.read(reinterpret_cast<char*>(values.data()), count * sizeof(int));
            infile.close();

            // Check if value already exists
            if (std::find(values.begin(), values.end(), value) != values.end()) {
                return;
            }
        }

        // Add new value
        values.push_back(value);
        sort(values.begin(), values.end());

        // Write back to file
        ofstream outfile(filename, ios::binary);
        int count = values.size();
        outfile.write(reinterpret_cast<char*>(&count), sizeof(count));
        outfile.write(reinterpret_cast<char*>(values.data()), count * sizeof(int));
        outfile.close();
    }

    void remove(const string& key, int value) {
        string filename = getFilename(key);

        if (!fs::exists(filename)) {
            return;
        }

        // Read existing values
        ifstream infile(filename, ios::binary);
        int count;
        infile.read(reinterpret_cast<char*>(&count), sizeof(count));
        vector<int> values(count);
        infile.read(reinterpret_cast<char*>(values.data()), count * sizeof(int));
        infile.close();

        // Remove value if exists
        auto it = std::find(values.begin(), values.end(), value);
        if (it != values.end()) {
            values.erase(it);

            // Write back to file
            ofstream outfile(filename, ios::binary);
            int newCount = values.size();
            outfile.write(reinterpret_cast<char*>(&newCount), sizeof(newCount));
            if (newCount > 0) {
                outfile.write(reinterpret_cast<char*>(values.data()), newCount * sizeof(int));
            }
            outfile.close();

            // Delete file if empty
            if (newCount == 0) {
                fs::remove(filename);
            }
        }
    }

    vector<int> find(const string& key) {
        string filename = getFilename(key);
        vector<int> result;

        if (!fs::exists(filename)) {
            return result;
        }

        // Read values from file
        ifstream infile(filename, ios::binary);
        int count;
        infile.read(reinterpret_cast<char*>(&count), sizeof(count));
        result.resize(count);
        infile.read(reinterpret_cast<char*>(result.data()), count * sizeof(int));
        infile.close();

        return result;
    }
};

int main() {
    ios_base::sync_with_stdio(false);
    cin.tie(nullptr);

    FileStorage storage;
    int n;
    cin >> n;
    cin.ignore();

    for (int i = 0; i < n; i++) {
        string line;
        getline(cin, line);
        istringstream iss(line);

        string cmd, key;
        iss >> cmd >> key;

        if (cmd == "insert") {
            int value;
            iss >> value;
            storage.insert(key, value);
        } else if (cmd == "delete") {
            int value;
            iss >> value;
            storage.remove(key, value);
        } else if (cmd == "find") {
            vector<int> values = storage.find(key);
            if (values.empty()) {
                cout << "null\n";
            } else {
                for (size_t j = 0; j < values.size(); j++) {
                    if (j > 0) cout << " ";
                    cout << values[j];
                }
                cout << "\n";
            }
        }
    }

    return 0;
}