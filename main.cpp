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


std::set<Literal> findPureLiterals(const NormalForm& formula) {
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

    std::set<Literal> pureLiterals;

    // Pronalazimo čiste litrele
    for (Literal literal : positiveLiterals) {
        if (negativeLiterals.count(literal) == 0) {
            pureLiterals.insert(literal);
        }
    }

    for (Literal literal : negativeLiterals) {
        if (positiveLiterals.count(literal) == 0) {
            pureLiterals.insert(literal);
        }
    }

    return pureLiterals;
}

bool davis_putnam (NormalForm& cnf) {
    auto it = cnf.begin();
    while(it != cnf.end()) {
         it = remove_tautology_clause(cnf, it);

    }
    std::cout << "posle brisanja tautologicnih klauza\n";
    print(cnf);
    int i = 1;
    while(true) {
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
	std::set<Literal> allLiteral =  findPureLiterals(cnf);
	auto itPureClause = cnf.begin();
	bool ind = false;
       	while(itPureClause != cnf.end()) {
		auto itC = (*itPureClause).begin();
		ind = 0;
		while (itC != (*itPureClause).end()) {
			if(allLiteral.find(*itC) != allLiteral.end()) {
				itPureClause = cnf.erase(itPureClause);
				ind = true;
				break;
			} 

			itC++;
		}
		if (!ind)
			itPureClause++;	

	}

        //izlazak
        if(cnf.empty()) {
            std::cout << "U koraku: " << i << " SAT\n";
	    return true;
	}
        it = cnf.begin();
        while(it != cnf.end()) {
            if(it->empty()) {
             	std::cout << "U koraku: " << i << " UNSAT\n";      
		return false;
	     }
	    it++;
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
        for(auto& it1 : tmp1) {
            cnf.erase(it1);
        }
        
        for(auto& it2 : tmp2) {
            cnf.erase(it2);
        }
        
        std::cout << "korak:" << i  << "\n";
	i++;
  	print(cnf); 
   
    }
    return true;
}


int main(int argc, char *argv[]) {
    std::ifstream infile;
    if(argc < 2) {
	    std::cout << "Program se poziva:\n ./izvrsni ulaz.txt\n";
    	    return -1;
    }
    infile.open(argv[1]);
    unsigned atomCount;
    NormalForm cnf = readCNF(infile, atomCount);
    bool val = davis_putnam(cnf);
    std::cout << "Formula je: " << val << '\n';

    return 0;
}
