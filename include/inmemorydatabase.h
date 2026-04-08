#ifndef INMEMORYDATABASE_H
#define INMEMORYDATABASE_H

#include "newsdatabase.h"

#include <string>
#include <vector>

class InMemoryDatabase : public NewsDatabase {
public:
    InMemoryDatabase() = default;
    ~InMemoryDatabase() override = default;

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
    struct StoredNewsgroup {
        int id = 0;
        std::string name;
        int nextArticleId = 1;
        std::vector<Article> articles;
    };

    std::vector<StoredNewsgroup> groups;
    int nextNewsgroupId = 1;

    std::vector<StoredNewsgroup>::iterator findGroup(int newsgroupId);
    std::vector<StoredNewsgroup>::const_iterator findGroup(int newsgroupId) const;

    std::vector<StoredNewsgroup>::iterator findGroupByName(const std::string& name);
    std::vector<StoredNewsgroup>::const_iterator findGroupByName(const std::string& name) const;

    std::vector<Article>::iterator findArticle(StoredNewsgroup& group, int articleId);
    std::vector<Article>::const_iterator findArticle(const StoredNewsgroup& group,
                                                     int articleId) const;
};

#endif
