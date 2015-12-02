#ifndef CONSTRAINT_PARSER_H
#define CONSTRAINT_PARSER_H

#include <regex>
#include <map>
#include <string>

#include <yices.h>

class ConstraintParser
{
	public:
		static ConstraintParser INSTANCE;
		term_t getNormalizedVarName(const std::string& var);
		term_t parse(const std::string& arg);

	private:
		term_t parseEdge(const std::string& arg);
		term_t parseAssign(const std::string& arg);
		term_t parseArgument(const std::string& arg);
		ConstraintParser();
		~ConstraintParser();
		unsigned int _index{0};
		const std::string _base{"var"};
		const std::map<std::string,term_t(*)(term_t,term_t)> _ops
			{
			  {">",   yices_arith_gt_atom},
			  {"<",   yices_arith_lt_atom},
			  {">=",  yices_arith_geq_atom},
			  {"<=",  yices_arith_leq_atom},
			  {"==",  yices_arith_eq_atom},
			  {"!=",  yices_arith_neq_atom},
			  {">!",  yices_arith_lt_atom},
			  {"<!",  yices_arith_gt_atom},
			  {">=!", yices_arith_leq_atom},
			  {"<=!", yices_arith_geq_atom},
			  {"==!", yices_arith_neq_atom},
			  {"!=!", yices_arith_eq_atom}
			};
		const type_t Yices_int{yices_int_type()};
		std::map<std::string,term_t> _vars;
		std::map<std::string,term_t> _built;
		std::regex _edgereg{R"regex(\[(\!?)\s*((?:.+\.\d+)|\d+)\s*([<>=!]+)\s*((?:.+\.\d+)|\d+)\])regex"};
		std::regex _numreg{R"regex(\d*)regex"};
		std::regex _assignreg{R"regex(([^=]*?) = ([^=]*))regex"};
};

class Constraint
{
	public:
		Constraint(const std::string& constraint) :
			_inner{ConstraintParser::INSTANCE.parse(constraint)}
#ifdef DEBUG_CONSTRAINTS
			,_display{constraint}
#endif
		{}
		Constraint(const Constraint& other) :
			_inner{other._inner}
#ifdef DEBUG_CONSTRAINTS
			,_display{other._display}
#endif
		{}
		static Constraint conjunct(const std::vector<Constraint>& c);
		static bool check_unsatisfiability(const Constraint& c);

	private:
		Constraint(term_t parsedConstraint) :
			_inner{parsedConstraint}
#ifdef DEBUG_CONSTRAINTS
			,_display{"-- unknown --"}
#endif
		{}
		term_t _inner;
#ifdef DEBUG_CONSTRAINTS
		const std::string _display;
#endif

	friend std::ostream& operator<<(std::ostream& out, const Constraint& c);
};

#endif
