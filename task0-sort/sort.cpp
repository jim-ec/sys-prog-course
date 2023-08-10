#include <iostream>
#include <chrono>
#include <future>
#include <vector>
#include <array>
#include <string_view>
#include <fstream>
#include <algorithm>
#include <memory>
#include <cstring>


using namespace std;


constexpr size_t mebibytes(size_t bytes) {
    return bytes * 1048576;
}


const size_t BLOCK_SIZE = mebibytes(12);


int millis() {
    auto now = chrono::system_clock::now().time_since_epoch();
    auto millis = chrono::duration_cast<chrono::milliseconds>(now);
    return millis.count();
}


namespace details {

void print() {
}
template<class T>
void print(T t) {
    clog << t;
}
template<class T, class... Ts>
void print(T t, Ts... ts) {
    clog << t;
    print(ts...);
}

static std::mutex mutex;
}

template<class... Ts>
void print(Ts... ts) {
#if DEBUG
    details::mutex.lock();
    details::print(ts...);
    clog << endl << flush;
    details::mutex.unlock();
#else
    (static_cast<void>(ts) , ...);
#endif
}


struct Block {
    string line;
    fstream file;
};


bool ascendingSort = true;


#if DEBUG
    std::ifstream in("sort-input");
    std::ofstream out("sort-output");
#else
    std::istream &in = cin;
    std::ostream &out = cout;
#endif


bool lexi(string_view a, string_view b) {
    return ascendingSort ? a.compare(b) < 0 : a.compare(b) > 0;
}


unique_ptr<Block> sortedBlock(unique_ptr<vector<string>> lines) {
    size_t t0 = millis();
    std::sort(lines->begin(), lines->end(), lexi);
    size_t t1 = millis();
    print("Sorted, took ", t1 - t0, " millis");

    std::string path = tmpnam(nullptr);
    std::fstream stream = std::fstream(path, std::ios::in | std::ios::out | std::ios::trunc);
    for (size_t i = 1; i < lines->size(); ++i) {
        stream.write((*lines)[i].data(), (*lines)[i].length());
        stream.put('\n');
    }
    stream.seekg(std::ios::beg);
    size_t t2 = millis();
    print("Wrote ", lines->size(), " lines, took ", t2 - t1, " millis");

    return make_unique<Block>(Block{std::move((*lines)[0]), std::move(stream)});
};


vector<unique_ptr<Block>> sortedBlocks() {
    future<unique_ptr<Block>> sorter;
    vector<unique_ptr<Block>> blocks;
    
    {
        int t0 = millis();
        unique_ptr<vector<string>> fillingLines(new vector<string>);
        size_t fillingBlockSize = 0;
        
        while (in.good()) {
            string line;
            getline(in, line);
            if (in.good() || !line.empty()) {
                fillingBlockSize += line.length();
                fillingLines->emplace_back(std::move(line));
            }
            if (fillingBlockSize >= BLOCK_SIZE) {
                int t1 = millis();
                print("Read ", fillingLines->size(), " lines in ", t1 - t0, " millis");
                t0 = t1;
                if (sorter.valid()) {
                    sorter.wait();
                    blocks.emplace_back(std::move(sorter.get()));
                }
                sorter = async(launch::async, [lines = std::move(fillingLines)] () mutable {
                    return sortedBlock(std::move(lines));
                });
                fillingLines = make_unique<vector<string>>();
                fillingBlockSize = 0;
            }
        }

        if (!fillingLines->empty()) {
            blocks.emplace_back(sortedBlock(std::move(fillingLines)));
        }
    }
    
    if (sorter.valid()) {
        sorter.wait();
        blocks.emplace_back(std::move(sorter.get()));
    }

    print("Generated ", blocks.size(), " blocks");
    return blocks;
}


void mergeBlocks(vector<unique_ptr<Block>> &&blocks) {
    while (!blocks.empty()) {
        size_t iMin = 0;
        for (size_t i = 1; i < blocks.size(); ++i) {
            if (lexi(blocks[i]->line, blocks[iMin]->line)) {
                iMin = i;
            }
        }
        Block &block = *blocks[iMin];

        out << block.line << '\n';
        getline(block.file, block.line);

        if (!block.file.good()) {
            blocks.erase(blocks.begin() + iMin);
        }
    }
}


int main(int argc, char **argv)
{
    if (argc > 1) {
        if (!strcmp(argv[1], "-r")) {
            ascendingSort = false;
        }
    }

    int t0 = millis();
    
    auto blocks = sortedBlocks();
    mergeBlocks(std::move(blocks));
    
    int t1 = millis();
    print("Finish, took ", t1 - t0, " millis");
}
