#ifndef _HEADER_H
#define _HEADER_H

#include <iostream>
#include <vector>
#include <unordered_map>
#include <set>

#include <string>
#include <sstream>

#define SUPPORT_SET 0
#define AUX_SET 1

#define IMPLY 0
#define AND 1
#define OR 2

using namespace std;

typedef int OPERATOR_TYPE;
typedef long PRED_ID_TYPE;
typedef long UNIV_ID_TYPE;
typedef long SENTENCE_ID_TYPE;
typedef int SET_TYPE;

// Declarations part
int getPredIdByName(string name);
int resolution_and_put_result_into_support_set(SENTENCE_ID_TYPE id1, long p1, SENTENCE_ID_TYPE id2, SET_TYPE set_id, vector<SENTENCE_ID_TYPE>& set_to_put);
/* Element is the universal quatifier or assigned Const name */
class Element {

public:

	bool isUniverse;

	string constNname;

	UNIV_ID_TYPE universeId;

	string stringify() const {
		std::stringstream ss;
		if(isUniverse) {
			ss <<"u"<<universeId;
			return ss.str();
		}
		else return constNname;
	}

	Element(string cnst) {
		isUniverse = false;
		constNname = cnst;
	}

	Element(UNIV_ID_TYPE univ) {
		isUniverse = true;
		universeId = univ;
	}

	Element() {}

	bool operator==(const Element &other) const
  {
		if(other.isUniverse != isUniverse) return false;
		else if(isUniverse == false) return other.constNname == constNname;
		else return other.universeId == universeId;
  }

	~Element(){}
};

namespace std
{
    template <>
    struct hash<Element>
    {
        size_t operator()(const Element& k) const
        {
            // Compute individual hash values for two data members and combine them using XOR and bit shifting
						return ((hash<bool>()(k.isUniverse)
										 ^ (hash<string>()(k.constNname) << 1)) >> 1)
										 ^ (hash<long>()(k.universeId) << 1);
			 }
    };
}

/* Literal is used in both normal FOL and CNF */
class Literal {

private:
	// True or negated
	bool noNegation;

	// Id of prediction
	PRED_ID_TYPE predict;

public:
	// Parameters
	vector<Element> paramList;

	bool negate() {
		noNegation = !noNegation;
		return noNegation;
	}

	bool getTrueOrNegated() {
		return noNegation;
	}

	void addParam(Element elem) {
		paramList.push_back(elem);
	}

  void setParams(vector<Element>& elems) {
    paramList.clear();
    for(int i = 0; i < elems.size(); i++) {
      paramList.push_back(elems[i]);
    }
  }

	PRED_ID_TYPE getPredictId(){
		return predict;
	}

	vector<Element>& getElements() {
		return paramList;
	}

	string stringify();

	Literal() {
		noNegation = true;
		predict = 0;
		paramList = vector<Element>(0);
	}

	Literal(PRED_ID_TYPE p) {
		noNegation = true;
		predict = p;
		paramList = vector<Element>(0);
	}

	Literal(string p_name) {
		noNegation = true;
		predict = getPredIdByName(p_name);
		paramList = vector<Element>(0);
	}

	Literal(string p_name, bool noNegat) {
		noNegation = noNegat;
		predict = getPredIdByName(p_name);
		paramList = vector<Element>(0);
	}

	Literal(PRED_ID_TYPE p, bool noNegat) {
		noNegation = noNegat;
		predict = p;
		paramList = vector<Element>(0);
	}

	~Literal() {}

};

class SentenceDNF {

private:

	vector<Literal> list;
	SET_TYPE belong_set;

public:
	set<SENTENCE_ID_TYPE> parents;

	SET_TYPE inSet() {
		return belong_set;
	}

	void setSet(SET_TYPE set) {
		belong_set = set;
	}

	void setParent(SENTENCE_ID_TYPE p) {
		parents.insert(p);
	}

	bool isMyParent(SENTENCE_ID_TYPE p) {
		return parents.find(p) != parents.end();
	}

	long literalCount() {
		return list.size();
	}

	void add(Literal l) {
		list.push_back(l);
	}

	string stringify();
	string stringify_local();
	vector<Literal>& getLiterals() {
		return list;
	}

	SentenceDNF(SET_TYPE set) {
		list = vector<Literal>(0);
		belong_set = set;
	}

	SentenceDNF() {
		list = vector<Literal>(0);
		belong_set = AUX_SET;
	}

	~SentenceDNF() {}

};

// For single literal, negation is kept at literal level
class SentenceFOL {

private:
	bool single;
	bool negated;
	OPERATOR_TYPE operat;
	SentenceFOL *op1;
	SentenceFOL *op2;
	Literal singleLiteral;

	SentenceDNF getDNFByFOL();
	void getDNFByFOL(SentenceDNF& dnf);

public:
	bool isSingle() {
		return single;
	}

	bool isNegated() {
		return negated;
	}

	bool negate() {
		if(single) {
				return singleLiteral.negate();
		}
		else {
				negated = !negated;
				return negated;
		}
	}

	void setOperator(OPERATOR_TYPE opt){
		single = false;
		operat = opt;
	}

	void setLeftOps(SentenceFOL *p1){
		single = false;
		op1 = p1;
	}

	void setRightOps(SentenceFOL *p2){
		single = false;
		op2 = p2;
	}

	bool addToKB(SET_TYPE set);
	void eliminateImplication();
	void walkInNegation();
	void generalToCNF();
	void putCNFIntoSentenceStore(SET_TYPE set);

	string stringify();

	SentenceFOL(OPERATOR_TYPE op, SentenceFOL &p1, SentenceFOL &p2) {
		single = false;
		negated = false;
		operat = op;
		op1 = &p1;
		op2 = &p2;
	}

	SentenceFOL(Literal l) {
		single = true;
		negated = false;
		singleLiteral = l;
		op1 = NULL;
		op2 = NULL;
	}

	SentenceFOL() {
		negated = false;
		op1 = NULL;
		op2 = NULL;
	}

	~SentenceFOL() {}

};


template <class T>
class IdGenerator{
private:
	T currentId;
public:
	T getNext() {
		return (++currentId);
	}
	IdGenerator(){currentId = 0;}
	~IdGenerator(){}
};

class TrueFalseLists {

private:
	set<SENTENCE_ID_TYPE> trueList;
	set<SENTENCE_ID_TYPE> falseList;

public:

	void addToTrueList(SENTENCE_ID_TYPE sentenceId) {
		trueList.insert(sentenceId);
	}

	set<SENTENCE_ID_TYPE> getTrueList() {
		return trueList;
	}

	void addToFalseList(SENTENCE_ID_TYPE sentenceId) {
		falseList.insert(sentenceId);
	}

	set<SENTENCE_ID_TYPE> getFalseList() {
		return falseList;
	}

	TrueFalseLists(){
		trueList = set<SENTENCE_ID_TYPE>();
		falseList = set<SENTENCE_ID_TYPE>();
	}

	~TrueFalseLists(){}

};

class Indexing {
private:
	unordered_map<PRED_ID_TYPE, TrueFalseLists> map;

public:
	vector<SENTENCE_ID_TYPE> find(PRED_ID_TYPE pred_id, bool trueOrNegated) {
		set<SENTENCE_ID_TYPE> t = trueOrNegated? map[pred_id].getTrueList(): map[pred_id].getFalseList();
		vector<SENTENCE_ID_TYPE> res;
		for(auto i = t.begin(); i != t.end(); i++) {
			res.push_back(*i);
		}
		return res;
	}

	void addSentence (SENTENCE_ID_TYPE sentenceId);

	string stringify ();

	Indexing() {
		map = unordered_map<PRED_ID_TYPE, TrueFalseLists>();
	}

	~Indexing() {}

};

void collapse(vector<Literal>& list);

bool find_a_substitution(
	vector<Element>& elems1, vector<Element>& elems2,
	unordered_map<Element, Element>& replace1,
	unordered_map<Element, Element>& replace2 );
void apply_a_substitution(vector<Literal>& list1, vector<Literal>& list2,
	unordered_map<Element, Element>& replace1,
	unordered_map<Element, Element>& replace2 );


#endif
