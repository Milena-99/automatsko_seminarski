#include <iostream>
#include <fstream>
#include<list>

using Atom = int;
using Literal = int;
using Clause = std::list<Literal>;
using NormalForm = std::list<Clause>;

NormalForm readCNF(std::istream& inputStream, unsigned& atomCount) {
    std::string buffer;
    do {
        inputStream >> buffer;
        if(buffer == "c")
            inputStream.ignore(std::string::npos, '\n');
    } while(buffer != "p");

    // sad je buffer == "p"
    inputStream >> buffer;
    // sad je buffer "cnf"

    unsigned clauseCount;
    inputStream >> atomCount >> clauseCount;

    NormalForm formula;
    for(unsigned i = 0; i < clauseCount; i++) {
        Clause c;

        Literal l;
        inputStream >> l;
        while(l != 0) {
            c.push_back(l);
            inputStream >> l;
        }
        formula.push_back(c);
    }
    return formula;
}

void print(NormalForm& f) {
    for(Clause& c : f){
        for(Literal& l : c){
            std::cout << l << " ";
        }
        std::cout << "\n";
    }

}

NormalForm::iterator& remove_tautology_clause(NormalForm& f, NormalForm::iterator& pos_clause) {
    for(Literal l : *pos_clause){
        for(Literal &l1 : *pos_clause) {
            if(l + l1 == 0){
               pos_clause = f.erase(pos_clause);
               return pos_clause;
            }
        }
    }
    return ++pos_clause;
}
bool davis_putnam (NormalForm& cnf) {
    auto it = cnf.begin();
    while(it != cnf.end()) {
        // brisanje tautologicnih klauza
         it=remove_tautology_clause(cnf,it);

        //propagacija jedinicnih klauza

        //eliminacija cistih lierala

       //eliminacija promenljive-rezolucija


   }
     print(cnf);

    return true;
}



int main() {
    std::ifstream infile;
    infile.open("in.txt");
    unsigned atomCount;
    NormalForm cnf = readCNF(infile, atomCount);
    bool val = davis_putnam(cnf);

    return 0;
}

