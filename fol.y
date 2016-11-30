%{
#include <cstdio>
#include <cstring>
#include <iostream>
#include <map>
#include <vector>
#include <fstream>
#include "header.h"

using namespace std;

extern "C" struct yy_buffer_state;
typedef yy_buffer_state *YY_BUFFER_STATE;
extern int yyparse();
extern YY_BUFFER_STATE yy_scan_string(const char *str);

// stuff from flex that bison needs to know about:
extern "C" int yylex();
extern "C" FILE *yyin;
void yyerror(const char *s);

map<string, UNIV_ID_TYPE> inside_literal_universal_name_to_id_map;

unordered_map<SENTENCE_ID_TYPE, SentenceDNF> sentenceStore;
unordered_map<string, PRED_ID_TYPE> predictStore;
vector<SENTENCE_ID_TYPE> set_support;
vector<SENTENCE_ID_TYPE> set_aux;
Indexing myIndex;

unordered_map<SENTENCE_ID_TYPE, SentenceDNF> sentenceStore_origin;
unordered_map<string, PRED_ID_TYPE> predictStore_origin;
vector<SENTENCE_ID_TYPE> set_support_origin;
vector<SENTENCE_ID_TYPE> set_aux_origin;
Indexing myIndex_origin;

IdGenerator<SENTENCE_ID_TYPE> sentenceId_generator = IdGenerator<SENTENCE_ID_TYPE>();
IdGenerator<PRED_ID_TYPE> predictionId_generator = IdGenerator<PRED_ID_TYPE>();
IdGenerator<UNIV_ID_TYPE> universeId_generator = IdGenerator<UNIV_ID_TYPE>();

IdGenerator<SENTENCE_ID_TYPE> sentenceId_generator_origin = IdGenerator<SENTENCE_ID_TYPE>();
IdGenerator<PRED_ID_TYPE> predictionId_generator_origin = IdGenerator<PRED_ID_TYPE>();
IdGenerator<UNIV_ID_TYPE> universeId_generator_origin = IdGenerator<UNIV_ID_TYPE>();

SET_TYPE current_set_to_put = AUX_SET;

%}

// Bison fundamentally works by asking flex to get the next token, which it
// returns as an object of type "yystype".  But tokens could be of any
// arbitrary data type!  So we deal with that in Bison by defining a C union
// holding each of the types of tokens that Flex could return, and have Bison
// use that union instead of "int" for the definition of "yystype":

%code requires
{
  #include "header.h"
}

%union {
    char *uppercase;
    char *lowercase;
    char *operat;
    SentenceFOL *sfol;
    Element *element;
    Literal *ltr;
    vector<Element> *prmlist;
}

%type <sfol> sentence
%type <sfol> exp
%type <ltr> literal
%type <prmlist> paramlist
%type <element> param
%type <element> constant
%type <element> variable

%token <uppercase> UPPERCASE
%token <lowercase> LOWERCASE
%token LEFTP RIGHTP NEGATE COMMA
%token <operat> OP

%start sentence

%%
sentence:
        exp {

          cout<<"add to KB"<<current_set_to_put<<endl;

          if(current_set_to_put == AUX_SET) {
            $1->addToKB(current_set_to_put);
          }
          else {
            // Negate the query
            $1->negate();
            $1->addToKB(current_set_to_put);
          }

          cout<<$1->stringify()<<endl;

          inside_literal_universal_name_to_id_map.clear();
          $$ = $1;
        }
  ;
exp:
        LEFTP exp OP exp RIGHTP  {
                                    SentenceFOL *s = new SentenceFOL();
                                    if(string($3) == "&") {
                                      s->setOperator(AND);
                                    }
                                    else if(string($3) == "|") {
                                      s->setOperator(OR);
                                    }
                                    else if (string($3) == "=>") {
                                      s->setOperator(IMPLY);
                                    }
                                    s->setLeftOps($2);
                                    s->setRightOps($4);
                                    $$ = s;
                                  }
    |   literal     { $$ = new SentenceFOL(*($1)); }
    |   LEFTP NEGATE exp RIGHTP     { $3->negate(); $$ = $3; }
    ;
literal:
        constant LEFTP paramlist RIGHTP    { Literal *l = new Literal($1->stringify()); l->setParams(*($3)); $$ = l; }
    ;
paramlist:
        paramlist COMMA param     { $1->push_back(*($3)); $$ = $1; }
    |   param   { vector<Element> *list = new vector<Element>(); list->push_back(*($1)); $$ = list; }
    ;
param:
        constant { $$ = $1; }
    |   variable { $$ = $1; }
    ;
constant:
    UPPERCASE  { $$ = new Element(string($1)); }
    ;
variable:
    LOWERCASE  {
      string name = string($1);
      UNIV_ID_TYPE id;
      if(inside_literal_universal_name_to_id_map.find(name) == inside_literal_universal_name_to_id_map.end()) {
        inside_literal_universal_name_to_id_map[name] = universeId_generator.getNext();
      }
      $$ = new Element(inside_literal_universal_name_to_id_map[name]);
    }
    ;
%%

int main(int, char**) {
    ifstream in_file;
    ofstream out_file;
    in_file.open("input.txt");
	  out_file.open("output.txt");
    int query_size = 0;
    string str;
    std::getline(in_file, str);
    query_size = stoi(str);
    vector<string> querys(query_size);
    for(int i = 0; i < query_size; i++) {
      std::getline(in_file, querys[i]);
      if(querys[i][0] == '~') {
        querys[i].insert(querys[i].begin(), '(');
        querys[i].push_back(')');
      }
    }

    int kb_size = 0;
    std::getline(in_file, str);
    kb_size = stoi(str);
    vector<string> kb(kb_size);
    for(int i = 0; i < kb_size; i++) {
      std::getline(in_file,kb[i]);
      // We need two NULL ending to make sure it works
      //http://stackoverflow.com/questions/780676/string-input-to-flex-lexer
      kb[i].push_back('\0');
      kb[i].push_back('\0');
      char *a=new char[kb[i].size()+2];
      memcpy(a,kb[i].c_str(),kb[i].size());
      yy_scan_string(a);
      yyparse();
    }

    // Preprocessing KB
    int unit_resolved = false;
    while(true) {
        for(int k = 0; k < set_aux.size(); k++) {
          SENTENCE_ID_TYPE sid = set_aux[k];
          SentenceDNF s = sentenceStore[sid];
          if(s.getLiterals().size() != 1 ) continue;
          vector<Literal> literals = s.getLiterals();

          bool resolved = false;
          for(int i = 0; i < literals.size(); i++) {
            Literal literal = literals[i];
            vector<SENTENCE_ID_TYPE> foundlist = myIndex.find(literal.getPredictId(), !literal.getTrueOrNegated());
            for(int j = 0; j < foundlist.size(); j++) {
              SENTENCE_ID_TYPE id = foundlist[j];
              if( s.isMyParent(id) ) continue;
              if( s.getLiterals().size() != 1 && sentenceStore[id].getLiterals().size() != 1) continue;
              int t = resolution_and_put_result_into_support_set(sid, i, id, AUX_SET, set_aux);
              // resolved successfully
              if(t == 1) {
                resolved = true;
                /*
                for(auto lt = sentenceStore.begin(); lt != sentenceStore.end(); lt++) {
                  cout << lt->first << "\t"<< (lt->second.inSet() == AUX_SET? "AUX_SET": "SUPPORT_SET")<<"\t\t" << lt->second.stringify();
                  cout << "\t parent : ";
                  for(auto ll = lt->second.parents.begin(); ll != lt->second.parents.end(); ll++) {
                    cout << " S_"<< *ll ;
                  }
                  cout <<endl;
                }
                getchar();
                */
              }
              else if (t==2) {
                cout<<"Invalid input, KB has Conflict" <<endl;
                break;
              }
            }
          }

        }
        if(!unit_resolved) break;
    }


    // Store a copy of initial state
    sentenceStore_origin = sentenceStore;
    predictStore_origin = predictStore;
    set_support_origin = set_support;
    set_aux_origin = set_aux;
    myIndex_origin = myIndex;
    sentenceId_generator_origin = sentenceId_generator;
    predictionId_generator_origin = predictionId_generator;
    universeId_generator_origin = universeId_generator;


/*
    cout<< "======================== Indexing ========================" << endl;
    cout<<myIndex.stringify()<<endl;

    cout<< "======================== SUPPORT SET ========================" << endl;
    for(int i = 0; i < set_support.size(); i++) {
      cout<<sentenceStore[set_support[i]].stringify()<<endl;
    }

    cout<< "======================== AUX SET ========================" << endl;
    for(int i = 0; i < set_aux.size(); i++) {
      cout<<sentenceStore[set_aux[i]].stringify()<<endl;
    }
*/
    for(int q = 0; q < query_size; q++) {
          sentenceStore = sentenceStore_origin;
          predictStore = predictStore_origin;
          set_support = set_support_origin;
          set_aux = set_aux_origin;
          myIndex = myIndex_origin;
          sentenceId_generator = sentenceId_generator_origin;
          predictionId_generator = predictionId_generator_origin;
          universeId_generator = universeId_generator_origin;

          current_set_to_put = SUPPORT_SET;
          // We need two NULL ending to make sure it works
          //http://stackoverflow.com/questions/780676/string-input-to-flex-lexer
          querys[q].push_back('\0');
          querys[q].push_back('\0');
          char *a=new char[querys[q].size()+2];
          memcpy(a,querys[q].c_str(),querys[q].size());
          yy_scan_string(a);
          yyparse();


          cout<< "-----------------------SENTENCE STORE -----------------------" << endl;
          for(auto lt = sentenceStore.begin(); lt != sentenceStore.end(); lt++) {
            cout << lt->first << "\t"<< (lt->second.inSet() == AUX_SET? "AUX_SET": "SUPPORT_SET")<<"\t\t" << lt->second.stringify()<<endl;
          }


          int last = 0;
          bool conflict_found = false;
          while(!conflict_found) {
    /*
            cout<< "======================== REASONING STEP ========================" << endl;
    */
            SENTENCE_ID_TYPE sid = set_support[last++];
            SentenceDNF s = sentenceStore[sid];
            vector<Literal> literals = s.getLiterals();
            bool resolved = false;
            for(int i = 0; i < literals.size() && !conflict_found; i++) {
              Literal literal = literals[i];
              vector<SENTENCE_ID_TYPE> foundlist = myIndex.find(literal.getPredictId(), !literal.getTrueOrNegated());
              for(int j = 0; j < foundlist.size() && !conflict_found; j++) {
                SENTENCE_ID_TYPE id = foundlist[j];
                if( s.isMyParent(id) ) continue;
                if( s.getLiterals().size() != 1 && sentenceStore[id].getLiterals().size() != 1) continue;
                int t = resolution_and_put_result_into_support_set(sid, i, id, SUPPORT_SET, set_support);
                // resolved successfully
                if(t == 1) {
                  resolved = true;

                  for(auto lt = sentenceStore.begin(); lt != sentenceStore.end(); lt++) {
                    cout << lt->first << "\t"<< (lt->second.inSet() == AUX_SET? "AUX_SET": "SUPPORT_SET")<<"\t\t" << lt->second.stringify();
                    cout << "\t parent : ";
                    for(auto ll = lt->second.parents.begin(); ll != lt->second.parents.end(); ll++) {
                      cout << " S_"<< *ll ;
                    }
                    cout <<endl;
                  }
                  getchar();

                }
                else if (t==2) {
                  conflict_found = true;
                  break;
                }
              }
            }

            if(conflict_found) {

              cout<<"END, GET CONFLICT" <<endl;

              out_file << "TRUE" <<endl;
              //getchar();
              break;
            }
            else if(!resolved && last == set_support.size()) {

              cout<<"CANNOT GO FORWARD !!!! WE CANNOT RESOLVE THIS SENTENCE:" <<endl;
              cout<<sid<<". "<<s.stringify()<<endl;

              out_file << "FALSE" <<endl;
              //getchar();
              break;
            }
        }

    }

    out_file.close();
    return 0;
}

void yyerror(const char *s) {
    cout << "EEK, parse error!  Message: " << s << endl;
    // might as well halt now:
    exit(-1);
}
