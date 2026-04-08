#include "inmemorydatabase.h"

#include <algorithm>

std::vector<NewsgroupSummary> InMemoryDatabase::listNewsgroups() const
{
    std::vector<NewsgroupSummary> result;
    result.reserve(groups.size());

    for (const auto& group : groups) {
        NewsgroupSummary summary;
        summary.id = group.id;
        summary.name = group.name;
        result.push_back(summary);
    }

    return result;
}

DbResult InMemoryDatabase::createNewsgroup(const std::string& name)
{
    if (findGroupByName(name) != groups.end()) {
        return DbResult::group_already_exists;
    }

    StoredNewsgroup group;
    group.id = nextNewsgroupId++;
    group.name = name;
    groups.push_back(group);

    return DbResult::ok;
}

DbResult InMemoryDatabase::deleteNewsgroup(int newsgroupId)
{
    auto it = findGroup(newsgroupId);
    if (it == groups.end()) {
        return DbResult::group_not_found;
    }

    groups.erase(it);
    return DbResult::ok;
}

ListArticlesResult InMemoryDatabase::listArticles(int newsgroupId) const
{
    ListArticlesResult result;
    auto groupIt = findGroup(newsgroupId);

    if (groupIt == groups.end()) {
        result.status = DbResult::group_not_found;
        return result;
    }

    result.articles.reserve(groupIt->articles.size());
    for (const auto& article : groupIt->articles) {
        ArticleSummary summary;
        summary.id = article.id;
        summary.title = article.title;
        result.articles.push_back(summary);
    }

    return result;
}

DbResult InMemoryDatabase::createArticle(int newsgroupId,
                                         const std::string& title,
                                         const std::string& author,
                                         const std::string& text)
{
    auto groupIt = findGroup(newsgroupId);
    if (groupIt == groups.end()) {
        return DbResult::group_not_found;
    }

    Article article;
    article.id = groupIt->nextArticleId++;
    article.title = title;
    article.author = author;
    article.text = text;

    groupIt->articles.push_back(article);
    return DbResult::ok;
}

DbResult InMemoryDatabase::deleteArticle(int newsgroupId, int articleId)
{
    auto groupIt = findGroup(newsgroupId);
    if (groupIt == groups.end()) {
        return DbResult::group_not_found;
    }

    auto articleIt = findArticle(*groupIt, articleId);
    if (articleIt == groupIt->articles.end()) {
        return DbResult::article_not_found;
    }

    groupIt->articles.erase(articleIt);
    return DbResult::ok;
}

GetArticleResult InMemoryDatabase::getArticle(int newsgroupId, int articleId) const
{
    GetArticleResult result;
    auto groupIt = findGroup(newsgroupId);

    if (groupIt == groups.end()) {
        result.status = DbResult::group_not_found;
        return result;
    }

    auto articleIt = findArticle(*groupIt, articleId);
    if (articleIt == groupIt->articles.end()) {
        result.status = DbResult::article_not_found;
        return result;
    }

    result.article = *articleIt;
    return result;
}

std::vector<InMemoryDatabase::StoredNewsgroup>::iterator
InMemoryDatabase::findGroup(int newsgroupId)
{
    return std::find_if(groups.begin(), groups.end(),
                        [newsgroupId](const StoredNewsgroup& group) {
                            return group.id == newsgroupId;
                        });
}

std::vector<InMemoryDatabase::StoredNewsgroup>::const_iterator
InMemoryDatabase::findGroup(int newsgroupId) const
{
    return std::find_if(groups.begin(), groups.end(),
                        [newsgroupId](const StoredNewsgroup& group) {
                            return group.id == newsgroupId;
                        });
}

std::vector<InMemoryDatabase::StoredNewsgroup>::iterator
InMemoryDatabase::findGroupByName(const std::string& name)
{
    return std::find_if(groups.begin(), groups.end(),
                        [&name](const StoredNewsgroup& group) {
                            return group.name == name;
                        });
}

std::vector<InMemoryDatabase::StoredNewsgroup>::const_iterator
InMemoryDatabase::findGroupByName(const std::string& name) const
{
    return std::find_if(groups.begin(), groups.end(),
                        [&name](const StoredNewsgroup& group) {
                            return group.name == name;
                        });
}

std::vector<Article>::iterator
InMemoryDatabase::findArticle(StoredNewsgroup& group, int articleId)
{
    return std::find_if(group.articles.begin(), group.articles.end(),
                        [articleId](const Article& article) {
                            return article.id == articleId;
                        });
}

std::vector<Article>::const_iterator
InMemoryDatabase::findArticle(const StoredNewsgroup& group, int articleId) const
{
    return std::find_if(group.articles.begin(), group.articles.end(),
                        [articleId](const Article& article) {
                            return article.id == articleId;
                        });
}
