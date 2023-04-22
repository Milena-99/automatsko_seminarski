#include <iostream>
#include <fstream>
#include <set>
#include <list>

using Atom = int;
using Literal = int;
using Clause = std::set<Literal>;
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
            c.insert(l);
            inputStream >> l;
        }
        formula.push_back(c);
    }
    return formula;
}

void print(NormalForm& f) {
    for(auto c : f){
        for(auto l : c){
            std::cout << l << " ";
        }
        std::cout << "\n";
    }

}

NormalForm::iterator& remove_tautology_clause(NormalForm& f, NormalForm::iterator& pos_clause) {
    for(Literal l : *pos_clause){
        for(auto l1 : (*pos_clause)) {
            if(l + l1 == 0){
               pos_clause = f.erase(pos_clause);
               return pos_clause;
            }
        }
    }
    return ++pos_clause;
}


void unit_propagation(NormalForm& f,  Clause& c) {
    auto it = f.begin();
    Literal l =  c.extract(c.begin()).value();
    while (it != f.end()) {
        if((*it).find(l) != (*it).end()) {
            printf("elementi pre 1:\n");
            print(f);
            it = f.erase(it);
            if(f.size() == 0)
                return;
            printf("elementi posle 1:\n");
            print(f);
        } else if((*it).find(-l) != (*it).end()) {
            (*it).erase((*it).find(-l));
            ++it;
            printf("elementi 2:\n");
            print(f);
        } else {
            ++it;
            printf("elementi 3:\n");
            print(f);
        }
        
    }
    printf("ok1");

}

void pure_literal(NormalForm& f, const Literal& l) {
    auto it = f.begin();
    while(it != f.end()) {
        if((*it).find(l) != (*it).end()){
            (*it).erase((*it).find(l));
        }
        ++it;
    }
}

bool davis_putnam (NormalForm& cnf) {
    auto it = cnf.begin();
    while(it != cnf.end()) {
        // brisanje tautologicnih klauza
         it=remove_tautology_clause(cnf, it);

    }
printf("pre unit_propagation\n");
print(cnf);
    //propagacija jedinicnih klauza
    it = cnf.begin();
    int num_translation;
    while(it != cnf.end()) {
        if((*it).size() == 1){
            //printf("ok");
            Clause c = *it;
            it = cnf.erase(it);
            unit_propagation(cnf, c);
        } else {
            it++;
        }
        
    }



    //eliminacija cistih literala
    it = cnf.begin();
    std::set<Literal> tmp;
    while(it != cnf.end()) {
        auto it1=(*it).begin();
        while(it1 != (*it).end()) {
            tmp.insert(*it1);
            ++it1;
        }
        ++it;
    }

    it = cnf.begin();
    while(it != cnf.end()) {
        auto it1=(*it).begin();
        while(it1 != (*it).end()) {
            if(tmp.find(-(*it1)) == tmp.end())
                pure_literal(cnf, (*it1));
            it1++;
        }
        ++it;
    }

    //eliminacija promenljive-rezolucija

    printf("kraj:\n");

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