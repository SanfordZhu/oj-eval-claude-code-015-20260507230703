#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <sstream>
#include <cstring>
#include <map>
#include <set>

using namespace std;

class FileStorage {
private:
    string filename;

    struct IndexEntry {
        char key[65];  // 64 bytes + null terminator
        long dataOffset;  // Offset to the data in the file
        int dataSize;     // Size of data in bytes

        IndexEntry() {
            memset(key, 0, sizeof(key));
            dataOffset = 0;
            dataSize = 0;
        }
    };

public:
    FileStorage() {
        filename = "storage.db";
    }

    void insert(const string& key, int value) {
        // Read current index
        vector<IndexEntry> index;
        map<string, set<int>> tempData;  // Temporary storage for all data

        ifstream infile(filename, ios::binary);
        if (infile.good()) {
            // Read index
            int indexCount;
            infile.read(reinterpret_cast<char*>(&indexCount), sizeof(indexCount));
            index.resize(indexCount);
            infile.read(reinterpret_cast<char*>(index.data()), indexCount * sizeof(IndexEntry));

            // Read all data
            for (const auto& idx : index) {
                infile.seekg(idx.dataOffset);
                set<int> values;
                int count = idx.dataSize / sizeof(int);
                for (int i = 0; i < count; i++) {
                    int val;
                    infile.read(reinterpret_cast<char*>(&val), sizeof(val));
                    values.insert(val);
                }
                tempData[idx.key] = values;
            }
            infile.close();
        }

        // Add new value
        tempData[key].insert(value);

        // Write everything back
        ofstream outfile(filename, ios::binary);

        // Calculate new index
        index.clear();
        long currentOffset = sizeof(int) + tempData.size() * sizeof(IndexEntry);

        for (const auto& [k, values] : tempData) {
            IndexEntry entry;
            strncpy(entry.key, k.c_str(), 64);
            entry.dataOffset = currentOffset;
            entry.dataSize = values.size() * sizeof(int);
            index.push_back(entry);
            currentOffset += entry.dataSize;
        }

        // Write index
        int indexCount = index.size();
        outfile.write(reinterpret_cast<char*>(&indexCount), sizeof(indexCount));
        outfile.write(reinterpret_cast<char*>(index.data()), indexCount * sizeof(IndexEntry));

        // Write data
        for (const auto& [k, values] : tempData) {
            for (int val : values) {
                outfile.write(reinterpret_cast<char*>(&val), sizeof(val));
            }
        }

        outfile.close();
    }

    void remove(const string& key, int value) {
        // Read current data
        vector<IndexEntry> index;
        map<string, set<int>> tempData;

        ifstream infile(filename, ios::binary);
        if (!infile.good()) {
            return;
        }

        // Read index
        int indexCount;
        infile.read(reinterpret_cast<char*>(&indexCount), sizeof(indexCount));
        index.resize(indexCount);
        infile.read(reinterpret_cast<char*>(index.data()), indexCount * sizeof(IndexEntry));

        // Read all data
        for (const auto& idx : index) {
            infile.seekg(idx.dataOffset);
            set<int> values;
            int count = idx.dataSize / sizeof(int);
            for (int i = 0; i < count; i++) {
                int val;
                infile.read(reinterpret_cast<char*>(&val), sizeof(val));
                values.insert(val);
            }
            tempData[idx.key] = values;
        }
        infile.close();

        // Remove value if exists
        if (tempData.find(key) != tempData.end()) {
            tempData[key].erase(value);
            if (tempData[key].empty()) {
                tempData.erase(key);
            }
        }

        // Write everything back
        ofstream outfile(filename, ios::binary);

        // Calculate new index
        index.clear();
        long currentOffset = sizeof(int) + tempData.size() * sizeof(IndexEntry);

        for (const auto& [k, values] : tempData) {
            IndexEntry entry;
            strncpy(entry.key, k.c_str(), 64);
            entry.dataOffset = currentOffset;
            entry.dataSize = values.size() * sizeof(int);
            index.push_back(entry);
            currentOffset += entry.dataSize;
        }

        // Write index
        indexCount = index.size();
        outfile.write(reinterpret_cast<char*>(&indexCount), sizeof(indexCount));
        outfile.write(reinterpret_cast<char*>(index.data()), indexCount * sizeof(IndexEntry));

        // Write data
        for (const auto& [k, values] : tempData) {
            for (int val : values) {
                outfile.write(reinterpret_cast<char*>(&val), sizeof(val));
            }
        }

        outfile.close();
    }

    vector<int> find(const string& key) {
        vector<int> result;

        ifstream infile(filename, ios::binary);
        if (!infile.good()) {
            return result;
        }

        // Read index
        int indexCount;
        infile.read(reinterpret_cast<char*>(&indexCount), sizeof(indexCount));
        vector<IndexEntry> index(indexCount);
        infile.read(reinterpret_cast<char*>(index.data()), indexCount * sizeof(IndexEntry));

        // Find the key in index
        for (const auto& idx : index) {
            if (string(idx.key) == key) {
                // Read values for this key
                infile.seekg(idx.dataOffset);
                int count = idx.dataSize / sizeof(int);
                for (int i = 0; i < count; i++) {
                    int val;
                    infile.read(reinterpret_cast<char*>(&val), sizeof(val));
                    result.push_back(val);
                }
                break;
            }
        }

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