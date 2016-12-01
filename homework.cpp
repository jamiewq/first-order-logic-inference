#include "header.h"

// Code part

extern unordered_map<SENTENCE_ID_TYPE, SentenceDNF> sentenceStore;
extern unordered_map<string, PRED_ID_TYPE> predictStore;
extern vector<SENTENCE_ID_TYPE> set_support;
extern vector<SENTENCE_ID_TYPE> set_aux;
extern Indexing myIndex;
extern IdGenerator<SENTENCE_ID_TYPE> sentenceId_generator;
extern IdGenerator<PRED_ID_TYPE> predictionId_generator;
extern IdGenerator<UNIV_ID_TYPE> universeId_generator;

void  Indexing::addSentence (SENTENCE_ID_TYPE sentenceId) {
	vector<Literal> literals = sentenceStore[sentenceId].getLiterals();
	for(int i = 0; i < literals.size(); i++) {
		Literal l = literals[i];
		if(l.getTrueOrNegated()) map[l.getPredictId()].addToTrueList(sentenceId);
		else map[l.getPredictId()].addToFalseList(sentenceId);
	}
}

string Indexing::stringify () {
	stringstream ss;
	for(auto lt = map.begin(); lt != map.end(); lt++) {
		string predName;
		PRED_ID_TYPE id = lt->first;
		TrueFalseLists list = lt->second;
		for(auto ltttt = predictStore.begin(); ltttt != predictStore.end(); ltttt++) {
			if(ltttt->second == id)
				predName = ltttt->first;
		}
		ss<<"---------------------------------------------------"<<endl;
		ss<<predName<<"\t True:\t";
		set<SENTENCE_ID_TYPE> trues = list.getTrueList();
		for(auto i = trues.begin(); i != trues.end(); i++) {
			ss<<sentenceStore[*i].stringify()<<",\t";
		}
		ss<<endl;
		ss<<predName<<"\t False:\t";
		set<SENTENCE_ID_TYPE> falses = list.getFalseList();
		for(auto i = falses.begin(); i != falses.end(); i++) {
			ss<<sentenceStore[*i].stringify()<<",\t";
		}
		ss<<endl;
	}
	return ss.str();
}

string Literal:: stringify() {
	stringstream ss;
	if(!noNegation) ss <<"~";
	for(auto lt = predictStore.begin(); lt != predictStore.end(); lt++) {
		if(lt->second == predict)
		ss<< lt->first;
	}
	ss<<"(";
	for (int i = 0; i < paramList.size(); ++i)
	{
		ss<< paramList[i].stringify();
		if(i != paramList.size() - 1) ss<<",";
	}
	ss<<")";
	return ss.str();
}

string SentenceFOL:: stringify() {
	stringstream ss;
	if(single) {
		if(negated) ss<<"~";
		ss << singleLiteral.stringify();
	}
	else {
		if(negated) {
			ss << " ~(";
			ss << op1->stringify();
			if(operat == IMPLY) ss<<" => ";
			if(operat == AND) ss<<" & ";
			if(operat == OR) ss<<" | ";
			ss << op2->stringify();
			ss <<") ";
		}
		else {
			ss << " (";
			ss << op1->stringify();
			if(operat == IMPLY) ss<<" => ";
			if(operat == AND) ss<<" & ";
			if(operat == OR) ss<<" | ";
			ss << op2->stringify();
			ss << " )";
		}
	}
	return ss.str();
}

string SentenceDNF::stringify() {
	stringstream ss;
	for(int i = 0; i < list.size(); i++) {
		ss << list[i].stringify();
		if(i != list.size()-1) ss<<" | ";
	}
	return ss.str();
	//
	// stringstream ss;
	// int univeral_id_local_represent = 0;
	// unordered_map<int, int> find_local_represent;
	// for(int i = 0; i < list.size(); i++) {
	// 	Literal& lit = list[i];
	// 	if(!lit.getTrueOrNegated()) ss <<"~";
	// 	for(auto lt = predictStore.begin(); lt != predictStore.end(); lt++) {
	// 		if(lt->second == lit.getPredictId())
	// 			ss<< lt->first;
	// 	}
	// 	ss<<"(";
	// 	for (int j = 0; j < lit.paramList.size(); ++j)
	// 	{
	// 		if(lit.paramList[j].isUniverse) {
	// 			if(find_local_represent.find(lit.paramList[j].universeId) == find_local_represent.end()) {
	// 					find_local_represent[lit.paramList[j].universeId] = univeral_id_local_represent;
	// 					ss <<"u"<<univeral_id_local_represent++;
	// 			}
	// 			else {
	// 					ss <<"u"<<find_local_represent[lit.paramList[j].universeId];
	// 			}
	// 		}
	// 		else ss<<lit.paramList[j].constNname;
	// 		if(j != lit.paramList.size() - 1) ss<<",";
	// 	}
	// 	ss<<")";
	//
	// 	if(i != list.size()-1) ss<<" | ";
	// }
	// return ss.str();
}

void SentenceFOL::generalToCNF() {
	bool children_changed = true;
	while( !single && children_changed ) {
			OPERATOR_TYPE op1_op = op1->operat;
			OPERATOR_TYPE op2_op = op2->operat;
			children_changed = false;
			if(operat == OR) {
				if(!op1->isSingle() && op1->operat == AND) {
					//(t1.op1 & t1.op2) | (t2.op1 |/& .....)
					SentenceFOL *t1 = op1;
					SentenceFOL *t2 = op2;
					// TODO: FIX THE MEMORY LEAK
					op1 = new SentenceFOL(OR, *(t1->op1), *t2);
					op2 = new SentenceFOL(OR, *(t1->op2), *t2);
					operat = AND;
				}
				else if(!op2->isSingle() && op2->operat == AND) {
					// (t1.op1 |/& .....) | (t2.op1 & t2.op2)
					SentenceFOL *t1 = op1;
					SentenceFOL *t2 = op2;
					// TODO: FIX THE MEMORY LEAK
					op1 = new SentenceFOL(OR, *t1, *(t2->op1));
					op2 = new SentenceFOL(OR, *t1, *(t2->op2));
					operat = AND;
				}
			}
			// Should it be out of the if(operat == OR) ?
			if(op1) op1->generalToCNF();
			if(op2) op2->generalToCNF();


			if( (op1_op != op1->operat) || (op2_op != op2->operat)) {
				children_changed = true;
			}
	}
}

void SentenceFOL::eliminateImplication() {
	// Implication Elimination
	if(!single) {
		if(operat == IMPLY) {
				operat = OR;
				op1->negate();
		}
		op1->eliminateImplication();
		op2->eliminateImplication();
	}
}

void SentenceFOL::walkInNegation() {
	if(negated) {
		if(!single) {
			negated = false;
			if(operat == OR) operat = AND;
			else if(operat == AND) operat = OR;
			op1->negate();
			op2->negate();
		}
	}
	if(!single) {
		op1->walkInNegation();
		op2->walkInNegation();
	}
}

bool SentenceFOL::addToKB(SET_TYPE set) {
	// cout <<"origin"<<endl;
	// cout<<stringify()<<endl;
	eliminateImplication();
	// cout <<"eliminateImplication"<<endl;
	// cout<<stringify()<<endl;
	walkInNegation();
	// cout <<"walkInNegation"<<endl;
	// cout<<stringify()<<endl;
	generalToCNF();
	// cout <<"generalToCNF"<<endl;
	// cout<<stringify()<<endl;
	putCNFIntoSentenceStore(set);
	return true;
}

// (A|B) & (C|D) & (E|F|G) & ....
void SentenceFOL::putCNFIntoSentenceStore(SET_TYPE set) {
	if(single) {
		SENTENCE_ID_TYPE id = sentenceId_generator.getNext();
		sentenceStore[id] = getDNFByFOL();
		sentenceStore[id].setSet(set);
		myIndex.addSentence(id);
		if(set == SUPPORT_SET) set_support.push_back(id);
		else if(set == AUX_SET) set_aux.push_back(id);
	}
	else {
		if(operat == AND) {
			op1->putCNFIntoSentenceStore(set);
			op2->putCNFIntoSentenceStore(set);
		}
		else {
			SENTENCE_ID_TYPE id = sentenceId_generator.getNext();
			sentenceStore[id] = getDNFByFOL();
			sentenceStore[id].setSet(set);
			myIndex.addSentence(id);
			if(set == SUPPORT_SET) set_support.push_back(id);
			else if(set == AUX_SET) set_aux.push_back(id);
		}
	}
}

// A|B|C|D|E
SentenceDNF SentenceFOL::getDNFByFOL() {
	SentenceDNF res;
	if(single) {
		res.add(singleLiteral);
	}
	else if(operat == OR){
		op1->getDNFByFOL(res);
		op2->getDNFByFOL(res);
	}
	return res;
}

void SentenceFOL::getDNFByFOL(SentenceDNF& dnf) {
	if(single) {
		dnf.add(singleLiteral);
	}
	else if(operat == OR){
		op1->getDNFByFOL(dnf);
		op2->getDNFByFOL(dnf);
	}
}

int getPredIdByName(string name) {
	if(predictStore.find(name) == predictStore.end()) {
		predictStore[name] = predictionId_generator.getNext();
	}
	return predictStore[name];
}

bool find_a_substitution(
	vector<Element>& elems1, vector<Element>& elems2,
	unordered_map<Element, Element>& replace1,
	unordered_map<Element, Element>& replace2 ) {

		int count = elems1.size();

		vector< vector<Element> > sets;
		unordered_map<string, long> belongs_to_set;
		unordered_map<string, int> in_sentence;

		for(int i = 0; i < count; i++) {
			Element& elem1 = elems1[i];
			Element& elem2 = elems2[i];
			string e1 = elem1.stringify();
			string e2 = elem2.stringify();

			if(elem1.isUniverse) in_sentence[e1] = 1;
			if(elem2.isUniverse) in_sentence[e2] = 2;

			if( !elem1.isUniverse && !elem2.isUniverse) {
				if(e1 != e2) return false;
			}
			else {
				if(belongs_to_set.find(e1) == belongs_to_set.end()) {
					if(belongs_to_set.find(e2) == belongs_to_set.end()) {
						vector<Element> new_set;
						new_set.push_back(elem1);
						new_set.push_back(elem2);
						sets.push_back(new_set);
						belongs_to_set[e1] = sets.size()-1;
						belongs_to_set[e2] = sets.size()-1;
					}
					else {
						long setid = belongs_to_set[e2];
						sets[setid].push_back(elem1);
						belongs_to_set[e1] = setid;
					}
				}
				else {
					if( belongs_to_set.find(e2) == belongs_to_set.end() ) {
						long setid = belongs_to_set[e1];
						sets[setid].push_back(elem2);
						belongs_to_set[e2] = setid;
					}
					else {
						long setid1 = belongs_to_set[e1];
						long setid2 = belongs_to_set[e2];
						if(setid1 != setid2) {
							vector<Element>& set1 = sets[setid1];
							vector<Element>& set2 = sets[setid2];
							for(auto lt = set2.begin(); lt != set2.end(); lt++) {
								belongs_to_set[(*lt).stringify()] = setid1;
								set1.push_back(*lt);
							}
							set2.clear();
						}
					}
				}
			}
		}


		for(int i = 0; i < sets.size(); i++) {
			vector<Element>& s = sets[i];
			int const_count = 0;
			Element change_to;

			for(auto lt = s.begin(); lt != s.end(); lt++) {
				if( !(*lt).isUniverse ) {
					change_to = *lt;
					const_count++;
				}
				else if(const_count == 0) {
					change_to = *lt;
				}
				if(const_count > 1) return false;
			}

			for(auto lt = s.begin(); lt != s.end(); lt++) {
				if( (*lt).isUniverse ) {
					if(in_sentence[(*lt).stringify()] == 1) replace1[*lt] = change_to;
					else replace2[*lt] = change_to;
				}
			}

		}
		// cout<<"change in sentence1"<<endl;
		// for(auto lt = replace1.begin(); lt != replace1.end(); lt++) {
		//   cout << (lt->first).stringify() << "\t -> "<< (lt->second).stringify();
		//   cout <<endl;
		// }
		// cout<<"change in sentence2"<<endl;
		// for(auto lt = replace2.begin(); lt != replace2.end(); lt++) {
		//   cout << (lt->first).stringify() << "\t -> "<< (lt->second).stringify();
		//   cout <<endl;
		// }
		// cout<<"change in sentence2 end"<<endl;

		return true;
}

void apply_a_substitution(vector<Literal>& list1, vector<Literal>& list2,
	unordered_map<Element, Element>& replace1,
	unordered_map<Element, Element>& replace2 ) {
		// Each predicate
		// cout << "Literal in sentence1"<<endl;
		// for(int i = 0; i < list1.size(); i++) cout << list1[i].stringify() << endl;
		// cout << "Literal in sentence2"<<endl;
		// for(int i = 0; i < list2.size(); i++) cout << list2[i].stringify() << endl;
		// cout<<"change in sentence1"<<endl;
		// for(auto lt = replace1.begin(); lt != replace1.end(); lt++) {
		//   cout << (lt->first).stringify() << "\t -> "<< (lt->second).stringify();
		//   cout <<endl;
		// }
		// cout<<"change in sentence2"<<endl;
		// for(auto lt = replace2.begin(); lt != replace2.end(); lt++) {
		//   cout << (lt->first).stringify() << "\t -> "<< (lt->second).stringify();
		//   cout <<endl;
		// }
		// cout<<"change in sentence2 end"<<endl;


		for(int j = 0; j < list1.size(); j++) {
			vector<Element>& elms = list1[j].getElements();
			// Each literal
			for(int k = 0; k < elms.size(); k++) {
				if(replace1.find(elms[k]) != replace1.end())
					elms[k] = replace1[elms[k]];
			}
		}

		for(int j = 0; j < list2.size(); j++) {
			vector<Element>& elms = list2[j].getElements();
			// Each literal
			for(int k = 0; k < elms.size(); k++) {
				if(replace2.find(elms[k]) != replace2.end()) {
					elms[k] = replace2[elms[k]];
				}
			}
		}

		// cout << "-----------after substitution" <<endl;
		// cout << "Literal in sentence1"<<endl;
		// for(int i = 0; i < list1.size(); i++) cout << list1[i].stringify() << endl;
		// cout << "Literal in sentence2"<<endl;
		// for(int i = 0; i < list2.size(); i++) cout << list2[i].stringify() << endl;

}

//Eliminate exactly same literals F(u1,A,B,u2) with F(u1,A,B,u2)
void EliminateExactlySameLiterals(vector<Literal>& list1, vector<Literal>& list2) {
	set<string> s1_literals;
	if(list2.size() == 0 || list1.size() == 0) return;
	for(int i = 0 ; i < list1.size(); i++ ) {
			s1_literals.insert(list1[i].stringify());
	}
	for(int i = 0 ; i < list2.size();) {
			if(s1_literals.find(list2[i].stringify()) != s1_literals.end() ) {
				//cout <<"found" <<endl;
				list2.erase(list2.begin() + i);
			}
			else i++;
	}
}

bool duplicateWithAncestors(string sentence, SENTENCE_ID_TYPE parent) {
	if(sentence == sentenceStore[parent].stringify()) {
		return true;
	}
	for(auto ll = sentenceStore[parent].parents.begin(); ll != sentenceStore[parent].parents.end(); ll++) {
		if(duplicateWithAncestors(sentence, *ll))
			return true;
	}
	return false;
}

void assign_new_univs(vector<Literal>& list ) {
	// Id old to new
	unordered_map<int, int> old_to_new;
	for(int i = 0 ; i < list.size(); i++) {
		for(int j = 0; j < list[i].paramList.size(); j++) {
			Element  e = list[i].paramList[j];
			if(e.isUniverse) {
				if(old_to_new.find(e.universeId) == old_to_new.end()) {
					UNIV_ID_TYPE new_id = universeId_generator.getNext();
					old_to_new[e.universeId] = new_id;
					list[i].paramList[j] = Element(new_id);
				}
				else {
					list[i].paramList[j] = Element(old_to_new[e.universeId]);
				}
			}
		}
	}
}

string literal_internal_stringify(Literal& lit) {
	unordered_map<int, int> find_local_represent;
	int univeral_id_local_represent = 0;
	stringstream ss;
	for (int j = 0; j < lit.paramList.size(); ++j) {
		if(lit.paramList[j].isUniverse) {
			if(find_local_represent.find(lit.paramList[j].universeId) == find_local_represent.end()) {
				find_local_represent[lit.paramList[j].universeId] = univeral_id_local_represent;
				ss <<"u"<<univeral_id_local_represent++;
			}
			else {
				ss <<"u"<<find_local_represent[lit.paramList[j].universeId];
			}
		}
		else ss<<lit.paramList[j].constNname;
		if(j != lit.paramList.size() - 1) ss<<",";
	}
	return ss.str();
}

bool literals_have_same_pattern(Literal& l1, Literal& l2) {
	string str1 = literal_internal_stringify(l1);
	string str2 = literal_internal_stringify(l2);
	cout << "comparing two literals pattern"<<endl;
	cout << str1 <<endl;
	cout << str2 <<endl;
	if(str1 == str2) return true;
	return false;
}

// 0 keep both;
// 1 keep l1;
// 2 keep l2;
int keepWhich(Literal l1, Literal l2, int pos_1, int pos_2, unordered_map<UNIV_ID_TYPE, bool>& uni_has_constrain, unordered_map<long, bool>& literal_only_contain_internal_constrain) {

	for(auto lt = uni_has_constrain.begin(); lt != uni_has_constrain.end(); lt++) {
	  cout << "u" <<lt->first << "\t -> "<< lt->second;
	  cout <<endl;
	}

	if(literal_only_contain_internal_constrain[pos_1] && literal_only_contain_internal_constrain[pos_2]) {
		if(literals_have_same_pattern(l1,l2)) {
			return 1;
		}
	}

	bool keep1 = false;
	bool keep2 = false;
	for(int i = 0 ; i < l1.paramList.size(); i++) {
		Element e1 = l1.paramList[i];
		Element e2 = l2.paramList[i];
		if( !e1.isUniverse && !e2.isUniverse ) {
			if(e1.constNname != e2.constNname) return 0;
		}
		else {
			if( !e1.isUniverse && e2.isUniverse ) {
				if(uni_has_constrain[e2.universeId]) return 0;
				else keep2 = true;
			}
			else if(!e2.isUniverse && e1.isUniverse ) {
				if( uni_has_constrain[e1.universeId] ) return 0;
				else keep1 = true;
			}
			else if( e1.isUniverse && e2.isUniverse ) {
				if(e1.universeId == e2.universeId) {}
				else if(uni_has_constrain[e1.universeId] && uni_has_constrain[e2.universeId]) return 0;
				else if(uni_has_constrain[e1.universeId]) keep2 = true;
				else if(uni_has_constrain[e2.universeId]) keep1 = true;
			}
		}
	}
	if(keep1 && keep2) {
		// cout << l1.stringify() << " and " << l2.stringify() << " keep both" << endl;
		return 0;
	}
	else if(keep1) {
		// cout << l1.stringify() << " and " << l2.stringify() << " keep 1" << endl;
		return 1;
	}
	else {
		// cout << l1.stringify() << " and " << l2.stringify() << " keep 2" << endl;
		return 2;
	}
}

// deal with A|B, ~B|A to A | A to A
void collapse(vector<Literal>& list) {
	unordered_map<UNIV_ID_TYPE, bool> uni_has_constrain;
	unordered_map<UNIV_ID_TYPE, int> uni_last_pos;
	unordered_map<UNIV_ID_TYPE, bool> univ_only_contain_internal_constrain;
	unordered_map<long, bool> literal_only_contain_internal_constrain;

	for(int j = 0 ; j < list.size(); j++) {
		for(int i = 0 ; i < list[j].paramList.size(); i++) {
			if(list[j].paramList[i].isUniverse) {
				UNIV_ID_TYPE univId = list[j].paramList[i].universeId;
				if( uni_last_pos.find(univId) != uni_last_pos.end() ) {
					if( uni_last_pos[univId] != j) {
						univ_only_contain_internal_constrain[univId] = false;
						uni_last_pos[univId] = j;
					}
				}
				else {
					univ_only_contain_internal_constrain[univId] = true;
					uni_last_pos[univId] = j;
				}

				if(uni_has_constrain.find(univId) == uni_has_constrain.end() ) {
					uni_has_constrain[univId] = false;
				}
				else uni_has_constrain[univId] = true;
			}
		}
	}

	for(int j = 0 ; j < list.size(); j++) {
		literal_only_contain_internal_constrain[j] = true;
		for(int i = 0 ; i < list[j].paramList.size(); i++) {
			if(list[j].paramList[i].isUniverse) {
				UNIV_ID_TYPE univId = list[j].paramList[i].universeId;
				if(!univ_only_contain_internal_constrain[univId]) {
					literal_only_contain_internal_constrain[j] = false;
					break;
				}
			}
		}
	}

	for(int i = 0; i < list.size(); ) {
		Literal l1 = list[i];
		int j = i+1;
		for(; j < list.size(); ) {
			Literal l2 = list[j];
			if(l1.getPredictId() == l2.getPredictId() && l1.getTrueOrNegated() == l2.getTrueOrNegated()) {
				int keep = keepWhich(l1, l2, i, j, uni_has_constrain, literal_only_contain_internal_constrain);
				if(keep == 1) {
					list.erase(list.begin() + j );
				}
				else if(keep == 2) {
					list.erase(list.begin() + i );
					break;
				}
				else {
					j ++;
				}
			}
			else j++;
		}
		if( j == list.size() ) {
			i++;
		}
	}

}

// 0-cannot resolve
// 1-resolved successfully
// 2-resolve into empty set

int resolution_and_put_result_into_support_set(SENTENCE_ID_TYPE id1, long p1, SENTENCE_ID_TYPE id2, SET_TYPE set_id, vector<SENTENCE_ID_TYPE>& set_to_put) {
	SentenceDNF s1 = sentenceStore[id1];
	SentenceDNF s2 = sentenceStore[id2];
	vector<Literal> list1 = s1.getLiterals();
	Literal literal1 = list1[p1];
	vector<Literal> list2 = s2.getLiterals();
	long list2_matched_position = -1;

	// Unify and Resolution
	for(int i = 0; i < list2.size(); i++) {
		// Find first substitutable literal
		if(list2[i].getPredictId() == literal1.getPredictId() && list2[i].getTrueOrNegated()!=literal1.getTrueOrNegated()) {
			unordered_map<Element, Element> replace1;
			unordered_map<Element, Element> replace2;
			vector<Element>& elems1 = literal1.getElements();
			vector<Element>& elems2 = list2[i].getElements();

			// Find substitution
			if(find_a_substitution(elems1, elems2, replace1, replace2)) {
				list2_matched_position = i;
				// Apply substitution
				apply_a_substitution(list1, list2, replace1, replace2);
				break;
			}

		}
	}

	if(list2_matched_position != -1) {

		list1.erase(list1.begin() + p1);
		list2.erase(list2.begin() + list2_matched_position);

		EliminateExactlySameLiterals(list1, list2);

		cout << "-----------after EliminateExactlySameLiterals" <<endl;
		cout << "Literal in sentence1"<<endl;
		for(int i = 0; i < list1.size(); i++) cout << list1[i].stringify() << endl;
		cout << "Literal in sentence2"<<endl;
		for(int i = 0; i < list2.size(); i++) cout << list2[i].stringify() << endl;

		list1.insert( list1.end(), list2.begin(), list2.end() );
		if(list1.size() == 0) return 2;

		assign_new_univs(list1);

		collapse(list1);

		SentenceDNF newSentence(set_id);
		for(int i = 0; i < list1.size(); i++) {
			newSentence.add(list1[i]);
		}

		for(auto lt = sentenceStore.begin(); lt != sentenceStore.end(); lt++) {
			if(newSentence.stringify() == lt->second.stringify()) {
				//cout <<"Already in sentenceStore"<<endl;
				return 0;
			}
		}

		newSentence.setParent(id1);
		newSentence.setParent(id2);

		if(duplicateWithAncestors(newSentence.stringify(), id1) || duplicateWithAncestors(newSentence.stringify(), id2)) {
			return 0;
		}

		SENTENCE_ID_TYPE id = sentenceId_generator.getNext();
		sentenceStore[id] = newSentence;
		myIndex.addSentence(id);
		set_to_put.push_back(id);
		cout<<"Get  " << id <<" by Resolve  "<<id1 << " with  "<< id2<<endl;
		return 1;
	}

	return 0;
}
