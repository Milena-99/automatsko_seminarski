#include <iostream>
#include <fstream>
#include <set>
#include <list>
#include <cstdlib>


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
        if((*it).find(l) != (*it).end()) {
          //  printf("elementi pre 1:\n");
          //  print(f);
            it = f.erase(it);
            if(f.size() == 0)
                return;
          //  printf("elementi posle 1:\n");
          //  print(f);
        } else if((*it).find(-l) != (*it).end()) {
            (*it).erase((*it).find(-l));
            ++it;
          //  printf("elementi 2:\n");
          //  print(f);
        } else {
            ++it;
           // printf("elementi 3:\n");
          //  print(f);
        }
        
    }
   // printf("ok1");

}


template <typename T>
void delta_razlika(std::set<T>& A, std::set<T>& B) {
	auto it = A.begin();
	while(it != A.end()) {
		if(B.erase(-(*it))) it = A.erase(it);
		else it++;
	}
	
	it = B.begin();
	while(it != B.end()) {
		if(A.erase(-(*it))) it = B.erase(it);
		else it++;
	}
	
	A.merge(B);	
}

void pure_literal(NormalForm& cnf, const Literal& l) {
   /* auto it = f.begin();
    while(it != f.end()) {
        if((*it).find(l) != (*it).end()){
            (*it).erase((*it).find(l));
        }
        ++it;
    }
    */
    
   std::set<Literal> tmp1;
    std::set<Literal> tmp2;
    
    for(const auto& C : cnf) {
    	for(const auto& L : C) {
    		if(L > 0) tmp1.insert(L);
    		else tmp2.insert(L);
    	}
    }
    
    delta_razlika<Literal>(tmp1, tmp2);
    
    for(const auto& L : tmp1) {
    	std::erase_if(cnf, [L](std::set<Literal>& C) {return C.find(L) != C.end();});
    }
}

bool clauseTautology(Clause& c) {
	auto it = c.begin();
	while(it != c.end()) {
		if(c.find(-(*it)) != c.end())
			return true;
	}
	return false;

}

bool resolveClauses(NormalForm& f, Clause& c1, Clause& c2, Literal l) {
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

bool davis_putnam (NormalForm& cnf) {
    auto it = cnf.begin();
    while(it != cnf.end()) {
        // brisanje tautologicnih klauza
         it=remove_tautology_clause(cnf, it);

    }
printf("pre unit_propagation\n");
print(cnf);
   while(cnf.size() != 0) {
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
    //izlazak
    if(cnf.size() == 0)
	    return true;
    it = cnf.begin();
    while(it != cnf.end()) {
	    if((*it).empty())
		    return false;
    	    it++;
    }
    //eliminacija promenljive-rezolucija
    std::set<Clause> tmp1;
    std::set<Clause> tmp2;
    it = cnf.begin();
    Literal l =*((*it).begin());
    for(auto it1 = cnf.begin(); it1 != cnf.end(); it++)
	    if((*it).find(l) != (*it).end())
		    tmp1.insert(*it1);
    	    else if((*it).find(-l) != (*it).end())
		    tmp2.insert(*it1);

    auto it2 = tmp1.begin();
    while(it2 != tmp1.end()) {
    	auto it1 = tmp2.begin();
	while(it1 != tmp2.end()){
		resolveClauses(cnf, *it2, *it1, l);
		it1++;
	}
	it2++;
    }

    it = cnf.begin();
    while(it != cnf.end()){
    	if((*it).find(l) != (*it).end() || (*it).find(-l) != (*it).end())
		it = cnf.erase(it);
	else 
		it++;
    }
   }
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
