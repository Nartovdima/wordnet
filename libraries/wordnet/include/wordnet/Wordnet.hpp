#ifndef WORDNET_WORDNET_HPP
#define WORDNET_WORDNET_HPP

#include <iosfwd>
#include <set>
#include <string>

class WordNet {
public:
    WordNet(std::istream& synsets, std::istream& hypernyms);

    /**
     * Simple proxy class used to enumerate nouns.
     *
     * Usage example:
     *
     * WordNet wordnet{...};
     * ...
     * for (const std::string & noun : wordnet.nouns()) {
     *     // ...
     * }
     */
    class Nouns {
    public:
        class iterator {
        public:
            using iterator_category = std::forward_iterator_tag;

            // To do
        };

        iterator begin() const;
        iterator end() const;
    };

    // lists all nouns stored in WordNet
    Nouns nouns() const;

    // returns 'true' iff 'word' is stored in WordNet
    bool is_noun(const std::string& word) const;

    // returns gloss of "shortest common ancestor" of noun1 and noun2
    std::string sca(const std::string& noun1, const std::string& noun2) const;

    // calculates distance between noun1 and noun2
    unsigned distance(const std::string& noun1, const std::string& noun2) const;
};

class Outcast {
public:
    explicit Outcast(WordNet& wordnet);

    // returns outcast word
    std::string outcast(const std::set<std::string>& nouns);
};

class Digraph {
    // Please implement...
};

class ShortestCommonAncestor {
    explicit ShortestCommonAncestor(Digraph& dg);

    // calculates length of shortest common ancestor path from node with id 'v' to
    // node with id 'w'
    unsigned length(unsigned v, unsigned w);

    // returns node id of shortest common ancestor of nodes v and w
    unsigned ancestor(unsigned v, unsigned w);

    // calculates length of shortest common ancestor path from node subset
    // 'subset_a' to node subset 'subset_b'
    unsigned length_subset(const std::set<unsigned>& subset_a, const std::set<unsigned>& subset_b);

    // returns node id of shortest common ancestor of node subset 'subset_a' and
    // node subset 'subset_b'
    unsigned ancestor_subset(const std::set<unsigned>& subset_a, const std::set<unsigned>& subset_b);
};

#endif  // WORDNET_WORDNET_HPP
