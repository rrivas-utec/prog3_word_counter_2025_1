#include <iostream>
#include "word_counter_aggregator.h"

int main() {
    WordCounterAggregator wc(".");
    wc.run("chunk", "word_counts.txt");
    return 0;
}