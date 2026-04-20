#include "diskdatabase.h"

#include <algorithm>
#include <cerrno>
#include <cctype>
#include <dirent.h>
#include <fstream>
#include <iterator>
#include <sstream>
#include <stdexcept>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

namespace {
std::string joinPath(const std::string& left, const std::string& right)
{
    if (left.empty()) {
        return right;
    }
    if (left.back() == '/') {
        return left + right;
    }
    return left + "/" + right;
}

bool isDirectory(const std::string& path)
{
    struct stat pathStat;
    if (stat(path.c_str(), &pathStat) != 0) {
        return false;
    }
    return S_ISDIR(pathStat.st_mode);
}

bool isRegularFile(const std::string& path)
{
    struct stat pathStat;
    if (stat(path.c_str(), &pathStat) != 0) {
        return false;
    }
    return S_ISREG(pathStat.st_mode);
}

bool isDigitsOnly(const std::string& text)
{
    if (text.empty()) {
        return false;
    }
    return std::all_of(text.begin(), text.end(),
                       [](unsigned char ch) { return std::isdigit(ch) != 0; });
}
}

DiskDatabase::DiskDatabase(const std::string& rootPathIn)
    : rootPath(rootPathIn)
{
    ensureStorage();
}

std::vector<NewsgroupSummary> DiskDatabase::listNewsgroups() const
{
    std::vector<NewsgroupSummary> result;
    std::vector<int> ids = listExistingNewsgroupIds();
    result.reserve(ids.size());

    for (int id : ids) {
        NewsgroupSummary summary;
        summary.id = id;
        summary.name = readGroupName(id);
        result.push_back(summary);
    }

    return result;
}

DbResult DiskDatabase::createNewsgroup(const std::string& name)
{
    if (groupNameExists(name)) {
        return DbResult::group_already_exists;
    }

    int newsgroupId = readNextNewsgroupId();
    createDirectory(groupPath(newsgroupId));
    createDirectory(groupArticlesPath(newsgroupId));
    writeFile(groupNamePath(newsgroupId), name);
    writeNextArticleId(newsgroupId, 1);
    writeNextNewsgroupId(newsgroupId + 1);

    return DbResult::ok;
}

DbResult DiskDatabase::deleteNewsgroup(int newsgroupId)
{
    if (!newsgroupExists(newsgroupId)) {
        return DbResult::group_not_found;
    }

    removeDirectoryRecursively(groupPath(newsgroupId));
    return DbResult::ok;
}

ListArticlesResult DiskDatabase::listArticles(int newsgroupId) const
{
    ListArticlesResult result;
    if (!newsgroupExists(newsgroupId)) {
        result.status = DbResult::group_not_found;
        return result;
    }

    std::vector<int> ids = listExistingArticleIds(newsgroupId);
    result.articles.reserve(ids.size());
    for (int id : ids) {
        ArticleSummary summary;
        summary.id = id;
        summary.title = readFile(articleTitlePath(newsgroupId, id));
        result.articles.push_back(summary);
    }

    return result;
}

DbResult DiskDatabase::createArticle(int newsgroupId,
                                     const std::string& title,
                                     const std::string& author,
                                     const std::string& text)
{
    if (!newsgroupExists(newsgroupId)) {
        return DbResult::group_not_found;
    }

    int articleId = readNextArticleId(newsgroupId);
    createDirectory(articlePath(newsgroupId, articleId));
    writeFile(articleTitlePath(newsgroupId, articleId), title);
    writeFile(articleAuthorPath(newsgroupId, articleId), author);
    writeFile(articleTextPath(newsgroupId, articleId), text);
    writeNextArticleId(newsgroupId, articleId + 1);

    return DbResult::ok;
}

DbResult DiskDatabase::deleteArticle(int newsgroupId, int articleId)
{
    if (!newsgroupExists(newsgroupId)) {
        return DbResult::group_not_found;
    }
    if (!articleExists(newsgroupId, articleId)) {
        return DbResult::article_not_found;
    }

    removeDirectoryRecursively(articlePath(newsgroupId, articleId));
    return DbResult::ok;
}

GetArticleResult DiskDatabase::getArticle(int newsgroupId, int articleId) const
{
    GetArticleResult result;
    if (!newsgroupExists(newsgroupId)) {
        result.status = DbResult::group_not_found;
        return result;
    }
    if (!articleExists(newsgroupId, articleId)) {
        result.status = DbResult::article_not_found;
        return result;
    }

    Article article;
    article.id = articleId;
    article.title = readFile(articleTitlePath(newsgroupId, articleId));
    article.author = readFile(articleAuthorPath(newsgroupId, articleId));
    article.text = readFile(articleTextPath(newsgroupId, articleId));
    result.article = article;
    return result;
}

void DiskDatabase::ensureStorage() const
{
    createDirectory(rootPath);
    createDirectory(groupsRootPath());

    if (!isRegularFile(nextNewsgroupIdPath())) {
        writeNextNewsgroupId(1);
    }
}

std::string DiskDatabase::groupsRootPath() const
{
    return joinPath(rootPath, "groups");
}

std::string DiskDatabase::nextNewsgroupIdPath() const
{
    return joinPath(rootPath, "next_newsgroup_id.txt");
}

std::string DiskDatabase::groupPath(int newsgroupId) const
{
    return joinPath(groupsRootPath(), std::to_string(newsgroupId));
}

std::string DiskDatabase::groupNamePath(int newsgroupId) const
{
    return joinPath(groupPath(newsgroupId), "name.txt");
}

std::string DiskDatabase::groupNextArticleIdPath(int newsgroupId) const
{
    return joinPath(groupPath(newsgroupId), "next_article_id.txt");
}

std::string DiskDatabase::groupArticlesPath(int newsgroupId) const
{
    return joinPath(groupPath(newsgroupId), "articles");
}

std::string DiskDatabase::articlePath(int newsgroupId, int articleId) const
{
    return joinPath(groupArticlesPath(newsgroupId), std::to_string(articleId));
}

std::string DiskDatabase::articleTitlePath(int newsgroupId, int articleId) const
{
    return joinPath(articlePath(newsgroupId, articleId), "title.txt");
}

std::string DiskDatabase::articleAuthorPath(int newsgroupId, int articleId) const
{
    return joinPath(articlePath(newsgroupId, articleId), "author.txt");
}

std::string DiskDatabase::articleTextPath(int newsgroupId, int articleId) const
{
    return joinPath(articlePath(newsgroupId, articleId), "text.txt");
}

int DiskDatabase::readNextNewsgroupId() const
{
    return readIntegerFile(nextNewsgroupIdPath());
}

void DiskDatabase::writeNextNewsgroupId(int nextId) const
{
    writeIntegerFile(nextNewsgroupIdPath(), nextId);
}

int DiskDatabase::readNextArticleId(int newsgroupId) const
{
    return readIntegerFile(groupNextArticleIdPath(newsgroupId));
}

void DiskDatabase::writeNextArticleId(int newsgroupId, int nextId) const
{
    writeIntegerFile(groupNextArticleIdPath(newsgroupId), nextId);
}

std::vector<int> DiskDatabase::listExistingNewsgroupIds() const
{
    std::vector<int> ids;
    DIR* groupDir = opendir(groupsRootPath().c_str());
    if (groupDir == nullptr) {
        throw std::runtime_error("Failed to open groups directory");
    }

    while (dirent* entry = readdir(groupDir)) {
        std::string entryName = entry->d_name;
        if (entryName == "." || entryName == ".." || !isDigitsOnly(entryName)) {
            continue;
        }

        int id = std::stoi(entryName);
        if (newsgroupExists(id)) {
            ids.push_back(id);
        }
    }

    closedir(groupDir);
    std::sort(ids.begin(), ids.end());
    return ids;
}

std::vector<int> DiskDatabase::listExistingArticleIds(int newsgroupId) const
{
    std::vector<int> ids;
    DIR* articleDir = opendir(groupArticlesPath(newsgroupId).c_str());
    if (articleDir == nullptr) {
        throw std::runtime_error("Failed to open articles directory");
    }

    while (dirent* entry = readdir(articleDir)) {
        std::string entryName = entry->d_name;
        if (entryName == "." || entryName == ".." || !isDigitsOnly(entryName)) {
            continue;
        }

        int id = std::stoi(entryName);
        if (articleExists(newsgroupId, id)) {
            ids.push_back(id);
        }
    }

    closedir(articleDir);
    std::sort(ids.begin(), ids.end());
    return ids;
}

bool DiskDatabase::newsgroupExists(int newsgroupId) const
{
    return isDirectory(groupPath(newsgroupId));
}

bool DiskDatabase::articleExists(int newsgroupId, int articleId) const
{
    return isDirectory(articlePath(newsgroupId, articleId));
}

bool DiskDatabase::groupNameExists(const std::string& name) const
{
    std::vector<int> ids = listExistingNewsgroupIds();
    for (int id : ids) {
        if (readGroupName(id) == name) {
            return true;
        }
    }
    return false;
}

std::string DiskDatabase::readGroupName(int newsgroupId) const
{
    return readFile(groupNamePath(newsgroupId));
}

void DiskDatabase::createDirectory(const std::string& path) const
{
    if (mkdir(path.c_str(), 0777) != 0) {
        if (errno == EEXIST && isDirectory(path)) {
            return;
        }
        throw std::runtime_error("Failed to create directory: " + path);
    }
}

void DiskDatabase::removeDirectoryRecursively(const std::string& path) const
{
    DIR* directory = opendir(path.c_str());
    if (directory == nullptr) {
        throw std::runtime_error("Failed to open directory for removal: " + path);
    }

    while (dirent* entry = readdir(directory)) {
        std::string entryName = entry->d_name;
        if (entryName == "." || entryName == "..") {
            continue;
        }

        std::string childPath = joinPath(path, entryName);
        if (isDirectory(childPath)) {
            removeDirectoryRecursively(childPath);
        } else if (unlink(childPath.c_str()) != 0) {
            closedir(directory);
            throw std::runtime_error("Failed to remove file: " + childPath);
        }
    }

    closedir(directory);
    if (rmdir(path.c_str()) != 0) {
        throw std::runtime_error("Failed to remove directory: " + path);
    }
}

std::string DiskDatabase::readFile(const std::string& path) const
{
    std::ifstream input(path.c_str(), std::ios::binary);
    if (!input) {
        throw std::runtime_error("Failed to read file: " + path);
    }

    return std::string(std::istreambuf_iterator<char>(input),
                       std::istreambuf_iterator<char>());
}

void DiskDatabase::writeFile(const std::string& path, const std::string& contents) const
{
    std::ofstream output(path.c_str(), std::ios::binary | std::ios::trunc);
    if (!output) {
        throw std::runtime_error("Failed to write file: " + path);
    }

    output.write(contents.data(), static_cast<std::streamsize>(contents.size()));
    if (!output) {
        throw std::runtime_error("Failed while writing file: " + path);
    }
}

int DiskDatabase::readIntegerFile(const std::string& path) const
{
    std::ifstream input(path.c_str());
    if (!input) {
        throw std::runtime_error("Failed to read integer file: " + path);
    }

    int value = 0;
    input >> value;
    if (!input) {
        throw std::runtime_error("Malformed integer file: " + path);
    }
    return value;
}

void DiskDatabase::writeIntegerFile(const std::string& path, int value) const
{
    std::ofstream output(path.c_str(), std::ios::trunc);
    if (!output) {
        throw std::runtime_error("Failed to write integer file: " + path);
    }

    output << value;
    if (!output) {
        throw std::runtime_error("Failed while writing integer file: " + path);
    }
}
