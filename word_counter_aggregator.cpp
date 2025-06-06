//
// Created by Ruben.Rivas on 6/6/2025.
//

#include "word_counter_aggregator.h"
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <thread>
#include <utility>

static std::string to_lower(std::string str) {
    std::ranges::transform(str, str.begin(), ::tolower);
    return str;
}

static std::vector<std::string> list_file_names(
    const std::string &path,
    const std::string &prefix,
    const std::string &ext) {
    std::vector<std::string> file_names;
    for ( const auto & entry : std::filesystem::directory_iterator(path)) {
        if (! entry.is_regular_file() ) continue ;
        if (const auto& name = entry.path().filename().string();
            name.rfind (prefix , 0) == 0 && entry.path().extension() == ext)
            file_names.push_back(entry.path().string());
    }
    return file_names;
}

static std::vector<MapType::const_iterator> sort_map(const MapType &map_words) {
    std::vector<MapType::const_iterator> its;
    its.reserve(map_words.size());

    // Copiar iterators
    for (auto it = map_words.cbegin(); it != map_words.cend(); ++it) {
        its.push_back(it);
    }

    // Ordenamos el vector de iterators según el valor (descendente)
    std::sort(its.begin(), its.end(), [](auto a, auto b) {
            // valor mayor primero
            if (a->second != b->second) return a->second > b->second;
            // Si son iguales, clave menor primero
            return a->first < b->first;
        });

    return its;
}

void WordCounterAggregator::reader_task(const std::string &filename) {
    std::ifstream file(filename);
    MapType word_counter;
    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string word;
        while (iss >> word) {
            word_counter[to_lower(word)]++;
        }
    }

    // Sincronización de PRODUCTOR - Agregando mapas temporal
    {
        std::lock_guard lock(mutex_);
        words_queue_.emplace(word_counter);
    }

    // Notificar y despertar aggregator
    condition_.notify_one();
}

void WordCounterAggregator::aggregate_task() {
    int i = 0;
    while (i < n_files_) {
        // Sincronización de CONSUMIDOR - Durmiendo el hilo aggregator
        // si no hay valores en cola
        std::unique_lock lock(mutex_);
        condition_.wait(lock, [this] { return! words_queue_.empty(); });

        // Descargando mapas temporales
        while (!words_queue_.empty()) {
            auto temp_map = words_queue_.front();
            words_queue_.pop();
            lock.unlock();
            // Combinar mapas
            join_maps(map_words_, temp_map,
                [](auto a, auto b) { return a + b; });
        }

        // Incrementando contador
        ++i;
    }
}

WordCounterAggregator::WordCounterAggregator(std::string path):
    path_(std::move(path)),
    n_files_(0) {}

void WordCounterAggregator::run(
    const std::string &file_pattern,
    const std::string &result_filename) {
    const auto file_names = list_file_names(path_, file_pattern, ".txt");
    n_files_ = file_names.size();

    // Generar hilos
    reader_threads_.reserve(n_files_);
    for (const auto & file_name : file_names) {
        reader_threads_.emplace_back(&WordCounterAggregator::reader_task, this, file_name);
    }
    aggregator_thread_ = std::jthread(&WordCounterAggregator::aggregate_task, this);

    // Union de hilos
    for (auto& reader : reader_threads_) reader.join();
    aggregator_thread_.join();

    // Ordenar mapa
    const auto& result = sort_map(map_words_);

    // Generar archivo
    std::ofstream result_file(result_filename);
    std::transform(result.begin(), result.end(),
       std::ostream_iterator<std::string>(result_file, "\n"),
       [](auto a) {
           return a->first + ": " + std::to_string(a->second);
       });
}
