#include <algorithm>
#include <cassert>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <queue>
#include <set>
#include <string>
#include <vector>
using namespace std;

#define rep(i, a, b) for (int i = (a); i < int(b); ++i)
#define rrep(i, a, b) for (int i = (a)-1; i >= int(b); --i)
#define trav(x, v) for (auto &x : v)
#define all(v) (v).begin(), (v).end()
#define what_is(x) cout << #x << " is " << x << endl;

typedef float fl;
typedef long long ll;
typedef pair<int, int> pii;
typedef vector<int> vi;
typedef vector<pii> vpi;

/** Represents a single word or phrase in a similarity engine */
enum wordID : int {};

void eraseFromVector(string word, vector<string> &v) {
	rep(i, 0, v.size()) {
		if (v[i] == word) {
			v.erase(v.begin() + i);
			i--;
		}
	}
}

string toLowerCase(string s) {
	rep(i, 0, s.size()) {
		if ('A' <= s[i] && s[i] <= 'Z') {
			s[i] = (char)tolower(s[i]);
		}
	}
	return s;
}

fl sigmoid(fl x) {
	return 1.0f / (1.0f + exp(-x));
}

struct SimilarityEngine {
private:
	map<string, wordID> word2id;
	vector<vector<fl>> words;
	vector<string> wordsStrings;

	/** Similarity between two word vectors.
	 * Implemented as an inner product.
	 */
	fl similarity(const vector<fl> &v1, const vector<fl> &v2) {
		fl ret = 0;
		int dim = (int)v1.size();
		rep(i, 0, dim) {
			ret += v1[i] * v2[i];
		}
		return ret;
	}

public:
	/** Returns true if successful */
	bool load(const char *fileName) {
		int dimension, numberOfWords;
		ifstream fin(fileName, ios::binary);
		fin.read((char *)&numberOfWords, sizeof numberOfWords);
		fin.read((char *)&dimension, sizeof dimension);
		if (!fin) {
			cerr << "Failed to load " << fileName << endl;
			return false;
		}
		cerr << "Loading word2vec (" << numberOfWords << " words, "
			 << dimension << " dimensions)..." << flush;

		const int bufSize = 1 << 16;
		char buf[bufSize];
		string word;
		vector<float> values(dimension);
		vector<fl> valuesd;
		words.resize(numberOfWords);
		wordsStrings.resize(numberOfWords);
		rep(i, 0, numberOfWords) {
			int len;
			fin.read((char *)&len, sizeof len);
			if (!fin) {
				cerr << " failed at reading entry " << i << endl;
				return false;
			}
			if (len > bufSize || len <= 0) {
				cerr << " invalid length " << len << endl;
				return false;
			}
			fin.read(buf, len);
			fin.read((char *)values.data(), dimension * sizeof(float));
			if (!fin) {
				cerr << " failed at reading entry " << i << endl;
				return false;
			}
			word.assign(buf, buf + len);
			valuesd.assign(all(values));
			words[i] = move(valuesd);
			wordsStrings[i] = word;
			word2id[word] = wordID(i);
		}
		cerr << " done!" << endl;
		return true;
	}

	/** Top N most popular words */
	vector<wordID> getCommonWords(int vocabularySize) {
		vector<wordID> ret;
		vocabularySize = min(vocabularySize, (int)words.size());
		ret.reserve(vocabularySize);
		for (int i = 0; i < vocabularySize; i++) {
			ret.push_back(wordID(i));
		}
		return ret;
	}

	fl similarity(wordID s1, wordID s2) {
		return similarity(words[s1], words[s2]);
	}

	fl similarity(const string &s1, const string &s2) {
		return similarity(getID(s1), getID(s2));
	}

	/** ID representing a particular word */
	wordID getID(const string &s) {
		return word2id.at(s);
	}

	/** Popularity of a word, the most popular word has a popularity of 1, the second most popular has a popularity of 2 etc. */
	int getPopularity(wordID id) {
		// Word IDs are the indices of words in the input file, which is assumed to be ordered according to popularity
		return id + 1;
	}

	/** Word string corresponding to the ID */
	const string &getWord(wordID id) {
		return wordsStrings[id];
	}

	/** True if the word2vec model includes a vector for the specified word */
	bool wordExists(const string &word) {
		return word2id.count(word) > 0;
	}

	vector<pair<fl, string>> similarWords(const string &s) {
		if (!wordExists(s)) {
			cout << s << " does not occur in the corpus" << endl;
			return vector<pair<fl, string>>();
		}
		vector<pair<fl, string>> ret;
		for (auto it : word2id) {
			ret.push_back(make_pair(-similarity(s, it.first), it.first));
		}
		sort(all(ret));
		vector<pair<fl, string>> res;
		rep(i, 0, 10) {
			res.push_back(make_pair(-ret[i].first, ret[i].second));
		}
		return res;
	}
};

struct Bot {
	// Give a similarity bonus to "bad" words
	fl marginCivilians = 0.02f;
	fl marginOpponentWords = 0.04f;
	fl marginAssassins = 0.07f;

	// Constants used in scoring function based
	// on the sigmoid function of the similarities
	fl fuzzyWeightAssassin = -0.2f;
	fl fuzzyWeightOpponent = -0.1f;
	fl fuzzyWeightMy = 0.1f;
	fl fuzzyWeightGrey = -0.05f;
	fl fuzzyExponent = 15;
	fl fuzzyOffset = 0.3f;

	// Assume that we will never succeed if the similarity
	// is at most minSimilarity
	fl minSimilarity = 0.2f;

	// Good words with smaller similarity than civilians and opponent
	// spies are worth less
	fl multiplierAfterBadWord = 0.7f;

	// How bad is it if there is an opponent word with high similarity
	fl weightOpponent = -1.5f;

	// How bad is it if there is a grey word with high similarity
	fl weightGrey = -0.2f;

	// How important is it that the last good word has greater
	// similarity than the next bad word
	fl marginWeight = 0.1f;

	// Number of words that are considered common
	int commonWordLimit = 1000;

	// Avoid common words
	fl commonWordWeight = 0.9f;

	// Number of words that are not considered rare
	int rareWordLimit = 15000;

	// Avoid rare words
	fl rareWordWeight = 0.8f;

	// Consider only the 50000 most common words
	int vocabularySize = 50000;

	SimilarityEngine &engine;

	Bot(SimilarityEngine &engine) : engine(engine) {}

	vector<string> myWords, opponentWords, greyWords, assassinWords;
	struct BoardWord {
		char type;
		string word;
		wordID id;
	};
	vector<BoardWord> boardWords;
	void addBoardWord(char type, const string &word) {
		boardWords.push_back({
			type,
			word,
			engine.getID(word)
		});
	}

	/** True if a is a super or substring of b or vice versa */
	bool superOrSubstring(const string &a, const string &b) {
		auto lowerA = toLowerCase(a);
		auto lowerB = toLowerCase(b);
		return lowerA.find(lowerB) != string::npos || lowerB.find(lowerA) != string::npos;
	}

	bool forbiddenWord(const string &word) {
		for (const BoardWord &w : boardWords) {
			if (superOrSubstring(w.word, word))
				return true;
		}
		return false;
	}

	pair<fl, int> getWordScore(wordID word, bool debugPrint) {
		if (debugPrint)
			cout << "Printing statistics for \"" << engine.getWord(word) << "\"" << endl;

		typedef pair<fl, BoardWord *> Pa;
		static vector<Pa> v;
		v.clear();
		rep(i, 0, boardWords.size()) {
			fl sim = engine.similarity(boardWords[i].id, word);
			if (boardWords[i].type == 'g')
				sim += marginCivilians;
			if (boardWords[i].type == 'o')
				sim += marginOpponentWords;
			if (boardWords[i].type == 'a')
				sim += marginAssassins;
			v.push_back({-sim, &boardWords[i]});
		}
		sort(all(v), [&](const Pa &a, const Pa &b) { return a.first < b.first; });

		if (debugPrint) {
			rep(i, 0, v.size()) {
				cout << setprecision(6) << fixed << -v[i].first << "\t" << v[i].second->word << " ";
				switch (v[i].second->type) {
					case 'm':
						cout << "(My)" << endl;
						break;
					case 'o':
						cout << "(Opponent)" << endl;
						break;
					case 'g':
						cout << "(Civilian)" << endl;
						break;
					case 'a':
						cout << "(Assassin)" << endl;
						break;
					default:
						assert(0);
				}
			}
			cout << endl;
		}

		// Compute a fuzzy score
		fl baseScore = 0;
		rep(i, 0, v.size()) {
			char type = v[i].second->type;
			fl weight;
			if (type == 'a')
				weight = fuzzyWeightAssassin;
			else if (type == 'o')
				weight = fuzzyWeightOpponent;
			else if (type == 'm')
				weight = fuzzyWeightMy;
			else if (type == 'g')
				weight = fuzzyWeightGrey;
			else
				assert(0);
			fl contribution = weight * sigmoid((-v[i].first - fuzzyOffset) * fuzzyExponent);
			baseScore += contribution;
		}

		int bestCount = 0;
		fl curScore = 0, bestScore = 0, lastGood = 0;
		int curCount = 0;
		fl mult = 1;
		rep(i, 0, v.size()) {
			if (-v[i].first < minSimilarity)
				break;
			char type = v[i].second->type;
			if (type == 'a')
				break;
			if (type == 'o') {
				curScore += weightOpponent;
				mult *= multiplierAfterBadWord;
				continue;
			}
			if (type == 'm') {
				lastGood = -v[i].first;
				curScore += mult * sigmoid((-v[i].first - fuzzyOffset) * fuzzyExponent);
				++curCount;
			}
			if (type == 'g') {
				curScore += mult * weightGrey;
				mult *= multiplierAfterBadWord;
				continue;
			}
			fl tmpScore = -1;
			rep(j, i + 1, v.size()) {
				char type2 = v[j].second->type;
				if (type2 == 'a' || type2 == 'o') {
					tmpScore = mult * marginWeight * sigmoid((lastGood - (-v[j].first)) * fuzzyExponent);
					break;
				}
			}
			tmpScore += baseScore + curScore;
			if (tmpScore > bestScore) {
				bestScore = tmpScore;
				bestCount = curCount;
			}
		}

		int popularity = engine.getPopularity(word);
		if (popularity < commonWordLimit)
			bestScore *= commonWordWeight;
		else if (popularity > rareWordLimit)
			bestScore *= rareWordWeight;
		return make_pair(bestScore, bestCount);
	}

	pair<fl, int> getWordScore(const string &word, bool debugPrint) {
		return getWordScore(engine.getID(word), debugPrint);
	}

	void setWords(const vector<string> &_myWords,
				  const vector<string> &_opponentWords,
				  const vector<string> &_greyWords,
				  const vector<string> &_assassinWords) {
		myWords = _myWords;
		opponentWords = _opponentWords;
		greyWords = _greyWords;
		assassinWords = _assassinWords;
		createBoardWords();
	}

	void createBoardWords() {
		boardWords.clear();
		trav(w, myWords) addBoardWord('m', w);
		trav(w, opponentWords) addBoardWord('o', w);
		trav(w, greyWords) addBoardWord('g', w);
		trav(w, assassinWords) addBoardWord('a', w);
	}

	/** Returns suffix on number such as 'th' for 5 or 'nd' for 2 */
	string orderSuffix(int p) {
		if (p % 10 == 1 && p % 100 != 11) {
			return "st";
		} else if (p % 10 == 2 && p % 100 != 12) {
			return "nd";
		} else if (p % 10 == 3 && p % 100 != 13) {
			return "rd";
		} else {
			return "th";
		}
	}

	pair<string, int> getBestWord() {
		vector<wordID> candidates = engine.getCommonWords(vocabularySize);
		priority_queue<pair<pair<fl, int>, wordID>> pq;
		for (wordID candidate : candidates) {
			pair<fl, int> res = getWordScore(candidate, false);
			pq.push({{res.first, -res.second}, candidate});
		}

		vector<pair<pair<fl, int>, wordID>> topList;

		// Extract the top 20 words that are not forbidden by the rules
		while (topList.size() < 20 && !pq.empty()) {
			auto pa = pq.top();
			pq.pop();
			if (!forbiddenWord(engine.getWord(pa.second)))
				topList.push_back(pa);
		}

		// Print how the score of the best word was computed
		assert(!topList.empty());
		wordID bestWord = topList[0].second;
		int bestCount = -topList[0].first.second;
		getWordScore(bestWord, true);

		// Print a list with the best clues
		rep(i, 0, (int)topList.size()) {
			auto res = topList[i];
			cout << (i + 1) << "\t" << setprecision(3) << fixed << res.first.first << "\t"
				 << engine.getWord(res.second) << " " << -res.first.second << endl;
		}

		int p = engine.getPopularity(bestWord);
		cout << "The best clue found is " << engine.getWord(bestWord) << " " << bestCount << endl;
		cout << engine.getWord(bestWord) << " is the " << p << orderSuffix(p);

		cout << " most popular word" << endl;
		return make_pair(engine.getWord(bestWord), bestCount);
	}
};

class GameInterface {
	SimilarityEngine engine;
	Bot bot;
	vector<string> myWords, opponentWords, greyWords, assassinWords;
	string myColor;

	void commandReset() {
		myWords.clear();
		opponentWords.clear();
		greyWords.clear();
		assassinWords.clear();
		bot.setWords(myWords, opponentWords, greyWords, assassinWords);
	}

	void commandSuggestWord() {
		cout << "Thinking..." << endl;
		bot.getBestWord();
	}

	void commandHelp() {
		cout << "The following commands are available:" << endl << endl;
		cout << "r <word>\t-\tAdd a red spy to the board" << endl;
		cout << "b <word>\t-\tAdd a blue spy to the board" << endl;
		cout << "c <word>\t-\tAdd a civilian to the board" << endl;
		cout << "a <word>\t-\tAdd an assassin to the board" << endl;
		cout << "- <word>\t-\tRemove a word from the board" << endl;
		cout << "go\t\t-\tReceive clues" << endl;
		cout << "reset\t\t-\tClear the board" << endl;
		cout << "board\t\t-\tPrints the words currently on the board" << endl;
		cout << "score <word>\t-\tCompute how good a given clue would be" << endl;
		cout << "quit\t\t-\tTerminates the program" << endl;
	}

	void commandBoard() {
		cout << "My spies:";
		for (auto word : myWords) {
			cout << " " << word;
		}
		cout << endl;
		cout << "Opponent spies:";
		for (auto word : opponentWords) {
			cout << " " << word;
		}
		cout << endl;
		cout << "Civilians:";
		for (auto word : greyWords) {
			cout << " " << word;
		}
		cout << endl;
		cout << "Assassins:";
		for (auto word : assassinWords) {
			cout << " " << word;
		}
		cout << endl;
	}

	void commandModifyBoard(const string &command) {
		vector<string> *v = NULL;
		if (command == myColor) {
			v = &myWords;
		} else if (command == "b" || command == "r") {
			v = &opponentWords;
		} else if (command == "g" || command == "c") {
			v = &greyWords;
		} else if (command == "a") {
			v = &assassinWords;
		} else if (command == "-") {
			string word;
			cin >> word;
			word = toLowerCase(word);
			eraseFromVector(word, myWords);
			eraseFromVector(word, opponentWords);
			eraseFromVector(word, greyWords);
			eraseFromVector(word, assassinWords);
		}

		if (v != NULL) {
			string word;
			cin >> word;
			word = toLowerCase(word);
			if (engine.wordExists(word)) {
				v->push_back(word);
			} else {
				cout << word << " was not found in the dictionary" << endl;
			}
		}
		bot.setWords(myWords, opponentWords, greyWords, assassinWords);
	}

	void commandScore() {
		string word;
		cin >> word;
		if (!engine.wordExists(word)) {
			cout << word << " was not found in the dictionary" << endl;
			return;
		}
		pair<fl, int> res = bot.getWordScore(word, true);
		cout << word << " " << res.second << " has score " << res.first << endl;
	}

	string inputColor() {
		string color;
		cin >> color;
		color = toLowerCase(color);
		while (true) {
			if (color == "b" || color == "blue") {
				color = "b";
				break;
			}
			if (color == "r" || color == "red") {
				color = "r";
				break;
			}
			cin >> color;
			color = toLowerCase(color);
		}
		return color;
	}

   public:
	GameInterface() : bot(engine) {}

	void run() {
		if (!engine.load("data.bin")) {
			cerr << "Failed to load data.bin" << endl;
			return;
		}

		cout << "Type \"help\" for help" << endl;
		cout << "My color (b/r): ";
		myColor = inputColor();

		while (true) {
			string command;
			cin >> command;
			if (!cin)
				break;
			command = toLowerCase(command);

			if (command.size() == 1 && string("rgbac-").find(command) != string::npos) {
				commandModifyBoard(command);
			}
			else if (command == "play" || command == "go") {
				commandSuggestWord();
			}
			else if (command == "quit" || command == "exit") {
				break;
			}
			else if (command == "reset") {
				commandReset();
			}
			else if (command == "help" || command == "\"help\"") {
				commandHelp();
			}
			else if (command == "board") {
				commandBoard();
			}
			else if (command == "score") {
				commandScore();
			}
			else {
				cout << "Unknown command \"" << command << "\"" << endl;
			}
		}
	}
};

int main() {
	GameInterface interface;
	interface.run();
	return 0;
}
