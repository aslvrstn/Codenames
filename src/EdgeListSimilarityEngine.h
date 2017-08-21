#pragma once

#include "Dictionary.h"
#include "SimilarityEngine.h"

#include <map>
#include <string>
#include <vector>

#include "Utilities.h"

struct EdgeListSimilarityEngine final : SimilarityEngine {
   private:
   	std::map<std::pair<wordID,wordID>,float> edges;

	Dictionary &dict;

   public:

	EdgeListSimilarityEngine(Dictionary &dict) : dict(dict) {}

	/** Arbitrary statistic, in this case the word norm. */
	float stat(wordID s);

	/** Returns true if successful */
	bool load(const std::string &fileName, bool verbose);

	float similarity(wordID fixedWord, wordID dynWord);

	/** True if the word2vec model includes a vector for the specified word */
	bool wordExists(const std::string &word);
};