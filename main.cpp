#include "word_counter_aggregator.h"

int main() {
    WordCounterAggregator wc(".");
    wc.run("chunk_1", "word_counts_1.txt");
    wc.run("chunk_2", "word_counts_2.txt");
    return 0;
}