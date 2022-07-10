#if !defined(SYSUTILS_HPP__)
#define SYSUTILS_HPP__

#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <functional>
#include <unistd.h>
#include <dirent.h>

using std::ifstream;
using std::ofstream;
using std::ios;
using std::string;
using std::map;
using std::function;

#define IGNORE_HIDDEN_FILES 1

int exec(string command, string* stdout)
{
    char buffer[128];
    *stdout = "";

    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe) {
        std::cerr << "popen failed!" << std::endl;
        exit(-1);
    }

    while (!feof(pipe)) {
        if (fgets(buffer, 128, pipe) != NULL)
            *stdout += buffer;
    }

    int last = stdout->length() - 1;
    if (last >= 0 && stdout->at(last) == '\n')
        stdout->erase(last);

    return WEXITSTATUS(pclose(pipe));
}

string dirname(string path)
{
    int pos = path.find_last_of("/");
    return pos == -1 ? "" : path.substr(0, pos);
}

string basename(string path)
{
    return path.substr(path.find_last_of("/") + 1);
}

string fullpath(string root, string rel = "")
{
    string fullpath = "";
    string cmd;
    if (rel.length() > 0)
        cmd = "cd '" + root + "' >/dev/null 2>&1 && cd '" + rel + "' >/dev/null 2>&1 && pwd -P";
    else
        cmd = "cd '" + root + "' >/dev/null 2>&1 && pwd -P";
    if (exec(cmd, &fullpath))
        return "";
    return fullpath;
}

bool exists(string file_path)
{
    return access(file_path.c_str(), F_OK) == 0;
}

void copyFile(string src_path, string dst_path)
{
    ifstream src(src_path, ios::binary);
    ofstream dst(dst_path, ios::binary);
    if (src.is_open() && dst.is_open()) {
        dst << src.rdbuf();
        src.close();
        dst.close();
    }
}

string getFile(string file_path)
{
    string contents = "";
    string line;
    ifstream file(file_path);
    if (file.is_open()) {
        while (std::getline(file, line)) {
            contents += line + '\n';
        }
        file.close();
    }
    int last = contents.length() - 1;
    if (last >= 0 && contents[last] == '\n')
        contents.erase(last);
    return contents;
}

void putFile(string file_path, string contents)
{
    ofstream file(file_path, ios::trunc);
    if (file.is_open()) {
        file << contents;
        file.close();
    }
}

bool dirEmpty(string path) {
    DIR* dirFile = opendir(path.c_str());

    if (!dirFile)
        return false;

    struct dirent* item;
    errno = 0;

    while ((item = readdir(dirFile)) != NULL) {
        string name(item->d_name);

        // Skip navigation dirs
        if (name == "." || name == "..") continue;

        // Ignore hidden directories
        if (IGNORE_HIDDEN_FILES && (name[0] == '.')) continue;

        closedir(dirFile);
        return false;
    }

    closedir(dirFile);
    return true;
}

void subdirForEach(string path, function<void(string)> callback) {
    DIR* dirFile = opendir(path.c_str());

    if (!dirFile)
        return;

    struct dirent* item;
    errno = 0;

    while ((item = readdir(dirFile)) != NULL) {
        string name(item->d_name);

        // Skip files
        if (item->d_type != DT_DIR) continue;
        // Skip navigation dirs
        if (name == "." || name == "..") continue;

        // Ignore hidden directories
        if (IGNORE_HIDDEN_FILES && (name[0] == '.')) continue;

        // Ignore empty directories
        if (dirEmpty(path + "/" + name)) continue;

        callback(name);
    }

    closedir(dirFile);
}

#endif // SYSUTILS_HPP__
