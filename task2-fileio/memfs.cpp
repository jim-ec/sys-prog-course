#include <iostream>
#include <cstring>
#include <vector>
#include <string_view>
#include <map>
#include <set>
#include <cstdint>

#define FUSE_USE_VERSION 26
#define _FILE_OFFSET_BITS 64
#include <fuse.h>


struct File {
    std::vector<char> contents;
};


static std::set<std::string> directories;
static std::map<std::string, File> files;


namespace impl {


static int getattr(const char *path, struct stat *stat)
{
    *stat = {};

    if (files.count(path) > 0) {
        auto &file = files[path];
        stat->st_mode = S_IFREG | 0777;
        stat->st_size = file.contents.size();
        return 0;
    }
    if (directories.count(path) > 0) {
        stat->st_mode = S_IFDIR | 0755;
        return 0;
    }
    
    return -ENOENT;
}


static int readdir(const char *path, void *buffer, fuse_fill_dir_t filler, off_t, struct fuse_file_info *)
{
    filler(buffer, ".", NULL, 0);
    filler(buffer, "..", NULL, 0);

    if (directories.count(path) == 0)
    {
        return -ENOENT;
    }

    std::string_view queryPath = path;
    if (queryPath.back() == '/') {
        queryPath.remove_suffix(1);
    }

    for (auto& fullDirectoryPath : directories)
    {
        if (fullDirectoryPath == "/")
        {
            continue;
        }
        if (fullDirectoryPath.length() <= queryPath.length())
        {
            continue;
        }
        if (0 != strncmp(fullDirectoryPath.c_str(), queryPath.data(), queryPath.length()))
        {
            continue;
        }
        if (fullDirectoryPath.find('/', queryPath.length() + 1) != std::string::npos)
        {
            continue;
        }

        filler(buffer, &fullDirectoryPath.c_str()[queryPath.length() + 1], NULL, 0);
    }
    for (auto& [fullFilePath, _] : files)
    {
        if (fullFilePath.length() <= queryPath.length())
        {
            continue;
        }
        if (0 != strncmp(fullFilePath.c_str(), queryPath.data(), queryPath.length()))
        {
            continue;
        }
        if (fullFilePath.find('/', queryPath.length() + 1) != std::string::npos)
        {
            continue;
        }

        filler(buffer, &fullFilePath.c_str()[queryPath.length() + 1], NULL, 0);
    }

    return 0;
}


static int open(const char *, struct fuse_file_info *)
{
    return 0;
}


static int read(const char *path, char *buffer, size_t size, off_t offset, struct fuse_file_info *)
{
    if (files.count(path) == 0) {
        return -ENOENT;
    }
    
    auto &file = files[path];
    if (offset >= static_cast<off_t>(file.contents.size())) {
        return 0;
    }
    if (offset + size > file.contents.size()) {
        memcpy(buffer, &file.contents[offset], file.contents.size() - offset);
        return file.contents.size() - offset;
    }
    memcpy(buffer, &file.contents[offset], size);
    return size;
}


static int write(const char *path, const char *buffer, size_t size, off_t offset, struct fuse_file_info *)
{
    if (files.count(path) == 0) {
         return -ENOENT;
    }

    auto &file = files[path];
    file.contents.resize(offset + size);
    memcpy(file.contents.data() + offset, buffer, size);

    return size;
}


static int create(const char *path, mode_t, struct fuse_file_info *)
{
    files[path] = File();
    return 0;
}


static int mkdir(const char *path, mode_t)
{
    directories.emplace(path);
    return 0;
}


static int utimens(const char *, const struct timespec[2])
{
    return 0;
}


}


int main(int argc, char *argv[])
{
    directories.emplace("/");
    
    if (argc != 2)
    {
        std::cerr << argv[0] << " [mount point]" << std::endl;
        exit(EXIT_FAILURE);
    }

    char *fuse_argv[] = { argv[0], const_cast<char *>("-s"), const_cast<char *>("-f"), argv[1], NULL };
    argc = 4;

    struct fuse_operations impls = {};
    impls.getattr = impl::getattr;
    impls.mkdir = impl::mkdir;
    impls.open = impl::open;
    impls.read = impl::read;
    impls.write = impl::write;
    impls.readdir = impl::readdir;
    impls.create = impl::create;
    impls.utimens = impl::utimens;
    return fuse_main(argc, fuse_argv, &impls, nullptr);
}
