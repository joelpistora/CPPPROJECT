#ifndef NEWSMODEL_H
#define NEWSMODEL_H

#include <string>
#include <vector>

enum class DbResult {
    ok,
    group_already_exists,
    group_not_found,
    article_not_found
};

struct NewsgroupSummary {
    int id = 0;
    std::string name;
};

struct ArticleSummary {
    int id = 0;
    std::string title;
};

struct Article {
    int id = 0;
    std::string title;
    std::string author;
    std::string text;
};

struct ListArticlesResult {
    DbResult status = DbResult::ok;
    std::vector<ArticleSummary> articles;
};

struct GetArticleResult {
    DbResult status = DbResult::ok;
    Article article;
};

#endif
