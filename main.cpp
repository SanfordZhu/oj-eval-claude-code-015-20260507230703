#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <unordered_map>
#include <sstream>
#include <filesystem>
#include <cstring>

using namespace std;
namespace fs = filesystem;

class FileStorage {
private:
    string baseDir;
    static const int NUM_BUCKETS = 100;  // Fixed number of buckets to limit file count

    int getBucketIndex(const string& key) {
        // Simple hash function
        unsigned int hash = 0;
        for (char c : key) {
            hash = hash * 31 + c;
        }
        return hash % NUM_BUCKETS;
    }

    string getBucketFilename(int bucketIdx) {
        return baseDir + "/bucket_" + to_string(bucketIdx) + ".dat";
    }

    struct Entry {
        char key[65];  // 64 bytes + null terminator
        int value;

        Entry() {
            memset(key, 0, sizeof(key));
            value = 0;
        }
    };

public:
    FileStorage() {
        baseDir = "storage";
        if (!fs::exists(baseDir)) {
            fs::create_directory(baseDir);
        }
    }

    void insert(const string& key, int value) {
        int bucketIdx = getBucketIndex(key);
        string filename = getBucketFilename(bucketIdx);

        vector<Entry> entries;

        // Read existing entries
        if (fs::exists(filename)) {
            ifstream infile(filename, ios::binary);
            int count;
            infile.read(reinterpret_cast<char*>(&count), sizeof(count));
            entries.resize(count);
            infile.read(reinterpret_cast<char*>(entries.data()), count * sizeof(Entry));
            infile.close();

            // Check if key-value pair already exists
            for (const auto& entry : entries) {
                if (string(entry.key) == key && entry.value == value) {
                    return;
                }
            }
        }

        // Add new entry
        Entry newEntry;
        strncpy(newEntry.key, key.c_str(), 64);
        newEntry.value = value;
        entries.push_back(newEntry);

        // Sort entries by key and value
        sort(entries.begin(), entries.end(), [](const Entry& a, const Entry& b) {
            int keyCmp = strcmp(a.key, b.key);
            if (keyCmp != 0) return keyCmp < 0;
            return a.value < b.value;
        });

        // Write back to file
        ofstream outfile(filename, ios::binary);
        int count = entries.size();
        outfile.write(reinterpret_cast<char*>(&count), sizeof(count));
        outfile.write(reinterpret_cast<char*>(entries.data()), count * sizeof(Entry));
        outfile.close();
    }

    void remove(const string& key, int value) {
        int bucketIdx = getBucketIndex(key);
        string filename = getBucketFilename(bucketIdx);

        if (!fs::exists(filename)) {
            return;
        }

        // Read existing entries
        ifstream infile(filename, ios::binary);
        int count;
        infile.read(reinterpret_cast<char*>(&count), sizeof(count));
        vector<Entry> entries(count);
        infile.read(reinterpret_cast<char*>(entries.data()), count * sizeof(Entry));
        infile.close();

        // Remove entry if exists
        bool found = false;
        for (auto it = entries.begin(); it != entries.end(); ++it) {
            if (string(it->key) == key && it->value == value) {
                entries.erase(it);
                found = true;
                break;
            }
        }

        if (found) {
            // Write back to file
            ofstream outfile(filename, ios::binary);
            int newCount = entries.size();
            outfile.write(reinterpret_cast<char*>(&newCount), sizeof(newCount));
            if (newCount > 0) {
                outfile.write(reinterpret_cast<char*>(entries.data()), newCount * sizeof(Entry));
            }
            outfile.close();

            // Delete file if empty
            if (newCount == 0) {
                fs::remove(filename);
            }
        }
    }

    vector<int> find(const string& key) {
        int bucketIdx = getBucketIndex(key);
        string filename = getBucketFilename(bucketIdx);
        vector<int> result;

        if (!fs::exists(filename)) {
            return result;
        }

        // Read entries from file
        ifstream infile(filename, ios::binary);
        int count;
        infile.read(reinterpret_cast<char*>(&count), sizeof(count));
        vector<Entry> entries(count);
        infile.read(reinterpret_cast<char*>(entries.data()), count * sizeof(Entry));
        infile.close();

        // Find all values for the given key
        for (const auto& entry : entries) {
            if (string(entry.key) == key) {
                result.push_back(entry.value);
            }
        }

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