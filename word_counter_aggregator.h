//
// Created by Ruben.Rivas on 6/6/2025.
//

#ifndef WORD_COUNTER_AGGREGATOR_H
#define WORD_COUNTER_AGGREGATOR_H
#include <condition_variable>
#include <string>
#include <vector>
#include <queue>
#include <thread>
#include <unordered_map>

// Algoritmo para combinar mapas
template <typename MapType, typename BinaryFunction>
void join_maps(MapType& target, const MapType& source, BinaryFunction fun) {
    for (const auto& [key, value] : source) {
        if (target.find(key) == target.end()) {
            target[key] = value;
        } else {
            target[key] = fun(target[key], value);
        }
    }
}

// Alias
using MapType = std::unordered_map<std::string, int>;

// Clase Principal
class WordCounterAggregator {

    // Sincronización ATRIBUTOS
    std::queue<MapType> words_queue_;
    std::mutex mutex_;
    std::condition_variable condition_;

    // Hilos
    std::vector<std::jthread> reader_threads_;
    std::jthread aggregator_thread_;

    // Variables de trabajo
    std::string path_;
    std::size_t n_files_;
    MapType map_words_;

    // Tareas
    void reader_task(const std::string& filename);
    void aggregate_task();

public:
    explicit WordCounterAggregator(std::string path);
    void run(
        const std::string &file_pattern,
        const std::string &result_filename);
};

#endif //WORD_COUNTER_AGGREGATOR_H
