#include "inmemorydatabase.h"

#include <cassert>
#include <vector>

int main()
{
    InMemoryDatabase db;

    assert(db.listNewsgroups().empty());

    assert(db.createNewsgroup("sports") == DbResult::ok);
    assert(db.createNewsgroup("music") == DbResult::ok);
    assert(db.createNewsgroup("sports") == DbResult::group_already_exists);

    std::vector<NewsgroupSummary> groups = db.listNewsgroups();
    assert(groups.size() == 2);
    assert(groups[0].id == 1);
    assert(groups[0].name == "sports");
    assert(groups[1].id == 2);
    assert(groups[1].name == "music");

    assert(db.deleteNewsgroup(999) == DbResult::group_not_found);

    assert(db.createArticle(1, "hello", "alice", "first post") == DbResult::ok);
    assert(db.createArticle(1, "update", "bob", "second post") == DbResult::ok);
    assert(db.createArticle(999, "x", "y", "z") == DbResult::group_not_found);

    ListArticlesResult listResult = db.listArticles(1);
    assert(listResult.status == DbResult::ok);
    assert(listResult.articles.size() == 2);
    assert(listResult.articles[0].id == 1);
    assert(listResult.articles[0].title == "hello");
    assert(listResult.articles[1].id == 2);
    assert(listResult.articles[1].title == "update");

    assert(db.listArticles(999).status == DbResult::group_not_found);

    GetArticleResult articleResult = db.getArticle(1, 1);
    assert(articleResult.status == DbResult::ok);
    assert(articleResult.article.id == 1);
    assert(articleResult.article.title == "hello");
    assert(articleResult.article.author == "alice");
    assert(articleResult.article.text == "first post");

    assert(db.getArticle(1, 999).status == DbResult::article_not_found);
    assert(db.getArticle(999, 1).status == DbResult::group_not_found);

    assert(db.deleteArticle(1, 1) == DbResult::ok);
    assert(db.deleteArticle(1, 1) == DbResult::article_not_found);

    assert(db.createArticle(1, "third", "carol", "after delete") == DbResult::ok);
    ListArticlesResult listAfterDelete = db.listArticles(1);
    assert(listAfterDelete.status == DbResult::ok);
    assert(listAfterDelete.articles.size() == 2);
    assert(listAfterDelete.articles[0].id == 2);
    assert(listAfterDelete.articles[1].id == 3);

    assert(db.deleteNewsgroup(1) == DbResult::ok);
    assert(db.deleteNewsgroup(1) == DbResult::group_not_found);

    assert(db.createNewsgroup("tech") == DbResult::ok);
    groups = db.listNewsgroups();
    assert(groups.size() == 2);
    assert(groups[0].id == 2);
    assert(groups[0].name == "music");
    assert(groups[1].id == 3);
    assert(groups[1].name == "tech");

    return 0;
}
