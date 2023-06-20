#include "wordnet/Wordnet.hpp"

Digraph::Digraph(std::istream& hypernyms, std::unordered_map<std::size_t, std::size_t> shrinked_id) {
    std::size_t child_id;
    graph.resize(shrinked_id.size());
    graph_size = shrinked_id.size();
    while (hypernyms >> child_id) {
        child_id = shrinked_id[child_id];
        while (hypernyms.peek() != '\n') {
            hypernyms.ignore();
            std::size_t parent_id;
            hypernyms >> parent_id;
            parent_id = shrinked_id[parent_id];
            graph[child_id].push_back(parent_id);
        }
    }
}

const std::vector<std::size_t>& Digraph::operator[](std::size_t ind) const {
    return graph[ind];
}

std::size_t Digraph::size() const {
    return graph_size;
}

std::ostream& operator<<(std::ostream& out, const Digraph& graph) {
    for (std::size_t i = 0; i < graph.size(); i++) {
        out << i << ": ";
        for (auto& j : graph.graph[i]) {
            out << j << ", ";
        }
        out << std::endl;
    }
    return out;
}

WordNet::WordNet(std::istream& synsets, std::istream& hypernyms) {
    std::size_t id;
    std::size_t new_id = 0;
    while (synsets >> id) {
        shrinked_id[id] = new_id;
        id              = new_id++;
        synsets.ignore();
        std::string synset;
        getline(synsets, synset, ',');
        std::stringstream words(synset);
        std::string word;
        while (words >> word) {
            words_synsets[word].insert(id);
        }
        getline(synsets, synset_gloss[id]);
    }

    sca_solver = ShortestCommonAncestor(Digraph(hypernyms, shrinked_id));
}

WordNet::Nouns::iterator WordNet::Nouns::begin() const {
    return iterator(storage.begin());
}

WordNet::Nouns::iterator WordNet::Nouns::end() const {
    return iterator(storage.end());
}

WordNet::Nouns WordNet::nouns() const {
    return Nouns(words_synsets);
}

bool WordNet::is_noun(const std::string& word) const {
    return words_synsets.contains(word);
}

std::string WordNet::sca(const std::string& noun1, const std::string& noun2) const {
    return synset_gloss.at((sca_solver.ancestor_subset(words_synsets.at(noun1), words_synsets.at(noun2))));
}

std::size_t WordNet::distance(const std::string& noun1, const std::string& noun2) const {
    return sca_solver.length_subset(words_synsets.at(noun1), words_synsets.at(noun2));
}

Outcast::Outcast(WordNet& wordnet) : wordnet(wordnet) {}

std::string Outcast::outcast(const std::set<std::string>& nouns) {
    if (nouns.size() == 1) {
        return std::string();
    }
    std::vector<std::size_t> distances(nouns.size(), 0);
    std::vector<std::string> nouns_copy(nouns.size());
    std::size_t noun_ind = 0;

    std::copy(nouns.begin(), nouns.end(), nouns_copy.begin());

    for (const auto& it1 : nouns) {
        for (const auto& it2 : nouns) {
            distances[noun_ind] += wordnet.distance(it1, it2);
        }
        noun_ind++;
    }

    std::size_t max_ind   = 0;
    bool answer_existance = true;

    for (std::size_t i = 1; i < distances.size(); i++) {
        if (distances[i] > distances[max_ind]) {
            max_ind = i;
        }
    }

    for (std::size_t i = 0; i < distances.size(); i++) {
        if (i != max_ind && distances[i] == distances[max_ind]) {
            answer_existance = false;
            break;
        }
    }

    return (answer_existance ? nouns_copy[max_ind] : std::string());
}

ShortestCommonAncestor::ShortestCommonAncestor(Digraph& dg) : wordnet_graph(dg) {}

ShortestCommonAncestor::ShortestCommonAncestor(Digraph dg) : wordnet_graph(std::move(dg)) {}

ShortestCommonAncestor::LCASolution::LCASolution(std::size_t sca_id, std::size_t sap_distance)
    : sca_id(sca_id), sap_distance(sap_distance) {}

std::size_t ShortestCommonAncestor::length(std::size_t v, std::size_t w) const {
    return bfs({v}, {w}).sap_distance;
}

std::size_t ShortestCommonAncestor::ancestor(std::size_t v, std::size_t w) const {
    return bfs({v}, {w}).sca_id;
}

std::size_t ShortestCommonAncestor::length_subset(const std::set<std::size_t>& subset_a,
                                                  const std::set<std::size_t>& subset_b) const {
    return bfs(subset_a, subset_b).sap_distance;
}

std::size_t ShortestCommonAncestor::ancestor_subset(const std::set<std::size_t>& subset_a,
                                                    const std::set<std::size_t>& subset_b) const {
    return bfs(subset_a, subset_b).sca_id;
}

ShortestCommonAncestor::LCASolution ShortestCommonAncestor::bfs(const std::set<std::size_t>& subset_a,
                                                                const std::set<std::size_t>& subset_b) const {
    const std::size_t DEFAULT_DISTANCE = -1;
    enum class vertex_status { not_visited, visited_by_first, visited_by_second };

    std::vector<std::size_t> distances(wordnet_graph.size(), DEFAULT_DISTANCE);
    std::vector<vertex_status> visit_status(wordnet_graph.size(), vertex_status::not_visited);
    std::queue<std::size_t> bfs_queue;

    for (const auto& it : subset_a) {
        bfs_queue.push(it);
        distances[it]    = 0;
        visit_status[it] = vertex_status::visited_by_first;
    }

    for (const auto& it : subset_b) {
        bfs_queue.push(it);
        distances[it] = 0;
        if (visit_status[it] == vertex_status::visited_by_first) {
            return LCASolution(it, 0);
        }
        visit_status[it] = vertex_status::visited_by_second;
    }
    auto ans = LCASolution(0, DEFAULT_DISTANCE);  // fictive answer
    while (!bfs_queue.empty()) {
        std::size_t curr_vertex = bfs_queue.front();
        bfs_queue.pop();

        for (auto& next_vertex : wordnet_graph[curr_vertex]) {
            if (visit_status[next_vertex] == vertex_status::not_visited) {
                distances[next_vertex]    = distances[curr_vertex] + 1;
                visit_status[next_vertex] = visit_status[curr_vertex];
                bfs_queue.push(next_vertex);
            } else if (visit_status[next_vertex] != vertex_status::not_visited &&
                       visit_status[next_vertex] != visit_status[curr_vertex] &&
                       ans.sap_distance > distances[next_vertex] + distances[curr_vertex] + 1) {
                ans = LCASolution(next_vertex, distances[next_vertex] + distances[curr_vertex] + 1);
            }
        }
    }
    return ans;
}
