#ifndef WORDNET_WORDNET_HPP
#define WORDNET_WORDNET_HPP

#include <iosfwd>
#include <istream>
#include <ostream>
#include <queue>
#include <set>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class Digraph {
public:
    Digraph() = default;
    Digraph(std::istream& hypernyms, std::unordered_map<std::size_t, std::size_t> shrinked_id);

    friend std::ostream& operator<<(std::ostream& out, const Digraph& graph);

    const std::vector<std::size_t>& operator[](std::size_t ind) const;

    std::size_t size() const;

private:
    std::vector<std::vector<std::size_t>> graph;
    std::size_t graph_size;
};

class ShortestCommonAncestor {
public:
    ShortestCommonAncestor()                                 = default;
    ShortestCommonAncestor(const ShortestCommonAncestor& dg) = default;

    explicit ShortestCommonAncestor(Digraph& dg);
    explicit ShortestCommonAncestor(Digraph dg);

    ShortestCommonAncestor& operator=(ShortestCommonAncestor&& tmp) noexcept {
        wordnet_graph = tmp.wordnet_graph;
        return *this;
    }

    // calculates length of shortest common ancestor path from node with id 'v' to
    // node with id 'w'
    std::size_t length(std::size_t v, std::size_t w) const;

    // returns node id of shortest common ancestor of nodes v and w
    std::size_t ancestor(std::size_t v, std::size_t w) const;

    // calculates length of shortest common ancestor path from node subset
    // 'subset_a' to node subset 'subset_b'
    std::size_t length_subset(const std::set<std::size_t>& subset_a, const std::set<std::size_t>& subset_b) const;

    // returns node id of shortest common ancestor of node subset 'subset_a' and
    // node subset 'subset_b'
    std::size_t ancestor_subset(const std::set<std::size_t>& subset_a, const std::set<std::size_t>& subset_b) const;

private:
    Digraph wordnet_graph;

    class LCASolution {
    public:
        std::size_t sca_id;
        std::size_t sap_distance;

        LCASolution(std::size_t sca_id, std::size_t sap_distance);
    };

    LCASolution bfs(const std::set<std::size_t>& subset_a, const std::set<std::size_t>& subset_b) const;
};

class WordNet {
    using words_to_synsets = std::unordered_map<std::string, std::set<std::size_t>>;

public:
    WordNet(std::istream& synsets, std::istream& hypernyms);

    WordNet(WordNet& wordnet) = default;

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
            using value_type        = std::string;
            using difference_type   = std::ptrdiff_t;
            using pointer           = const value_type*;
            using reference         = const value_type&;

            iterator() = default;

            [[nodiscard]] reference operator*() const { return it->first; }

            [[nodiscard]] pointer operator->() const { return std::addressof(operator*()); }

            iterator& operator++() {
                ++it;
                return *this;
            }

            iterator operator++(int) {
                auto tmp = *this;
                ++*this;
                return tmp;
            }

            iterator& operator--() {
                --it;
                return *this;
            }

            iterator operator--(int) {
                auto tmp = *this;
                --*this;
                return tmp;
            }

            friend bool operator==(const iterator& left, const iterator& right) { return left.it == right.it; }

            friend bool operator!=(const iterator& left, const iterator& right) { return !(left == right); }

            iterator(const words_to_synsets::const_iterator& it) : it(it) {}

        private:
            words_to_synsets::const_iterator it;
        };

        iterator begin() const;
        iterator end() const;

    private:
        friend class WordNet;

        Nouns(const words_to_synsets& elements) : storage(elements) {}

        const words_to_synsets& storage;
    };

    // lists all nouns stored in WordNet
    Nouns nouns() const;

    // returns 'true' iff 'word' is stored in WordNet
    bool is_noun(const std::string& word) const;

    // returns gloss of "shortest common ancestor" of noun1 and noun2
    std::string sca(const std::string& noun1, const std::string& noun2) const;

    // calculates distance between noun1 and noun2
    std::size_t distance(const std::string& noun1, const std::string& noun2) const;

private:
    words_to_synsets words_synsets;
    std::unordered_map<std::size_t, std::string> synset_gloss;
    std::unordered_map<std::size_t, std::size_t> shrinked_id;
    ShortestCommonAncestor sca_solver;
};

class Outcast {
public:
    explicit Outcast(WordNet& wordnet);

    // returns outcast word
    std::string outcast(const std::set<std::string>& nouns);

private:
    WordNet wordnet;
};

#endif  // WORDNET_WORDNET_HPP
