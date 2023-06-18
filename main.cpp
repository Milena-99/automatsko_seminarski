#include <iostream>
#include <fstream>
#include <set>
#include <list>
#include <cstdlib>
#include <vector>

//Impelemnatacija DP procedure za iskaznu logiku
//
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
        if(it->find(l) != it->end()) {
            it = f.erase(it);
            if(f.empty())
                return;
        } else if(it->find(-l) != it->end()) {
            it->erase(it->find(-l));
            ++it;
        } else {
            ++it;
        }
        
    }

}


bool clauseTautology(Clause& c) {
	auto it = c.begin();
	while(it != c.end()) {
		if(c.find(-(*it)) != c.end())
			return true;
		++it;
	}
	return false;

}

bool resolveClauses(NormalForm& f, const Clause& c1, const Clause& c2, Literal l) {
    bool change = false;
    Clause r;
    for(const auto& literal : c1)
        if(literal != l)
	    r.insert(literal);
    for(const auto& literal : c2)
	if(literal != -l)
            r.insert(literal);

    if(!clauseTautology(r)) {
        change = true;
        f.push_back(r);
    }
    
    return change;
}



bool pureLiteralClauses(NormalForm& f, const Clause& c1, Literal l) {
    bool change = false;
    Clause r;
    for(const auto& literal : c1)
        if(literal != l)
            r.insert(literal);

    if(!clauseTautology(r)) {
        change = true;
        f.push_back(r);
    }

    return change;
}

std::vector<Literal> findPureLiterals(const NormalForm& formula) {
    std::set<Literal> positiveLiterals;
    std::set<Literal> negativeLiterals;

    // Prolazimo kroz sve klauze u formuli i beležimo literale
    for (const Clause& clause : formula) {
        for (Literal literal : clause) {
            if (literal > 0) {
                positiveLiterals.insert(literal);
            } else {
                negativeLiterals.insert(-literal);
            }
        }
    }

    std::vector<Literal> pureLiterals;

    // Pronalazimo čiste litrele
    for (Literal literal : positiveLiterals) {
        if (negativeLiterals.count(literal) == 0) {
            pureLiterals.push_back(literal);
        }
    }

    for (Literal literal : negativeLiterals) {
        if (positiveLiterals.count(literal) == 0) {
            pureLiterals.push_back(-literal);
        }
    }

    return pureLiterals;
}

bool davis_putnam (NormalForm& cnf) {
    auto it = cnf.begin();
    while(it != cnf.end()) {
         it = remove_tautology_clause(cnf, it);

    }
    printf("pre unit_propagation\n");
    print(cnf);
    while(!cnf.empty()) {
      //propagacija jedinicnih klauze
        it = cnf.begin();
        while(it != cnf.end()) {
            if(it->size() == 1) {
                Clause c = *it;
                it = cnf.erase(it);
                unit_propagation(cnf, c);
                it = cnf.begin();
            } else {
                ++it;
            }
            
        }
	
	//pure literal
	std::vector<Literal> allLiteral =  findPureLiterals(cnf);
	auto itPure = allLiteral.begin();
	while(itPure != allLiteral.end()) {
		auto itc = cnf.begin();
		while (itc != cnf.end()) {
			pureLiteralClauses(cnf, *itc, *itPure);
			itc++;
		}
		itPure++;
	}

	itPure = allLiteral.begin();	
	while(itPure != allLiteral.end()) {
                auto itc = cnf.begin();
                while (itc != cnf.end()) {
                	if((*itc).find(*itPure) != (*itc).end())
				cnf.erase(itc);
			else
				itc++;
                }
                itPure++;
        }

        //izlazak
        if(cnf.empty())
            return true;
        it = cnf.begin();
        while(it != cnf.end()) {
            if(it->empty())
                return false;
                ++it;
        }
        
        it = cnf.begin();
        Literal l = *(it->begin());
        std::cout << "Literal koji eliminisemo: " << l << '\n';
        std::list<std::list<Clause>::iterator> tmp1, tmp2;
        
        for(; it != cnf.end(); ++it) {
            if(it->find(l) != it->end())
                tmp1.push_back(it);
            else if(it->find(-l) != it->end())
                tmp2.push_back(it);
            
        }
        
        // sada se setamo po oba
        for(const auto& it1 : tmp1) {
            for(const auto& it2 : tmp2) {
                resolveClauses(cnf, *it1, *it2, l);
            }
        }

        std::cout << "tmp1\n";
        for(const auto& it1 : tmp1) {
            for(const auto& l : *it1) {
                std::cout << l << " ";
            }
            std::cout << '\n';
        }
        std::cout << '\n';
        std::cout << "tmp2\n";
        for(const auto& it1 : tmp2) {
            for(const auto& l : *it1) {
                std::cout << l << " ";
            }
            std::cout << '\n';
        }
        std::cout << '\n';

        for(auto& it1 : tmp1) {
            cnf.erase(it1);
        }
        
        for(auto& it2 : tmp2) {
            cnf.erase(it2);
        }
        
        std::cout << '\n';
        std::cout << "cnf\n";
        for(const auto& clause : cnf) {
            for(const auto& l : clause) {
                std::cout << l << " ";
            }
            std::cout << '\n';
        }
   
   
    }
    return true;
}


int main(int argc, char *argv[]) {
    std::ifstream infile;
    if(argc < 2) {
	    printf("Program se poziva:\n ./izvrsni ulaz.txt\n");
    	    return -1;
    }
    infile.open(argv[1]);
    unsigned atomCount;
    NormalForm cnf = readCNF(infile, atomCount);
    bool val = davis_putnam(cnf);
    std::cout << "Formula je: " << val << '\n';

    return 0;
}
