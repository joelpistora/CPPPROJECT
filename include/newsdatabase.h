#ifndef NEWSDATABASE_H
#define NEWSDATABASE_H

#include "newsmodel.h"
#include <string>
#include <vector>

class NewsDatabase {
public:
    virtual ~NewsDatabase() {}

    virtual std::vector<NewsgroupSummary> listNewsgroups() const = 0;

    virtual DbResult createNewsgroup(const std::string& name) = 0;
    virtual DbResult deleteNewsgroup(int newsgroupId) = 0;

    virtual ListArticlesResult listArticles(int newsgroupId) const = 0;
    virtual DbResult createArticle(int newsgroupId,
                                   const std::string& title,
                                   const std::string& author,
                                   const std::string& text) = 0;
    virtual DbResult deleteArticle(int newsgroupId, int articleId) = 0;
    virtual GetArticleResult getArticle(int newsgroupId, int articleId) const = 0;
};

#endif
