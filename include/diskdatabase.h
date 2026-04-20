#ifndef DISKDATABASE_H
#define DISKDATABASE_H

#include "newsdatabase.h"

#include <string>
#include <vector>

class DiskDatabase : public NewsDatabase {
public:
    explicit DiskDatabase(const std::string& rootPath);
    ~DiskDatabase() override = default;

    std::vector<NewsgroupSummary> listNewsgroups() const override;

    DbResult createNewsgroup(const std::string& name) override;
    DbResult deleteNewsgroup(int newsgroupId) override;

    ListArticlesResult listArticles(int newsgroupId) const override;
    DbResult createArticle(int newsgroupId,
                           const std::string& title,
                           const std::string& author,
                           const std::string& text) override;
    DbResult deleteArticle(int newsgroupId, int articleId) override;
    GetArticleResult getArticle(int newsgroupId, int articleId) const override;

private:
    std::string rootPath;

    void ensureStorage() const;

    std::string groupsRootPath() const;
    std::string nextNewsgroupIdPath() const;
    std::string groupPath(int newsgroupId) const;
    std::string groupNamePath(int newsgroupId) const;
    std::string groupNextArticleIdPath(int newsgroupId) const;
    std::string groupArticlesPath(int newsgroupId) const;
    std::string articlePath(int newsgroupId, int articleId) const;
    std::string articleTitlePath(int newsgroupId, int articleId) const;
    std::string articleAuthorPath(int newsgroupId, int articleId) const;
    std::string articleTextPath(int newsgroupId, int articleId) const;

    int readNextNewsgroupId() const;
    void writeNextNewsgroupId(int nextId) const;
    int readNextArticleId(int newsgroupId) const;
    void writeNextArticleId(int newsgroupId, int nextId) const;

    std::vector<int> listExistingNewsgroupIds() const;
    std::vector<int> listExistingArticleIds(int newsgroupId) const;

    bool newsgroupExists(int newsgroupId) const;
    bool articleExists(int newsgroupId, int articleId) const;
    bool groupNameExists(const std::string& name) const;

    std::string readGroupName(int newsgroupId) const;

    void createDirectory(const std::string& path) const;
    void removeDirectoryRecursively(const std::string& path) const;

    std::string readFile(const std::string& path) const;
    void writeFile(const std::string& path, const std::string& contents) const;
    int readIntegerFile(const std::string& path) const;
    void writeIntegerFile(const std::string& path, int value) const;
};

#endif
